#include "KLine.h"
#include "hardware_config.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "soc/gpio_sig_map.h"
#include "esp_rom_gpio.h"

// KLINE_MODE = 3 (HONDA GIORNO+ / CLICK125 / s4)
#if KLINE_MODE == 1

bool kline_use_handshake = true;   
bool kline_handshake_done = false; 

namespace {

HardwareSerial kline_serial(2);
constexpr uint32_t KLINE_BAUD_RATE = 10400;
constexpr uint32_t KLINE_DATA_TIMEOUT_MS = 500;

// ชุดคำสั่งสำหรับ Giorno+ / Click125 /s4(0x17 - 21 Bytes)
const uint8_t cmd_17[7] = {0x72, 0x07, 0x72, 0x17, 0x00, 0x0F, 0xEF}; 
const uint8_t cmd_20[7] = {0x72, 0x07, 0x72, 0x20, 0x00, 0x02, 0xF3}; // AFR

uint16_t latest_rpm = 0;
int16_t latest_engine_temp = 0;
int16_t latest_map = 0;
int16_t latest_tps = 0;
int16_t latest_ignition = 0;
int16_t latest_tinj = 0;
int16_t latest_speed = 0;
int16_t latest_batt_v = 0;
int16_t latest_afr = 0;
uint16_t latest_inj_duty = 0; 
uint32_t last_valid_data_time = 0;

static portMUX_TYPE kline_mux = portMUX_INITIALIZER_UNLOCKED;

bool checksum_is_valid(const uint8_t* data, size_t length) {
    uint8_t sum = 0;
    for (size_t i = 0; i < length; ++i) {
        sum = static_cast<uint8_t>(sum + data[i]);
    }
    return sum == 0;
}

void send_kline_command(const uint8_t* command, size_t length) {
    kline_serial.write(command, length);
    kline_serial.flush();
}

void KLine_BeginUART(void) {

    kline_serial.setRxBufferSize(512);
    kline_serial.begin(KLINE_BAUD_RATE, SERIAL_8N1, static_cast<int>(KLINE_RX_PIN), -1);
    uart_set_line_inverse(UART_NUM_2, UART_SIGNAL_TXD_INV);
    esp_rom_gpio_connect_out_signal(static_cast<uint32_t>(KLINE_TX_PIN), U2TXD_OUT_IDX, false, false);
}

void KLine_SendWakeupPulse() {

    kline_serial.end();

    pinMode(static_cast<uint8_t>(KLINE_TX_PIN), OUTPUT);
    digitalWrite(static_cast<uint8_t>(KLINE_TX_PIN), HIGH);
    delay(70);
    
    digitalWrite(static_cast<uint8_t>(KLINE_TX_PIN), LOW);
    delay(125);

    KLine_BeginUART();
    delay(10);
    while (kline_serial.available()) kline_serial.read();
}

}  // namespace

