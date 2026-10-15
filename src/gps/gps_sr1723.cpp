
#include "gps_sr1723.h"
#include "hardware_config.h"
#include <Arduino.h>
#include <HardwareSerial.h>

// UART instance
static HardwareSerial GPSSerial(GPS_UART_NUM);

// GPS data storage (protected by mutex)
static portMUX_TYPE s_gps_data_lock = portMUX_INITIALIZER_UNLOCKED;
static portMUX_TYPE s_satellite_lock = portMUX_INITIALIZER_UNLOCKED;

static double s_latitude = 0.0;
static double s_longitude = 0.0;
static float s_altitude_m = 0.0f;
static float s_speed_kmh = 0.0f;
static uint8_t s_num_satellites = 0;
static gps_satellite_t s_satellites[GPS_MAX_SATELLITES];
static uint8_t s_satellite_count = 0;
static bool s_gps_fix_valid = false;
static uint32_t s_gps_updated_at_ms = 0;

// Time/Date from UBX-NAV-PVT
static uint16_t s_gps_year = 0;
static uint8_t s_gps_month = 0;
static uint8_t s_gps_day = 0;
static uint8_t s_gps_hour = 0;
static uint8_t s_gps_minute = 0;
static uint8_t s_gps_second = 0;

// Satellite signal strength data (from UBX-NAV-SAT)
static gps_debug_info_t s_gps_debug_info = {0};

// UBX protocol state machine
static uint8_t s_ubx_state = 0;
static uint8_t s_ubx_class = 0;
static uint8_t s_ubx_id = 0;
static uint16_t s_ubx_length = 0;
static uint16_t s_ubx_payload_counter = 0;
static uint8_t s_ubx_payload[1024];  
static uint8_t s_ubx_ck_a = 0;
static uint8_t s_ubx_ck_b = 0;

// Helper functions for Little-Endian parsing
static inline uint16_t read_u16_le(const uint8_t *buf) {
    return (uint16_t)buf[0] | ((uint16_t)buf[1] << 8);
}

static inline int16_t read_i16_le(const uint8_t *buf) {
    return (int16_t)read_u16_le(buf);
}

static inline uint32_t read_u32_le(const uint8_t *buf) {
    return (uint32_t)buf[0] | ((uint32_t)buf[1] << 8) | 
           ((uint32_t)buf[2] << 16) | ((uint32_t)buf[3] << 24);
}

static inline int32_t read_i32_le(const uint8_t *buf) {
    return (int32_t)read_u32_le(buf);
}

// ---------------------------------------------------------
// UBX Parsers
// ---------------------------------------------------------

static void gps_parse_ubx_nav_pvt(const uint8_t *payload, uint16_t length) {
    if (length < 92) return;  
    
    // Parse time/date
    uint16_t year = read_u16_le(&payload[4]);
    uint8_t month = payload[6];
    uint8_t day = payload[7];
    uint8_t hour = payload[8];
    uint8_t minute = payload[9];
    uint8_t second = payload[10];
    
    // Convert UTC to UTC+7 (Thailand Time)
    hour += 7;
    if (hour >= 24) {
        hour -= 24;
        day += 1;
    }
    
    // Parse fix info
    uint8_t fix_type = payload[20];   
    uint8_t flags = payload[21];      
    uint8_t num_sv = payload[23];     
    
    int32_t lon_raw = read_i32_le(&payload[24]);  
    int32_t lat_raw = read_i32_le(&payload[28]);  
    int32_t alt_raw = read_i32_le(&payload[36]);  
    
    int32_t vel_n = read_i32_le(&payload[48]);  
    int32_t vel_e = read_i32_le(&payload[52]);  
    
    uint32_t h_acc = read_u32_le(&payload[40]);  
    
    // Calculations
    float speed_ms = sqrtf((float)(vel_n * vel_n + vel_e * vel_e)) / 1000.0f;  
    float speed_kmh = speed_ms * 3.6f;  
    
    double latitude = (double)lat_raw / 10000000.0;
    double longitude = (double)lon_raw / 10000000.0;
    float altitude = (float)alt_raw / 1000.0f;  
    
    bool fix_valid = (flags & 0x01) && (fix_type >= 2);
    
    // Update global state
    taskENTER_CRITICAL(&s_gps_data_lock);
    
    s_gps_year = year;
    s_gps_month = month;
    s_gps_day = day;
    s_gps_hour = hour;
    s_gps_minute = minute;
    s_gps_second = second;
    
    s_latitude = latitude;
    s_longitude = longitude;
    s_altitude_m = altitude;
    s_speed_kmh = speed_kmh;
    s_num_satellites = num_sv;
    s_gps_fix_valid = fix_valid;
    s_gps_updated_at_ms = millis();
    
    snprintf(s_gps_debug_info.lat, sizeof(s_gps_debug_info.lat), "%.6f", latitude);
    snprintf(s_gps_debug_info.lon, sizeof(s_gps_debug_info.lon), "%.6f", longitude);
    snprintf(s_gps_debug_info.altitude_m, sizeof(s_gps_debug_info.altitude_m), "%.1f", altitude);
    snprintf(s_gps_debug_info.speed_kmh, sizeof(s_gps_debug_info.speed_kmh), "%.2f", speed_kmh);
    snprintf(s_gps_debug_info.num_sats, sizeof(s_gps_debug_info.num_sats), "%u", num_sv);
    snprintf(s_gps_debug_info.utc_time, sizeof(s_gps_debug_info.utc_time), "%02u:%02u:%02u", hour, minute, second);
    snprintf(s_gps_debug_info.utc_date, sizeof(s_gps_debug_info.utc_date), "%02u/%02u/%04u", day, month, year);
    
    s_gps_debug_info.horizontal_accuracy_mm = h_acc;
    s_gps_debug_info.fix_quality = fix_type;
    
    taskEXIT_CRITICAL(&s_gps_data_lock);
    
}

