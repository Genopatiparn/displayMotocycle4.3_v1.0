/**
 * @file gps_sr1723.cpp
 * @brief SR1723 GPS Module Driver Implementation
 * 
 * Adapted from existing GPS_Driver for SR1723 compatibility
 */

#include "gps_sr1723.h"
#include "hardware_config.h"
#include <Arduino.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

// GPS Serial port
static HardwareSerial gpsSerial(GPS_UART_NUM);

// Thread-safe locks for data access
static portMUX_TYPE s_gps_line_lock = portMUX_INITIALIZER_UNLOCKED;
static portMUX_TYPE s_gsv_lock = portMUX_INITIALIZER_UNLOCKED;

// Latest NMEA sentences
static char s_gps_latest_line[256] = "Waiting for GPS data...";
static char s_gps_latest_gga[256] = "No GGA data yet";

// GPS data storage
static gps_debug_info_t s_gps_debug_info = {
    "N/A", "N/A", "N/A", "0.00", "N/A", "N/A", "N/A", "N/A", 0, 0
};

// Satellite counters
static uint8_t s_gsv_gps = 0;
static uint8_t s_gsv_glo = 0;
static uint8_t s_gsv_gal = 0;
static uint8_t s_gsv_bds = 0;

// Satellite signal strength data
static gps_satellite_t s_satellites[GPS_MAX_SATELLITES] = {0};
static uint8_t s_satellite_count = 0;
static portMUX_TYPE s_satellite_lock = portMUX_INITIALIZER_UNLOCKED;

// NMEA line buffer
static char s_nmea_line[256] = {0};
static size_t s_nmea_idx = 0;

static uint8_t s_ubx_state = 0;
static uint8_t s_ubx_class = 0;
static uint8_t s_ubx_id = 0;
static uint16_t s_ubx_length = 0;
static uint16_t s_ubx_index = 0;
static uint8_t s_ubx_ck_a = 0;
static uint8_t s_ubx_ck_b = 0;
static uint8_t s_ubx_rx_ck_a = 0;
static uint8_t s_ubx_rx_ck_b = 0;
static uint8_t s_ubx_payload[96] = {0};

// GPS fix status
static bool s_gps_fix_valid = false;

static uint32_t read_u32_le(const uint8_t *data)
{
    return (uint32_t)data[0] |
           ((uint32_t)data[1] << 8) |
           ((uint32_t)data[2] << 16) |
           ((uint32_t)data[3] << 24);
}

static void gps_parse_ubx_byte(uint8_t byte)
{
    switch (s_ubx_state) {
        case 0: s_ubx_state = byte == 0xB5 ? 1 : 0; break;
        case 1: s_ubx_state = byte == 0x62 ? 2 : 0; break;
        case 2: s_ubx_class = byte; s_ubx_ck_a = byte; s_ubx_ck_b = s_ubx_ck_a; s_ubx_state = 3; break;
        case 3: s_ubx_id = byte; s_ubx_ck_a += byte; s_ubx_ck_b += s_ubx_ck_a; s_ubx_state = 4; break;
        case 4: s_ubx_length = byte; s_ubx_ck_a += byte; s_ubx_ck_b += s_ubx_ck_a; s_ubx_state = 5; break;
        case 5:
            s_ubx_length |= (uint16_t)byte << 8;
            s_ubx_ck_a += byte;
            s_ubx_ck_b += s_ubx_ck_a;
            s_ubx_index = 0;
            s_ubx_state = s_ubx_length <= sizeof(s_ubx_payload) ? 6 : 0;
            break;
        case 6:
            s_ubx_payload[s_ubx_index++] = byte;
            s_ubx_ck_a += byte;
            s_ubx_ck_b += s_ubx_ck_a;
            if (s_ubx_index >= s_ubx_length) s_ubx_state = 7;
            break;
        case 7: s_ubx_rx_ck_a = byte; s_ubx_state = 8; break;
        case 8:
            s_ubx_rx_ck_b = byte;
            if (s_ubx_rx_ck_a == s_ubx_ck_a && s_ubx_rx_ck_b == s_ubx_ck_b &&
                s_ubx_class == 0x01 && s_ubx_id == 0x07 && s_ubx_length >= 44) {  // Need at least 44 bytes
                const uint8_t fix_type = s_ubx_payload[20];
                const uint8_t flags = s_ubx_payload[21];
                const uint32_t accuracy_mm = read_u32_le(&s_ubx_payload[40]);  // hAcc is at offset 40!
                
                taskENTER_CRITICAL(&s_gps_line_lock);
                s_gps_debug_info.horizontal_accuracy_mm = accuracy_mm;
                if ((flags & 0x01) == 0 || fix_type == 0) {
                    s_gps_debug_info.horizontal_accuracy_mm = 0;
                }
                taskEXIT_CRITICAL(&s_gps_line_lock);
            }
            s_ubx_state = 0;
            break;
    }
}