bool KLine_SendHandshake() {
    const uint8_t cmd_step1[4] = {0xFE, 0x04, 0x72, 0x8C}; 
    const uint8_t cmd_step2[5] = {0x72, 0x05, 0x00, 0xF0, 0x99}; 
    const uint8_t expected_step1[4] = {0x0E, 0x04, 0x72, 0x7C};
    const uint8_t expected_step2[4] = {0x02, 0x04, 0x00, 0xFA};

    bool step1_success = false;
    constexpr uint8_t total_rounds = 10;

    for (uint8_t round = 0; round < total_rounds && !step1_success; round++) {
        const uint8_t step1_attempts = ((round % 2) == 0) ? 5U : 1U;
        KLine_SendWakeupPulse();
        delay(5);

        for (uint8_t attempt = 0; attempt < step1_attempts && !step1_success; attempt++) {
            Serial.printf("  -> Attempt %d/%d...\n", attempt + 1, step1_attempts);
            
            while (kline_serial.available()) kline_serial.read();
            
            kline_serial.write(cmd_step1, 4);
            kline_serial.flush();

            uint32_t wait_start = millis();
            uint8_t rx_buf[64];
            size_t rx_len = 0;

            while (millis() - wait_start < 150) {
                while (kline_serial.available() && rx_len < sizeof(rx_buf)) {
                    rx_buf[rx_len++] = static_cast<uint8_t>(kline_serial.read());
                }
                vTaskDelay(1);
            }

            if (rx_len > 0) {
                Serial.print("  [DEBUG] RAW RX:");
                for (size_t i = 0; i < rx_len; i++) {
                    Serial.printf(" %02X", rx_buf[i]);
                }
                Serial.println();
            }

            for (size_t i = 0; i < rx_len; i++) {
                if (i + 3 < rx_len &&
                    rx_buf[i] == expected_step1[0] &&
                    rx_buf[i+1] == expected_step1[1] &&
                    rx_buf[i+2] == expected_step1[2] &&
                    rx_buf[i+3] == expected_step1[3]) {
                    step1_success = true;
                    break;
                }
            }
            if (!step1_success && attempt < step1_attempts - 1) delay(10);
        }

        if (!step1_success && round < total_rounds - 1) {
            delay(1010);
        }
    }

    if (!step1_success) {
        return false;
    }

    delay(30);

    bool step2_success = false;
    for (uint8_t attempt = 0; attempt < 5 && !step2_success; attempt++) {
        Serial.printf("\n[Handshake] Step 2 Attempt %d/5...\n", attempt + 1);
        
        while (kline_serial.available()) kline_serial.read();
        
        kline_serial.write(cmd_step2, 5);
        kline_serial.flush();

        uint32_t wait_start = millis();
        uint8_t rx_buf[64];
        size_t rx_len = 0;

        while (millis() - wait_start < 150) {
            while (kline_serial.available() && rx_len < sizeof(rx_buf)) {
                rx_buf[rx_len++] = static_cast<uint8_t>(kline_serial.read());
            }
            vTaskDelay(1);
        }

        if (rx_len > 0) {
            Serial.print("  [DEBUG] RAW RX:");
            for (size_t i = 0; i < rx_len; i++) {
                Serial.printf(" %02X", rx_buf[i]);
            }
            Serial.println();
        }

        for (size_t i = 0; i < rx_len; i++) {
            if (i + 3 < rx_len &&
                rx_buf[i] == expected_step2[0] &&
                rx_buf[i+1] == expected_step2[1] &&
                rx_buf[i+2] == expected_step2[2] &&
                rx_buf[i+3] == expected_step2[3]) {
                step2_success = true;
                break;
            }
        }
    }

    if (!step2_success) {
        return false;
    }

    return true;
}

void KLine_Init(void) {
    pinMode(static_cast<uint8_t>(KLINE_RX_PIN), INPUT);
    
    
    if (kline_use_handshake && !kline_handshake_done) {
        if (KLine_SendHandshake()) {
            kline_handshake_done = true;
        } else {
            kline_serial.end();
        }
    } else {
        KLine_BeginUART();
    }
}

void reset_kline_uart(void) {
    kline_serial.end();
    KLine_BeginUART();
    while (kline_serial.available()) kline_serial.read();
}