static uint8_t ubx_gnss_id_to_constellation(uint8_t gnss_id) {
    switch (gnss_id) {
        case 0: return 1;  // GPS
        case 1: return 6;  // SBAS (WAAS/EGNOS/MSAS)
        case 2: return 3;  // Galileo
        case 3: return 4;  // BeiDou
        case 5: return 5;  // QZSS
        case 6: return 2;  // GLONASS
        default: return 0; // Unknown (skip these)
    }
}

static void gps_parse_ubx_nav_sat(const uint8_t *payload, uint16_t length) {
    if (length < 8) return;
    
    uint8_t num_svs = payload[5];
    
    if (num_svs == 0 || length < (8 + num_svs * 12)) return;
    
    taskENTER_CRITICAL(&s_satellite_lock);
    s_satellite_count = 0;
    memset(s_satellites, 0, sizeof(s_satellites));
    taskEXIT_CRITICAL(&s_satellite_lock);
    
    uint8_t visible_count = 0;
    
    for (uint8_t i = 0; i < num_svs && i < GPS_MAX_SATELLITES; i++) {
        uint16_t offset = 8 + (i * 12);
        if (offset + 12 > length) break;
        
        uint8_t gnss_id = payload[offset + 0];
        uint8_t sv_id = payload[offset + 1];
        uint8_t cno = payload[offset + 2];        
        int8_t elev = (int8_t)payload[offset + 3]; 
        int16_t azim = read_i16_le(&payload[offset + 4]);  
        
        if (cno == 0) continue;
        
        // Filter out low elevation satellites (Horizon multipath rejection)
        if (elev < 5) continue;
        
        uint8_t constellation = ubx_gnss_id_to_constellation(gnss_id);
        if (constellation == 0) continue;

        taskENTER_CRITICAL(&s_satellite_lock);
        if (visible_count < GPS_MAX_SATELLITES) {
            s_satellites[visible_count].prn = sv_id;
            s_satellites[visible_count].elevation = (elev < 0) ? 0 : (uint8_t)elev;
            s_satellites[visible_count].azimuth = (azim < 0) ? 0 : (uint16_t)azim;
            s_satellites[visible_count].snr = cno;  
            s_satellites[visible_count].constellation = constellation;
            visible_count++;
        }
        taskEXIT_CRITICAL(&s_satellite_lock);
    }

    taskENTER_CRITICAL(&s_satellite_lock);
    s_satellite_count = visible_count;
    taskEXIT_CRITICAL(&s_satellite_lock);

    taskENTER_CRITICAL(&s_gps_data_lock);
    snprintf(s_gps_debug_info.total_sats, sizeof(s_gps_debug_info.total_sats), "%u", visible_count);
    taskEXIT_CRITICAL(&s_gps_data_lock);
}