// ==================== Helper Functions ====================

static void gps_set_latest_line(const char *line) {
    if (line == NULL) return;
    taskENTER_CRITICAL(&s_gps_line_lock);
    snprintf(s_gps_latest_line, sizeof(s_gps_latest_line), "%s", line);
    taskEXIT_CRITICAL(&s_gps_line_lock);
}

static void gps_set_latest_gga(const char *line) {
    if (line == NULL) return;
    taskENTER_CRITICAL(&s_gps_line_lock);
    snprintf(s_gps_latest_gga, sizeof(s_gps_latest_gga), "%s", line);
    taskEXIT_CRITICAL(&s_gps_line_lock);
}

static int nmea_hex_nibble(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

static bool nmea_checksum_valid(const char *line) {
    if (line == NULL || line[0] != '$') return false;
    
    const char *asterisk = strchr(line, '*');
    if (asterisk == NULL || asterisk[1] == '\0' || asterisk[2] == '\0') 
        return false;

    uint8_t checksum = 0;
    for (const char *p = line + 1; p < asterisk; p++) {
        checksum ^= (uint8_t)(*p);
    }

    int high = nmea_hex_nibble(asterisk[1]);
    int low = nmea_hex_nibble(asterisk[2]);
    if (high < 0 || low < 0) return false;

    uint8_t expected = (uint8_t)((high << 4) | low);
    return checksum == expected;
}

static bool nmea_get_field(const char *line, uint8_t field_index, 
                          char *out, size_t out_size) {
    if (line == NULL || out == NULL || out_size == 0) return false;

    uint8_t current = 0;
    const char *field_start = line;
    const char *p = line;

    while (true) {
        char c = *p;
        if (c == ',' || c == '*' || c == '\0') {
            if (current == field_index) {
                size_t len = (size_t)(p - field_start);
                if (len >= out_size) len = out_size - 1;
                memcpy(out, field_start, len);
                out[len] = '\0';
                return true;
            }

            if (c == '*' || c == '\0') break;
            
            current++;
            field_start = p + 1;
        }
        p++;
    }

    out[0] = '\0';
    return false;
}

static bool nmea_to_decimal_degrees(const char *value, const char *hemi, 
                                    bool is_latitude, double *out_deg) {
    if (value == NULL || hemi == NULL || out_deg == NULL || 
        value[0] == '\0' || hemi[0] == '\0') 
        return false;

    double raw = strtod(value, NULL);
    if (raw <= 0.0) return false;

    int degrees = (int)(raw / 100.0);
    double minutes = raw - (degrees * 100.0);
    double decimal = (double)degrees + (minutes / 60.0);

    if ((is_latitude && (hemi[0] == 'S' || hemi[0] == 's')) ||
        (!is_latitude && (hemi[0] == 'W' || hemi[0] == 'w'))) {
        decimal = -decimal;
    }

    *out_deg = decimal;
    return true;
}

static bool nmea_sentence_is_type(const char *line, const char *type3) {
    if (line == NULL || type3 == NULL) return false;
    if (line[0] != '$') return false;
    if (line[1] == '\0' || line[2] == '\0' || line[3] == '\0' || 
        line[4] == '\0' || line[5] == '\0') return false;

    return (line[3] == type3[0] && line[4] == type3[1] && line[5] == type3[2]);
}

// ==================== NMEA Parsers ====================

static void gps_parse_rmc_fields(const char *line) {
    char status[4] = {0};
    if (!nmea_get_field(line, 2, status, sizeof(status)) || status[0] != 'A') {
        s_gps_fix_valid = false;
        return;
    }

    s_gps_fix_valid = true;

    char lat_raw[20] = {0};
    char lat_hemi[4] = {0};
    char lon_raw[20] = {0};
    char lon_hemi[4] = {0};
    char speed_knots[20] = {0};
    char utc_time_raw[20] = {0};
    char utc_date_raw[20] = {0};

    nmea_get_field(line, 1, utc_time_raw, sizeof(utc_time_raw));
    nmea_get_field(line, 3, lat_raw, sizeof(lat_raw));
    nmea_get_field(line, 4, lat_hemi, sizeof(lat_hemi));
    nmea_get_field(line, 5, lon_raw, sizeof(lon_raw));
    nmea_get_field(line, 6, lon_hemi, sizeof(lon_hemi));
    nmea_get_field(line, 7, speed_knots, sizeof(speed_knots));
    nmea_get_field(line, 9, utc_date_raw, sizeof(utc_date_raw));

    taskENTER_CRITICAL(&s_gps_line_lock);

    // Parse UTC time
    if (utc_time_raw[0] != '\0' && strlen(utc_time_raw) >= 6) {
        snprintf(s_gps_debug_info.utc_time, sizeof(s_gps_debug_info.utc_time),
                "%.2s:%.2s:%.2s", utc_time_raw, utc_time_raw + 2, utc_time_raw + 4);
    }

    // Parse UTC date
    if (utc_date_raw[0] != '\0' && strlen(utc_date_raw) >= 6) {
        snprintf(s_gps_debug_info.utc_date, sizeof(s_gps_debug_info.utc_date),
                "%.2s/%.2s/%.2s", utc_date_raw, utc_date_raw + 2, utc_date_raw + 4);
    }

    // Parse latitude
    double lat_dec = 0.0;
    if (nmea_to_decimal_degrees(lat_raw, lat_hemi, true, &lat_dec)) {
        snprintf(s_gps_debug_info.lat, sizeof(s_gps_debug_info.lat), "%.6f", lat_dec);
    }

    // Parse longitude
    double lon_dec = 0.0;
    if (nmea_to_decimal_degrees(lon_raw, lon_hemi, false, &lon_dec)) {
        snprintf(s_gps_debug_info.lon, sizeof(s_gps_debug_info.lon), "%.6f", lon_dec);
    }

    // Parse speed (knots to km/h)
    if (speed_knots[0] != '\0') {
        double kmh = strtod(speed_knots, NULL) * 1.852;
        snprintf(s_gps_debug_info.speed_kmh, sizeof(s_gps_debug_info.speed_kmh), 
                "%.2f", kmh);
    }

    taskEXIT_CRITICAL(&s_gps_line_lock);
} 

static void gps_parse_gga_fields(const char *line) {
    char fix_quality[4] = {0};
    char num_sats[8] = {0};
    char altitude[16] = {0};
    nmea_get_field(line, 6, fix_quality, sizeof(fix_quality));
    nmea_get_field(line, 7, num_sats, sizeof(num_sats));
    nmea_get_field(line, 9, altitude, sizeof(altitude));

    taskENTER_CRITICAL(&s_gps_line_lock);
    s_gps_debug_info.fix_quality = (uint8_t)atoi(fix_quality);
    if (num_sats[0] != '\0')
        snprintf(s_gps_debug_info.num_sats, sizeof(s_gps_debug_info.num_sats), 
                "%s", num_sats);
    if (altitude[0] != '\0')
        snprintf(s_gps_debug_info.altitude_m, sizeof(s_gps_debug_info.altitude_m), 
                "%s", altitude);
    taskEXIT_CRITICAL(&s_gps_line_lock);
}

static void gps_parse_gsv_fields(const char *line) {
    char msg_num_str[4] = {0};
    char total_msg_str[4] = {0};
    char total_sv_str[4] = {0};

    nmea_get_field(line, 1, total_msg_str, sizeof(total_msg_str));
    nmea_get_field(line, 2, msg_num_str, sizeof(msg_num_str));
    nmea_get_field(line, 3, total_sv_str, sizeof(total_sv_str));

    if (msg_num_str[0] == '\0' || total_sv_str[0] == '\0') return;
    
    int msg_num = atoi(msg_num_str);
    uint8_t sv_count = (uint8_t)atoi(total_sv_str);

    // NOTE: We do NOT clear the satellite array here anymore!
    // The satellite array accumulates satellites from ALL constellations (GPS, GLONASS, Galileo, Beidou, QZSS).
    // Each satellite is identified by (constellation, prn) pair, so they won't overwrite each other.
    // Old satellites with SNR=0 will naturally expire or be filtered out.

    // Update constellation counts (only on first message)
    if (msg_num == 1) {
        taskENTER_CRITICAL(&s_gsv_lock);
        if (line[1] == 'G' && line[2] == 'P')
            s_gsv_gps = sv_count;
        else if (line[1] == 'G' && line[2] == 'L')
            s_gsv_glo = sv_count;
        else if (line[1] == 'G' && line[2] == 'A')
            s_gsv_gal = sv_count;
        else if (line[1] == 'G' && line[2] == 'B')
            s_gsv_bds = sv_count;

        uint8_t total = s_gsv_gps + s_gsv_glo + s_gsv_gal + s_gsv_bds;
        taskEXIT_CRITICAL(&s_gsv_lock);

        taskENTER_CRITICAL(&s_gps_line_lock);
        snprintf(s_gps_debug_info.total_sats, sizeof(s_gps_debug_info.total_sats), 
                "%u", total);
        taskEXIT_CRITICAL(&s_gps_line_lock);
    }

    // Parse satellite details (up to 4 satellites per GSV message)
    // Fields: prn, elevation, azimuth, snr (repeat 4 times)
    for (int i = 0; i < 4; i++) {
        char prn_str[8] = {0};
        char elev_str[8] = {0};
        char azi_str[8] = {0};
        char snr_str[8] = {0};

        int base_field = 4 + (i * 4);
        if (!nmea_get_field(line, base_field, prn_str, sizeof(prn_str)) || prn_str[0] == '\0')
            break;

        uint8_t prn = (uint8_t)atoi(prn_str);
        uint8_t elev = 0;
        uint16_t azi = 0;
        uint8_t snr = 0;

        if (nmea_get_field(line, base_field + 1, elev_str, sizeof(elev_str)) && elev_str[0] != '\0')
            elev = (uint8_t)atoi(elev_str);
        if (nmea_get_field(line, base_field + 2, azi_str, sizeof(azi_str)) && azi_str[0] != '\0')
            azi = (uint16_t)atoi(azi_str);
        if (nmea_get_field(line, base_field + 3, snr_str, sizeof(snr_str)) && snr_str[0] != '\0')
            snr = (uint8_t)atoi(snr_str);

        // Store satellite info
        taskENTER_CRITICAL(&s_satellite_lock);
        
        // Find existing satellite or add new one
        int slot = -1;
        uint8_t constellation = 0;
        if (line[1] == 'G' && line[2] == 'P') constellation = 1;
        else if (line[1] == 'G' && line[2] == 'L') constellation = 2;
        else if (line[1] == 'G' && line[2] == 'A') constellation = 3;
        else if (line[1] == 'G' && line[2] == 'B') constellation = 4;
        else if (line[1] == 'G' && line[2] == 'Q') constellation = 5;

        for (int j = 0; j < s_satellite_count; j++) {
            if (s_satellites[j].prn == prn &&
                s_satellites[j].constellation == constellation) {
                slot = j;
                break;
            }
        }
        
        if (slot == -1 && s_satellite_count < GPS_MAX_SATELLITES) {
            slot = s_satellite_count++;
        }
        
        if (slot >= 0) {
            s_satellites[slot].prn = prn;
            s_satellites[slot].elevation = elev;
            s_satellites[slot].azimuth = azi;
            s_satellites[slot].snr = snr;
            s_satellites[slot].constellation = constellation;
        }
        
        taskEXIT_CRITICAL(&s_satellite_lock);
    }
}

// ==================== Public API ====================

void GPS_UART_Init(void) {
    gpsSerial.setRxBufferSize(GPS_RX_BUF_SIZE);
    gpsSerial.begin(GPS_BAUD_RATE, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
}

// ============================================================================
// UBX Binary Protocol Functions (for u-blox GPS chipsets)
// ============================================================================

static void GPS_SendUBX(uint8_t msg_class, uint8_t msg_id, const uint8_t *payload, uint16_t payload_len) {
    uint8_t ck_a = 0;
    uint8_t ck_b = 0;
    
    auto checksum_add = [&](uint8_t b) {
        ck_a = (uint8_t)(ck_a + b);
        ck_b = (uint8_t)(ck_b + ck_a);
    };
    
    // UBX header
    gpsSerial.write((uint8_t)0xB5);  // Sync char 1
    gpsSerial.write((uint8_t)0x62);  // Sync char 2
    
    gpsSerial.write(msg_class);
    checksum_add(msg_class);
    
    gpsSerial.write(msg_id);
    checksum_add(msg_id);
    
    // Payload length (little-endian)
    uint8_t len_l = (uint8_t)(payload_len & 0xFF);
    uint8_t len_h = (uint8_t)((payload_len >> 8) & 0xFF);
    gpsSerial.write(len_l);
    checksum_add(len_l);
    gpsSerial.write(len_h);
    checksum_add(len_h);
    
    // Payload
    for (uint16_t i = 0; i < payload_len; i++) {
        gpsSerial.write(payload[i]);
        checksum_add(payload[i]);
    }
    
    // Checksum
    gpsSerial.write(ck_a);
    gpsSerial.write(ck_b);
    gpsSerial.flush();
}

static void GPS_SendUBX_EnableNMEA() {
    // Enable antenna power (UBX-CFG-ANT) for active GPS antennas
    uint8_t cfg_ant[4] = {
        0x00, 0x00,      // flags: bit 0 = svcs (antenna control enabled), bit 1 = scd (short circuit detection)
        0xF0, 0x3D       // pins: antenna power on
    };
    GPS_SendUBX(0x06, 0x13, cfg_ant, sizeof(cfg_ant));
    delay(100);
    
    // Enable GGA message (position fix data) - output every epoch
    uint8_t cfg_gga[8] = {0xF0, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00};
    GPS_SendUBX(0x06, 0x01, cfg_gga, sizeof(cfg_gga));
    delay(50);
    
    // Enable RMC message (recommended minimum data)
    uint8_t cfg_rmc[8] = {0xF0, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00};
    GPS_SendUBX(0x06, 0x01, cfg_rmc, sizeof(cfg_rmc));
    delay(50);
    
    // Enable GSV message (satellites in view) - output EVERY epoch for real-time satellite display
    uint8_t cfg_gsv[8] = {0xF0, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00};  // Changed from 0x05 to 0x01
    GPS_SendUBX(0x06, 0x01, cfg_gsv, sizeof(cfg_gsv));
    delay(50);
    
    // Enable GSA message (DOP and active satellites)
    uint8_t cfg_gsa[8] = {0xF0, 0x02, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00};
    GPS_SendUBX(0x06, 0x01, cfg_gsa, sizeof(cfg_gsa));
    delay(50);

    // Enable UBX NAV-PVT so hAcc (horizontal accuracy) is available.
    uint8_t cfg_nav_pvt[8] = {0x01, 0x07, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00};
    GPS_SendUBX(0x06, 0x01, cfg_nav_pvt, sizeof(cfg_nav_pvt));
    delay(50);
}

// ============================================================================

static void GPS_ConfigureBaudRate230400(void) {
    
    // UBX-CFG-PRT: Configure UART port
    // Structure: portID(1), reserved1(1), txReady(2), mode(4), baudRate(4), inProtoMask(2), outProtoMask(2), flags(2), reserved2(2)
    uint8_t cfg_prt[20] = {
        0x01,             // portID = 1 (UART)
        0x00,             // reserved1
        0x00, 0x00,       // txReady
        0xC0, 0x08, 0x00, 0x00,  // mode = 8N1 (8-bit, no parity, 1 stop bit)
        0x00, 0x84, 0x03, 0x00,  // baudRate = 230400 (0x00038400 little-endian)
        0x01, 0x00,       // inProtoMask = NMEA
        0x01, 0x00,       // outProtoMask = NMEA
        0x00, 0x00,       // flags
        0x00, 0x00        // reserved2
    };
    
    GPS_SendUBX(0x06, 0x00, cfg_prt, sizeof(cfg_prt));
    delay(100);
    
    // Save configuration to flash (BBR/Flash/EEPROM/SPI Flash)
    uint8_t cfg_save[12] = {
        0x00, 0x00, 0x00, 0x00,  // clearMask = 0 (don't clear)
        0xFF, 0xFF, 0x00, 0x00,  // saveMask = 0xFFFF (save everything)
        0x00, 0x00, 0x00, 0x00   // loadMask = 0 (don't load now)
    };
    GPS_SendUBX(0x06, 0x09, cfg_save, sizeof(cfg_save));
    delay(100);
    
    // Now restart ESP32 UART at 230400 baud
    gpsSerial.end();
    delay(200);
    gpsSerial.setRxBufferSize(GPS_RX_BUF_SIZE);
    gpsSerial.begin(230400, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
    delay(500);
}

void GPS_Init(void) {
    
    // Simple init like original GPS_Driver - no auto-detection
    gpsSerial.setRxBufferSize(GPS_RX_BUF_SIZE);
    gpsSerial.begin(GPS_BAUD_RATE, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
    
    delay(500);
    
    // Send factory reset to ensure NMEA output is enabled
    uint8_t cfg_reset[13] = {
        0xFF, 0xFF, 0xFF, 0xFF,  // clearMask
        0x00, 0x00, 0x00, 0x00,  // saveMask
        0xFF, 0xFF, 0xFF, 0xFF,  // loadMask
        0x1F                      // deviceMask
    };
    GPS_SendUBX(0x06, 0x09, cfg_reset, sizeof(cfg_reset));
    delay(1000);
    
    // Configure NMEA sentences
    GPS_SendUBX_EnableNMEA();
    delay(200);
    
}

void GPS_ProcessIncoming(void) {
    int available = gpsSerial.available();
    if (available <= 0) return;

    while (gpsSerial.available() > 0) {
        uint8_t raw_byte = gpsSerial.read();
        gps_parse_ubx_byte(raw_byte);
        char c = (char)raw_byte;
        
        if (c == '\r' || c == '\n') {
            if (s_nmea_idx > 0) {
                s_nmea_line[s_nmea_idx] = '\0';
                
                // Validate and parse NMEA sentence
                if (s_nmea_line[0] == '$' && nmea_checksum_valid(s_nmea_line)) {
                    if (nmea_sentence_is_type(s_nmea_line, "GGA")) {
                        gps_set_latest_gga(s_nmea_line);
                        gps_parse_gga_fields(s_nmea_line);
                    }
                    else if (nmea_sentence_is_type(s_nmea_line, "RMC")) {
                        gps_set_latest_line(s_nmea_line);
                        gps_parse_rmc_fields(s_nmea_line);
                    }
                    else if (nmea_sentence_is_type(s_nmea_line, "GSV")) {
                        gps_parse_gsv_fields(s_nmea_line);
                    }
                }
                
                s_nmea_idx = 0;
            }
        }
        else if (s_nmea_idx < (sizeof(s_nmea_line) - 1)) {
            s_nmea_line[s_nmea_idx++] = c;
        }
        else {
            s_nmea_idx = 0; // Buffer overflow, reset
        }
    }
}

void GPS_GetLatestLine(char *out, size_t out_size) {
    if (out == NULL || out_size == 0) return;
    taskENTER_CRITICAL(&s_gps_line_lock);
    snprintf(out, out_size, "%s", s_gps_latest_line);
    taskEXIT_CRITICAL(&s_gps_line_lock);
}

void GPS_GetLatestGGA(char *out, size_t out_size) {
    if (out == NULL || out_size == 0) return;
    taskENTER_CRITICAL(&s_gps_line_lock);
    snprintf(out, out_size, "%s", s_gps_latest_gga);
    taskEXIT_CRITICAL(&s_gps_line_lock);
}

void GPS_GetDebugInfo(gps_debug_info_t *out) {
    if (out == NULL) return;
    taskENTER_CRITICAL(&s_gps_line_lock);
    memcpy(out, &s_gps_debug_info, sizeof(gps_debug_info_t));
    taskEXIT_CRITICAL(&s_gps_line_lock);
}

void GPS_PrintInfo(void) {
}

bool GPS_HasFix(void) {
    return s_gps_fix_valid;
}

float GPS_GetSpeed(void) {
    taskENTER_CRITICAL(&s_gps_line_lock);
    float speed = atof(s_gps_debug_info.speed_kmh);
    taskEXIT_CRITICAL(&s_gps_line_lock);
    return speed;
}

float GPS_GetLatitude(void) {
    taskENTER_CRITICAL(&s_gps_line_lock);
    float lat = atof(s_gps_debug_info.lat);
    taskEXIT_CRITICAL(&s_gps_line_lock);
    return lat;
}

float GPS_GetLongitude(void) {
    taskENTER_CRITICAL(&s_gps_line_lock);
    float lon = atof(s_gps_debug_info.lon);
    taskEXIT_CRITICAL(&s_gps_line_lock);
    return lon;
}

uint8_t GPS_GetSatellites(void) {
    taskENTER_CRITICAL(&s_gps_line_lock);
    uint8_t sats = (uint8_t)atoi(s_gps_debug_info.num_sats);
    taskEXIT_CRITICAL(&s_gps_line_lock);
    return sats;
}

int GPS_GetAvailableBytes(void) {
    return gpsSerial.available();
}

void GPS_PrintRawData(int max_bytes) {
    int available = gpsSerial.available();
    if (available > 0) {
        char buf[129];
        int read_len = gpsSerial.readBytes((uint8_t*)buf, min(available, max_bytes));
        buf[read_len] = '\0';
        
        Serial.print("[GPS RAW] ");
        for (int i = 0; i < read_len; i++) {
            if (buf[i] >= 32 && buf[i] <= 126) {
                Serial.print(buf[i]);
            } else if (buf[i] == '\r') {
                Serial.print("<CR>");
            } else if (buf[i] == '\n') {
                Serial.print("<LF>");
            }
        }
    }
}

bool GPS_AutoDetectBaudRate(void) {
    // Try 230400 first (common for SR1723), then fallback to standard rates
    uint32_t baud_rates[] = {230400, 115200, 9600, 38400, 57600};
    int num_rates = sizeof(baud_rates) / sizeof(baud_rates[0]);
    
    
    for (int i = 0; i < num_rates; i++) {
        uint32_t baud = baud_rates[i];
        
        // Reinitialize UART with new baud rate
        gpsSerial.end();
        delay(100);
        gpsSerial.setRxBufferSize(GPS_RX_BUF_SIZE);
        gpsSerial.begin(baud, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
        delay(3000); // รอ 3 วินาที เพื่อให้แน่ใจว่าได้ NMEA sentences
        
        // Check if data is available
        int available = gpsSerial.available();
        if (available > 0) {
            
            // Read and validate NMEA data
            char test_buf[512];
            int read_len = gpsSerial.readBytes((uint8_t*)test_buf, min(available, 511));
            test_buf[read_len] = '\0';
            
            // Look for NMEA sentence patterns ($GPxxx, $GNxxx, $GLxxx, etc.)
            // Must find at least 2 valid NMEA sentences to confirm
            int nmea_count = 0;
            for (int j = 0; j < read_len - 6; j++) {
                if (test_buf[j] == '$' && 
                    test_buf[j+1] == 'G' && 
                    (test_buf[j+2] == 'P' || test_buf[j+2] == 'N' || 
                     test_buf[j+2] == 'L' || test_buf[j+2] == 'A' || 
                     test_buf[j+2] == 'B' || test_buf[j+2] == 'Q')) {
                    nmea_count++;
                    if (nmea_count >= 2) break;  // เจอ 2 sentences แล้วพอ
                }
            }
            
            if (nmea_count >= 2) {
                Serial.print("[GPS] Sample: ");
                for (int j = 0; j < min(read_len, 80); j++) {
                    if (test_buf[j] >= 32 && test_buf[j] <= 126) {
                        Serial.print(test_buf[j]);
                    }
                }
                // Keep this baud rate - already initialized!
                return true;
            } else {
                Serial.print("[GPS] Sample hex: ");
                for (int j = 0; j < min(read_len, 40); j++) {
                    Serial.print((uint8_t)test_buf[j], HEX);
                    Serial.print(' ');
                }
            }
        }
    }
    
    
    // Try default 230400
    gpsSerial.end();
    delay(100);
    gpsSerial.setRxBufferSize(GPS_RX_BUF_SIZE);
    gpsSerial.begin(230400, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
    
    return false;
}

uint8_t GPS_GetVisibleSatellites(void) {
    taskENTER_CRITICAL(&s_gps_line_lock);
    uint8_t visible = (uint8_t)atoi(s_gps_debug_info.total_sats);
    taskEXIT_CRITICAL(&s_gps_line_lock);
    return visible;
}

uint8_t GPS_GetSatelliteInfo(gps_satellite_t *satellites) {
    if (satellites == NULL) return 0;
    
    taskENTER_CRITICAL(&s_satellite_lock);
    uint8_t count = s_satellite_count;
    if (count > GPS_MAX_SATELLITES) count = GPS_MAX_SATELLITES;
    memcpy(satellites, s_satellites, count * sizeof(gps_satellite_t));
    taskEXIT_CRITICAL(&s_satellite_lock);
    
    return count;
}

const char* GPS_GetQualityStatus(void) {
    taskENTER_CRITICAL(&s_gps_line_lock);
    const uint32_t accuracy_mm = s_gps_debug_info.horizontal_accuracy_mm;  // From UBX NAV-PVT
    taskEXIT_CRITICAL(&s_gps_line_lock);

    // Use UBX NAV-PVT horizontal accuracy only (don't check s_gps_fix_valid)
    // UBX-NAV-PVT is more reliable than NMEA RMC status
    if (accuracy_mm == 0) return "Bad";          // No accuracy data or invalid fix
    if (accuracy_mm <= 500) return "Excellent";  // < 0.5m (RTK level)
    if (accuracy_mm <= 2000) return "Good";      // < 2m (DGPS level)
    if (accuracy_mm <= 10000) return "Fair";     // < 10m (Standard GPS)
    return "Bad";                                 // > 10m (Poor accuracy)
}