void KLine_Poll(void) {

    if (kline_use_handshake && !kline_handshake_done) {
        static uint32_t last_retry_time = 0;
        if (millis() - last_retry_time > 2000) {
            last_retry_time = millis();
            Serial.println("[POLL] Attempting Handshake Recovery...");
            KLine_BeginUART();
            if (KLine_SendHandshake()) {
                kline_handshake_done = true;
            } else {
                kline_serial.end(); 
            }
        }
        return; 
    }

    static uint32_t last_request_time = 0;
    static bool request_engine = false;
    static uint8_t failed_data_frames = 0;

    // หน่วงเวลาขอข้อมูลทุก 50ms
    if (millis() - last_request_time < 50) return;
    last_request_time = millis();

    while(kline_serial.available()) kline_serial.read();

    const uint8_t* command = request_engine ? cmd_17 : cmd_20;
    send_kline_command(command, 7);

    uint32_t wait_start = millis();
    uint8_t rx_buf[128];
    size_t rx_len = 0;
    bool valid_response = false;

    while (millis() - wait_start < 100) {
        while (kline_serial.available() && rx_len < sizeof(rx_buf)) {
            rx_buf[rx_len++] = static_cast<uint8_t>(kline_serial.read());
        }
        vTaskDelay(1);
    }

    if (rx_len > 0) {
        for (size_t i = 0; i < rx_len; i++) {

            if (rx_buf[i] == 0x02) { 
                uint8_t pkt_len = rx_buf[i+1]; 
                if (pkt_len >= 8 && pkt_len <= 32 && (i + pkt_len) <= rx_len) {
                    
                    if (checksum_is_valid(&rx_buf[i], pkt_len)) {
                        portENTER_CRITICAL(&kline_mux);
                        
                        // ถ้ารอบนี้ขอคำสั่ง 0x17 (RPM, Temp, TPS) ความยาวต้องเป็น 21 ไบต์
                        if (request_engine && pkt_len == 21 && rx_buf[i+2] == 0x72 && rx_buf[i+3] == 0x17) {
                            latest_rpm = (static_cast<uint16_t>(rx_buf[i+5]) << 8) | rx_buf[i+6];
                            latest_tps = rx_buf[i+8] / 0.18f;
                            latest_engine_temp = (rx_buf[i+10] == 0xFF) ? -41 : rx_buf[i+10];
                            latest_map = 0;
                            latest_speed = 0;

                            const uint16_t raw_tinj = (static_cast<uint16_t>(rx_buf[i+16]) << 8) | rx_buf[i+17];
                            latest_tinj = raw_tinj / 25.0f;
                            latest_ignition = (static_cast<uint16_t>(rx_buf[i+18]) * 5) - 640;

                            if (latest_rpm > 0) {
                                const uint32_t calculated_duty = (static_cast<uint32_t>(latest_rpm) * raw_tinj) / 300000;
                                latest_inj_duty = calculated_duty > 100 ? 100 : static_cast<uint16_t>(calculated_duty);
                            } else {
                                latest_inj_duty = 0;
                            }
                            valid_response = true;
                        } 

                        else if (!request_engine && pkt_len == 8 && rx_buf[i+2] == 0x72 && rx_buf[i+3] == 0x20) {
                            latest_afr = rx_buf[i+5];
                            valid_response = true;
                        }
                        
                        portEXIT_CRITICAL(&kline_mux);
                        
                        if (valid_response) break;
                    }
                }
            }
        }
    }

    // 3. จัดการกรณีข้อมูลขาดหาย
    if (valid_response) {
        last_valid_data_time = millis();
    } else {
        if (++failed_data_frames >= 50) {
            Serial.printf("[KLINE] Data lost %u times! Forcing Handshake Restart...\n", failed_data_frames);
            reset_kline_uart();
            failed_data_frames = 0;
            request_engine = true;
            kline_handshake_done = false;
            return;
        }
    }

    request_engine = !request_engine; // สลับคำสั่ง 0x17 และ 0x20 ในรอบถัดไป
}

KLineData KLine_GetData(void) {
    static bool stale_logged = false;
    const uint32_t data_age = millis() - last_valid_data_time;
    if (data_age > KLINE_DATA_TIMEOUT_MS) {
        if (!stale_logged) {
            Serial.printf("[KLINE] No valid frame for %lu ms; returning zero data\n",
                          static_cast<unsigned long>(data_age));
            stale_logged = true;
        }
        return {0, 0, 0, 0, 0, 0, 0, 0, 0};  
    }
    stale_logged = false;

    KLineData data;
    portENTER_CRITICAL(&kline_mux);
    data = {latest_rpm, latest_engine_temp, latest_map, latest_tps,
            latest_ignition, latest_afr, latest_tinj, latest_speed, latest_inj_duty};  
    portEXIT_CRITICAL(&kline_mux);
    
    return data;
}

#endif // KLINE_MODE == 3