static void gps_parse_ubx_byte(uint8_t byte) {
    switch (s_ubx_state) {
        case 0: if (byte == 0xB5) s_ubx_state = 1; break;
        case 1: 
            if (byte == 0x62) {
                s_ubx_state = 2; s_ubx_ck_a = 0; s_ubx_ck_b = 0;
            } else s_ubx_state = 0;
            break;
        case 2: s_ubx_class = byte; s_ubx_ck_a += byte; s_ubx_ck_b += s_ubx_ck_a; s_ubx_state = 3; break;
        case 3: s_ubx_id = byte; s_ubx_ck_a += byte; s_ubx_ck_b += s_ubx_ck_a; s_ubx_state = 4; break;
        case 4: s_ubx_length = byte; s_ubx_ck_a += byte; s_ubx_ck_b += s_ubx_ck_a; s_ubx_state = 5; break;
        case 5: 
            s_ubx_length |= ((uint16_t)byte << 8);
            s_ubx_ck_a += byte; s_ubx_ck_b += s_ubx_ck_a;
            s_ubx_payload_counter = 0;
            s_ubx_state = (s_ubx_length > 0) ? 6 : 7;
            break;
        case 6: 
            // Store byte in buffer only if there's space, but always increment counter
            if (s_ubx_payload_counter < sizeof(s_ubx_payload)) {
                s_ubx_payload[s_ubx_payload_counter] = byte;
            }
            s_ubx_payload_counter++;
            s_ubx_ck_a += byte; s_ubx_ck_b += s_ubx_ck_a;
            if (s_ubx_payload_counter >= s_ubx_length) s_ubx_state = 7;
            break;
        case 7: s_ubx_state = (byte == s_ubx_ck_a) ? 8 : 0; break;
        case 8:
            if (byte == s_ubx_ck_b) {
                // Only parse if payload was fully captured in buffer
                if (s_ubx_length <= sizeof(s_ubx_payload)) {
                    if (s_ubx_class == 0x01 && s_ubx_id == 0x07) gps_parse_ubx_nav_pvt(s_ubx_payload, s_ubx_length);
                    else if (s_ubx_class == 0x01 && s_ubx_id == 0x35) gps_parse_ubx_nav_sat(s_ubx_payload, s_ubx_length);
                }
            }
            s_ubx_state = 0;
            break;
    }
}

static void GPS_SendUBX(uint8_t msg_class, uint8_t msg_id, const uint8_t *payload, uint16_t length) {
    uint8_t ck_a = 0, ck_b = 0;
    
    GPSSerial.write(0xB5); GPSSerial.write(0x62);
    
    GPSSerial.write(msg_class); ck_a += msg_class; ck_b += ck_a;
    GPSSerial.write(msg_id); ck_a += msg_id; ck_b += ck_a;
    
    GPSSerial.write((uint8_t)(length & 0xFF)); ck_a += (length & 0xFF); ck_b += ck_a;
    GPSSerial.write((uint8_t)(length >> 8)); ck_a += (length >> 8); ck_b += ck_a;
    
    for (uint16_t i = 0; i < length; i++) {
        GPSSerial.write(payload[i]);
        ck_a += payload[i]; ck_b += ck_a;
    }
    
    GPSSerial.write(ck_a); GPSSerial.write(ck_b);
}

