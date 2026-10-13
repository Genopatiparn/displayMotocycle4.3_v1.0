#include "KLine.h"
#include "hardware_config.h"
#include "driver/uart.h"

//KLINE_MODE = 2
#if KLINE_MODE == 99

namespace {

HardwareSerial kline_serial(2);
constexpr uint32_t KLINE_BAUD_RATE = 10400;

// ชุดคำสั่งสำหรับถาม ECU
const uint8_t cmd_11[7] = {0x72, 0x07, 0x72, 0x11, 0x00, 0x14, 0xF0}; 
const uint8_t cmd_20[7] = {0x72, 0x07, 0x72, 0x20, 0x00, 0x02, 0xF3}; //  AFR

uint16_t latest_rpm = 0;
int16_t latest_engine_temp = -40;
int16_t latest_map = 0;
int16_t latest_tps = 0;
int16_t latest_ignition = 0;
int16_t latest_tinj = 0;
int16_t latest_speed = 0;
int16_t latest_batt_v = 0;
int16_t latest_afr = 147; 
uint16_t latest_inj_duty = 0; 

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

void KLine_Init(void) {
    pinMode(static_cast<uint8_t>(KLINE_TX_PIN), OUTPUT); 
    pinMode(static_cast<uint8_t>(KLINE_RX_PIN), INPUT);
    kline_serial.begin(KLINE_BAUD_RATE, SERIAL_8N1, static_cast<int>(KLINE_RX_PIN), static_cast<int>(KLINE_TX_PIN));
    uart_set_line_inverse(UART_NUM_2, UART_SIGNAL_TXD_INV);
}

void KLine_Poll(void) {
    static uint32_t last_request_time = 0;
    static bool request_11 = true; 

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
    uint32_t wait_start = millis();

    while (millis() - wait_start < 60) {
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
                            
                        } 
                        else if (!request_11 && expected_len >= 8) {
                            latest_afr = 211 - (resp[5] * 2); 
                        }
                    }
                    break; 
                }
            }
        }
    }

    request_11 = !request_11;
}

KLineData KLine_GetData(void) {
    return {latest_rpm, latest_engine_temp, latest_map, latest_tps,
            latest_ignition, latest_afr, latest_tinj, latest_speed, latest_inj_duty};
}

#endif // KLINE_MODE == 0