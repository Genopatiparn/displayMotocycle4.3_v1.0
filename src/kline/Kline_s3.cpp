#include "KLine.h"
#include "hardware_config.h"
#include "driver/uart.h"

//KLINE_MODE = 0 
#if KLINE_MODE == 0

// ตัวแปรควบคุม Handshake (global - ใช้ได้ทุกไฟล์)
bool kline_use_handshake = true;   // เปิดใช้งาน Handshake โดยค่าเริ่มต้น
bool kline_handshake_done = false; // ยังไม่ได้ทำ Handshake

namespace {

HardwareSerial kline_serial(2);
constexpr uint32_t KLINE_BAUD_RATE = 10400;
constexpr uint32_t KLINE_DATA_TIMEOUT_MS = 500;

// ชุดคำสั่งสำหรับถาม ECU
const uint8_t cmd_11[7] = {0x72, 0x07, 0x72, 0x11, 0x00, 0x14, 0xF0}; 
const uint8_t cmd_20[7] = {0x72, 0x07, 0x72, 0x20, 0x00, 0x02, 0xF3}; //  AFR

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

// Mutex for protecting K-Line data from concurrent access (Core 0 writes, Core 1 reads)
static portMUX_TYPE kline_mux = portMUX_INITIALIZER_UNLOCKED;

bool checksum_is_valid(const uint8_t* data, size_t length) {
    uint8_t sum = 0;
    for (size_t i = 0; i < length; ++i) {
        sum = static_cast<uint8_t>(sum + data[i]);
    }
    return sum == 0;
}

void send_kline_command(const uint8_t* command, size_t length, uint8_t command_id) {
    kline_serial.write(command, length);
    kline_serial.flush();
}

}  // namespace

// ==========================================
// ฟังก์ชัน Handshake - ปลุก ECU (5-baud init)
// ==========================================
bool KLine_SendHandshake() {
    const uint8_t cmd_read_id[5] = {0x72, 0x05, 0x73, 0x00, 0x16}; 
    
    // ล้าง buffer
    while (kline_serial.available()) kline_serial.read();

    // ปิด UART ชั่วคราวเพื่อควบคุม K-Line ด้วย GPIO
    kline_serial.end(); 
    gpio_reset_pin(static_cast<gpio_num_t>(KLINE_TX_PIN));  // ล้างการตั้งค่า UART
    pinMode(static_cast<uint8_t>(KLINE_TX_PIN), OUTPUT);
    
    // 5-baud init pulse: 70ms HIGH, 120ms LOW
    digitalWrite(static_cast<uint8_t>(KLINE_TX_PIN), HIGH); 
    delay(70);
    digitalWrite(static_cast<uint8_t>(KLINE_TX_PIN), LOW);  
    delay(120);

    // เปิด UART กลับมาใช้งาน
    kline_serial.begin(KLINE_BAUD_RATE, SERIAL_8N1, static_cast<int>(KLINE_RX_PIN), static_cast<int>(KLINE_TX_PIN));
    uart_set_line_inverse(UART_NUM_2, UART_SIGNAL_TXD_INV);

    // ส่งคำสั่ง READ_ID
    kline_serial.write(cmd_read_id, sizeof(cmd_read_id));
    kline_serial.flush();

    // รอรับ response จาก ECU
    const uint32_t wait_start = millis();
    uint8_t response[32] = {};
    uint8_t length = 0;
    uint8_t expected_length = 0;
    uint8_t echo_length = 0;
    bool receiving_echo = false;

    while (millis() - wait_start < 150) {
        if (!kline_serial.available()) continue;
        const uint8_t value = static_cast<uint8_t>(kline_serial.read());

        // ข้าม echo ของคำสั่งที่ส่งไป
        if (length == 0 && !receiving_echo) {
            if (value == 0x72) { receiving_echo = true; echo_length = 1; continue; }
            if (value != 0x02) continue;
        }

        if (receiving_echo) {
            if (++echo_length >= sizeof(cmd_read_id)) { receiving_echo = false; echo_length = 0; }
            continue;
        }

        if (length >= sizeof(response)) break;
        response[length++] = value;
        
        if (length == 2) {
            expected_length = response[1];
            if (expected_length < 4 || expected_length > sizeof(response)) break;
        }
        
        if (expected_length > 0 && length == expected_length) {
            // ตรวจสอบ checksum
            if (checksum_is_valid(response, length)) {
                return true;
            }
            return false;
        }
    }
    return false;
}

void KLine_Init(void) {
    pinMode(static_cast<uint8_t>(KLINE_TX_PIN), OUTPUT); 
    pinMode(static_cast<uint8_t>(KLINE_RX_PIN), INPUT);
    
    // กำหนด UART buffer ขนาดใหญ่พอ - ป้องกัน overflow
    kline_serial.setRxBufferSize(512);
    kline_serial.begin(KLINE_BAUD_RATE, SERIAL_8N1, static_cast<int>(KLINE_RX_PIN), static_cast<int>(KLINE_TX_PIN));
    uart_set_line_inverse(UART_NUM_2, UART_SIGNAL_TXD_INV);
    // ถ้าเปิดใช้งาน Handshake ให้ปลุก ECU ก่อน
    if (kline_use_handshake && !kline_handshake_done) {
        if (KLine_SendHandshake()) {
            kline_handshake_done = true;
        }
    }
}