// M10 Advanced Configuration Function
static void GPS_ConfigurePureUBX() {
    uint8_t cfg_valset[] = {
        0x00,                   // Version
        0x01,                   // Layer: RAM + BBR + Flash (มีผลทันทีและจำค่าไว้ตอนปิดเครื่อง)
        0x00, 0x00,             // Reserved

        // 1. Protocol Configuration
        // CFG-UART1OUTPROT-NMEA (0x10740002) -> 0 (Disable NMEA)
        0x02, 0x00, 0x74, 0x10, 
        0x00,
        // CFG-UART1OUTPROT-UBX (0x10740001) -> 1 (Enable UBX)
        0x01, 0x00, 0x74, 0x10, 
        0x01,

        // 2. Rate Configuration
        // CFG-RATE-MEAS (0x30210001) -> 100 ms (10 Hz)
        0x01, 0x00, 0x21, 0x30, 
        0x64, 0x00,
        // CFG-RATE-NAV (0x30210002) -> 1 cycle
        0x02, 0x00, 0x21, 0x30, 
        0x01, 0x00,
        // CFG-RATE-TIMEREF (0x20210003) -> 1 (GPS Time)
        0x03, 0x00, 0x21, 0x20, 
        0x01,

        // 3. Navigation Configuration
        // CFG-NAVSPG-DYNMODEL (0x20110021) -> 4 (Automotive)
        0x21, 0x00, 0x11, 0x20, 
        0x04,
        // CFG-NAVSPG-INFIL_MINELEV (0x201100a4) -> 5 degrees
        0xA4, 0x00, 0x11, 0x20, 
        0x05,

        // 4. Constellation Configuration 
// CFG-SIGNAL-GPS_ENA (0x1031001f) -> 1 (GPS)
        0x1F, 0x00, 0x31, 0x10, 0x01,
        // CFG-SIGNAL-SBAS_ENA (0x10310020) -> 1 (SBAS)
        0x20, 0x00, 0x31, 0x10, 0x01,
        // CFG-SIGNAL-GAL_ENA (0x10310021) -> 1 (Galileo)
        0x21, 0x00, 0x31, 0x10, 0x01,
        // CFG-SIGNAL-BDS_ENA (0x10310022) -> 1 (BeiDou)
        0x22, 0x00, 0x31, 0x10, 0x01,
        // CFG-SIGNAL-BDS_B1_ENA (0x1031000d) -> 0 (B1I disabled) // 1561.098 MHz
        0x0D, 0x00, 0x31, 0x10, 0x00,
        // CFG-SIGNAL-BDS_B1C_ENA (0x1031000f) -> 1 (B1C enabled) // 1575.42 MHz
        0x0F, 0x00, 0x31, 0x10, 0x01,
        // CFG-SIGNAL-QZSS_ENA (0x10310024) -> 1 (QZSS)
        0x24, 0x00, 0x31, 0x10, 0x01,
        // CFG-SIGNAL-GLO_ENA (0x10310025) -> 1 (GLONASS)
        0x25, 0x00, 0x31, 0x10, 0x01,


        // 5. Message Rate Configuration
        // CFG-MSGOUT-UBX_NAV_PVT_UART1 (0x20910007) -> 1 (10 Hz)
        0x07, 0x00, 0x91, 0x20, 
        0x01,
        // CFG-MSGOUT-UBX_NAV_SAT_UART1 (0x20910016) 
        0x16, 0x00, 0x91, 0x20, 
        0x0A
    };

    GPS_SendUBX(0x06, 0x8A, cfg_valset, sizeof(cfg_valset));
    delay(500); // Allow time for flash write

    // External Antenna Configuration (If needed by hardware)
    uint8_t cfg_ant_valset[] = {
        0x00, 0x07, 0x00, 0x00,
        // CFG-HW-ANT_CFG_VOLTCTRL (0x10a3002e) -> 1
        0x2e, 0x00, 0xa3, 0x10, 0x01,
        // CFG-HW-ANT_CFG_SHORTDET (0x10a3002f) -> 1
        0x2f, 0x00, 0xa3, 0x10, 0x01
    };
    GPS_SendUBX(0x06, 0x8A, cfg_ant_valset, sizeof(cfg_ant_valset));
    delay(100);

}

static void GPS_SetBaudRate(uint32_t baud) {
    // M10 Baud Rate Configuration (CFG-UART1-BAUDRATE: 0x40520001)
    uint8_t cfg_baud[] = {
        0x00, 0x07, 0x00, 0x00,
        0x01, 0x00, 0x52, 0x40, 
        (uint8_t)(baud & 0xFF), (uint8_t)((baud >> 8) & 0xFF), 
        (uint8_t)((baud >> 16) & 0xFF), (uint8_t)((baud >> 24) & 0xFF)
    };

    GPS_SendUBX(0x06, 0x8A, cfg_baud, sizeof(cfg_baud));
    delay(200);

    GPSSerial.updateBaudRate(baud);
    delay(200);
}

bool GPS_Init(void) {
    const uint32_t baud_rates[] = {230400, 115200, 38400, 9600};
    bool detected = false;
    uint32_t detected_baud = 0;
    
    for (int i = 0; i < 4; i++) {
        GPSSerial.begin(baud_rates[i], SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
        delay(500);
        
        int count = 0;
        uint8_t valid_bytes = 0;
        for (int j = 0; j < 100; j++) {
            while (GPSSerial.available()) {
                uint8_t b = GPSSerial.read();
                count++;
                if (b == '$' || b == 0xB5) valid_bytes++;
            }
            delay(10);
        }
        
        if (count > 10 && valid_bytes > 0) {
            detected = true;
            detected_baud = baud_rates[i];
            break;
        }
    }
    
    if (!detected) {
        return false;
    }
    
    if (detected_baud != GPS_BAUD_RATE) {
        GPS_SetBaudRate(GPS_BAUD_RATE);
    }

    GPS_ConfigurePureUBX();
 
    return true;
}

void GPS_Update(void) {
    while (GPSSerial.available()) {
        gps_parse_ubx_byte(GPSSerial.read());
    }
}

// ---------------------------------------------------------
// Getters
// ---------------------------------------------------------

bool GPS_HasFix(void) {
    taskENTER_CRITICAL(&s_gps_data_lock);
    bool fix = s_gps_fix_valid;
    taskEXIT_CRITICAL(&s_gps_data_lock);
    return fix;
}

void GPS_GetPosition(double *lat, double *lon, float *alt) {
    taskENTER_CRITICAL(&s_gps_data_lock);
    if (lat) *lat = s_latitude;
    if (lon) *lon = s_longitude;
    if (alt) *alt = s_altitude_m;
    taskEXIT_CRITICAL(&s_gps_data_lock);
}

void GPS_GetFix(gps_fix_t *out) {
    if (!out) return;
    taskENTER_CRITICAL(&s_gps_data_lock);
    out->latitude_deg = s_latitude;
    out->longitude_deg = s_longitude;
    out->speed_kmh = s_speed_kmh;
    out->horizontal_accuracy_mm = s_gps_debug_info.horizontal_accuracy_mm;
    out->updated_at_ms = s_gps_updated_at_ms;
    out->valid = s_gps_fix_valid;
    taskEXIT_CRITICAL(&s_gps_data_lock);
}

float GPS_GetSpeed(void) {
    taskENTER_CRITICAL(&s_gps_data_lock);
    float speed = s_speed_kmh;
    taskEXIT_CRITICAL(&s_gps_data_lock);
    return speed;
}

uint8_t GPS_GetNumSatellites(void) {
    taskENTER_CRITICAL(&s_gps_data_lock);
    uint8_t num = s_num_satellites;
    taskEXIT_CRITICAL(&s_gps_data_lock);
    return num;
}

uint8_t GPS_GetSatelliteInfo(gps_satellite_t *satellites) {
    taskENTER_CRITICAL(&s_satellite_lock);
    uint8_t count = s_satellite_count;
    if (satellites && count > 0) {
        memcpy(satellites, s_satellites, count * sizeof(gps_satellite_t));
    }
    taskEXIT_CRITICAL(&s_satellite_lock);
    return count;
}

void GPS_GetDebugInfo(gps_debug_info_t *info) {
    if (!info) return;
    taskENTER_CRITICAL(&s_gps_data_lock);
    memcpy(info, &s_gps_debug_info, sizeof(gps_debug_info_t));
    taskEXIT_CRITICAL(&s_gps_data_lock);
}

void GPS_ProcessIncoming(void) { GPS_Update(); }
uint8_t GPS_GetSatellites(void) { return GPS_GetNumSatellites(); }

uint8_t GPS_GetVisibleSatellites(void) {
    taskENTER_CRITICAL(&s_gps_data_lock);
    uint8_t total = (uint8_t)atoi(s_gps_debug_info.total_sats);
    taskEXIT_CRITICAL(&s_gps_data_lock);
    return total;
}

const char* GPS_GetQualityStatus(void) {
    uint32_t h_acc;
    taskENTER_CRITICAL(&s_gps_data_lock);
    h_acc = s_gps_debug_info.horizontal_accuracy_mm;
    bool has_fix = s_gps_fix_valid;
    taskEXIT_CRITICAL(&s_gps_data_lock);
    
    if (!has_fix) return "Bad";
    float h_acc_m = h_acc / 1000.0f;
    if (h_acc_m < 2.0f) return "Excellent";
    else if (h_acc_m < 5.0f) return "Good";
    else if (h_acc_m < 10.0f) return "Moderate";
    else if (h_acc_m < 20.0f) return "Fair";
    else return "Bad";
}

float GPS_GetLatitude(void) {
    double lat, lon; float alt;
    GPS_GetPosition(&lat, &lon, &alt);
    return (float)lat;
}

float GPS_GetLongitude(void) {
    double lat, lon; float alt;
    GPS_GetPosition(&lat, &lon, &alt);
    return (float)lon;
}