void reset_kline_uart(void) {
    kline_serial.end();
    kline_serial.begin(KLINE_BAUD_RATE, SERIAL_8N1,
                      static_cast<int>(KLINE_RX_PIN), static_cast<int>(KLINE_TX_PIN));
    uart_set_line_inverse(UART_NUM_2, UART_SIGNAL_TXD_INV);
    while (kline_serial.available()) {
        kline_serial.read();
    }
}

void KLine_Poll(void) {
    static uint32_t last_request_time = 0;
    static bool request_11 = true; 
    static uint8_t failed_data_frames = 0;

    if (millis() - last_request_time < 50) return;
    last_request_time = millis();

    while(kline_serial.available()) kline_serial.read();

    const uint8_t command_id = request_11 ? 0x11 : 0x20;
    if (request_11) {
        send_kline_command(cmd_11, sizeof(cmd_11), command_id);
    } else {
        send_kline_command(cmd_20, sizeof(cmd_20), command_id);
    }

    uint8_t resp[32];
    uint8_t len = 0;
    uint8_t expected_len = 0;
    bool receiving_echo = false;
    bool valid_response = false;
    uint32_t wait_start = millis();

    // เพิ่ม timeout เป็น 100ms เพื่อรองรับ baud rate ช้า
    while (millis() - wait_start < 100) {
        if (kline_serial.available()) {
            uint8_t c = kline_serial.read();
            
            if (len == 0) {
                if (c == 0x72) receiving_echo = true;
                else if (c == 0x02) receiving_echo = false;
                else continue; 
            }

            if (receiving_echo) {
                len++;
                if (len == 7) len = 0; 
            } 
            else {
                resp[len++] = c;
                if (len == 2) {
                    expected_len = resp[1];
                    if (expected_len < 4 || expected_len > 32) len = 0; 
                }
                
                if (len > 2 && len == expected_len) {
                    if (checksum_is_valid(resp, len)) {

                        if (request_11 && expected_len >= 26) {

                            // Lock before writing to shared data (Core 0 writes)
                            portENTER_CRITICAL(&kline_mux);

                            latest_rpm = (static_cast<uint16_t>(resp[5]) << 8) | resp[6]; 
                            latest_tps = resp[8] / 0.18;            
                            latest_engine_temp = resp[10]-40;           
                            latest_map = resp[14] ;                     
                            latest_speed = resp[18];             
                            
                            uint16_t raw_tinj = (static_cast<uint16_t>(resp[19]) << 8) | resp[20]; 
                            latest_tinj = raw_tinj / 2.5; 

                            latest_ignition = (static_cast<int16_t>(resp[21]) - 128) * 5; 
                            

                            if (latest_rpm > 0) {
                                uint32_t calc_duty = ((uint32_t)latest_rpm * raw_tinj) / 300000;
                                latest_inj_duty = (calc_duty > 100) ? 100 : static_cast<uint16_t>(calc_duty);
                            } else {
                                latest_inj_duty = 0;
                            }

                            portEXIT_CRITICAL(&kline_mux);

                            last_valid_data_time = millis();
                            valid_response = true;
                            
                        } 
                        else if (!request_11 && expected_len >= 8) {
                            portENTER_CRITICAL(&kline_mux);
                            latest_afr = 211 - (resp[5] * 2); 
                            portEXIT_CRITICAL(&kline_mux);

                            last_valid_data_time = millis();
                            valid_response = true;
                        }
                    }
                    break; 
                }
            }
        } else {
            // เพิ่ม delay เป็น 2ms เพื่อให้ UART มีเวลารับข้อมูล
            vTaskDelay(2 / portTICK_PERIOD_MS);
        }
    }

    if (request_11) {
        if (valid_response) {
            failed_data_frames = 0;
        } else if (++failed_data_frames >= 10) {  // เพิ่มจาก 5 เป็น 10 ครั้ง
            reset_kline_uart();
            failed_data_frames = 0;
            request_11 = true;
            return;
        }
    }

    request_11 = !request_11;
}

KLineData KLine_GetData(void) {
    if (millis() - last_valid_data_time > KLINE_DATA_TIMEOUT_MS) {
        return {0, 0, 0, 0, 0, 0, 0, 0, 0};
    }

    // Lock before reading shared data (Core 1 reads)
    KLineData data;
    portENTER_CRITICAL(&kline_mux);
    data = {latest_rpm, latest_engine_temp, latest_map, latest_tps,
            latest_ignition, latest_afr, latest_tinj, latest_speed, latest_inj_duty};
    portEXIT_CRITICAL(&kline_mux);
    
    return data;
}

#endif // KLINE_MODE == 0