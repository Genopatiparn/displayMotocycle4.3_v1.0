#include "KLine.h"
#include "hardware_config.h"

// KLINE_MODE = 1 เท่านั้น
#if KLINE_MODE == 1

namespace {

HardwareSerial kline_serial(2);
constexpr uint32_t KLINE_BAUD_RATE = 10400;
constexpr uint32_t RESPONSE_TIMEOUT_MS = 120;

const uint8_t cmd_11[7] = {0x72, 0x07, 0x72, 0x11, 0x00, 0x14, 0xF0}; 
const uint8_t cmd_20[7] = {0x72, 0x07, 0x72, 0x20, 0x00, 0x02, 0xF3}; 

KLineData latest_data = {0, -40, 0, 0, 0, 147, 0, 0, 0, 0};

uint8_t match_len_11 = 0;
uint8_t match_len_20 = 0;
uint8_t response[32] = {};
uint8_t response_length = 0;
bool capturing_response = false;
uint32_t response_started_at = 0;
uint8_t current_command = 0;
uint8_t expected_length = 0;

void reset_response_capture(void) {
    response_length = 0;
    capturing_response = false;
    current_command = 0;
    expected_length = 0;
}

bool checksum_is_valid(const uint8_t* data, size_t length) {
    uint8_t sum = 0;
    for (size_t i = 0; i < length; ++i) {
        sum = static_cast<uint8_t>(sum + data[i]);
    }
    return sum == 0;
}

void process_bus_byte(uint8_t value);

void capture_response_byte(uint8_t value) {
    if (response_length == 0 && value != 0x02) {
        reset_response_capture();
        process_bus_byte(value);
        return;
    }

    response[response_length++] = value;

    if (response_length == 2) {
        if (response[1] >= 4 && response[1] <= 32) expected_length = response[1];
        else {
            reset_response_capture();
            return;
        }
    }

    if (response_length < expected_length) return;

    if (checksum_is_valid(response, expected_length)) {
        
        // ==========================================
        // ถอดรหัส: อิงตามโครงสร้าง ECU=SHOP Simulator (26 Bytes)
        // ==========================================
        if (current_command == 0x11 && expected_length >= 26) {
            
            latest_data.rpm = (static_cast<uint16_t>(response[5]) << 8) | response[6]; // ไบต์ 5, 6
            latest_data.tps = response[8] / 0.18;             // ไบต์ 8
            latest_data.engine_temp = response[10];           // ไบต์ 10
            latest_data.map = response[14] / 0.103;           // ไบต์ 14
            latest_data.batt_v = response[17] * 10;           // ไบต์ 17
            latest_data.speed = response[18];                 // ไบต์ 18
            
            // เวลาฉีด (T.inj): ไบต์ 19, 20
            uint16_t raw_tinj = (static_cast<uint16_t>(response[19]) << 8) | response[20];
            latest_data.tinj = raw_tinj / 2.5; 
            
            // องศาไฟ (Ignition): ไบต์ 21
            latest_data.ignition = (static_cast<int16_t>(response[21]) - 128) * 5; 
            
            // คำนวณ Duty
            if (latest_data.rpm > 0) {
                uint32_t calc_duty = ((uint32_t)latest_data.rpm * raw_tinj) / 300000;
                latest_data.inj_duty = (calc_duty > 100) ? 100 : static_cast<uint16_t>(calc_duty);
            } else {
                latest_data.inj_duty = 0;
            }
        }
        else if (current_command == 0x20 && expected_length >= 8) {
            latest_data.afr = 211 - (response[5] * 2); // ไบต์ 5 ถอดรหัส AFR
        }
    }
    reset_response_capture();
}

void process_bus_byte(uint8_t value) {
    if (capturing_response) {
        capture_response_byte(value);
        return;
    }

    if (value == cmd_11[match_len_11]) {
        if (++match_len_11 == 7) {
            current_command = 0x11;
            expected_length = 26; 
            capturing_response = true;
            response_started_at = millis();
            match_len_11 = 0;
            match_len_20 = 0;
            return;
        }
    } else match_len_11 = (value == cmd_11[0]) ? 1 : 0;

    if (value == cmd_20[match_len_20]) {
        if (++match_len_20 == 7) {
            current_command = 0x20;
            expected_length = 8; 
            capturing_response = true;
            response_started_at = millis();
            match_len_11 = 0;
            match_len_20 = 0;
            return;
        }
    } else match_len_20 = (value == cmd_20[0]) ? 1 : 0;
}

} // namespace

void KLine_Init(void) {
    pinMode(static_cast<uint8_t>(KLINE_TX_PIN), INPUT); // เซ็ต TX เป็น INPUT เพื่อปิดการส่งข้อมูล
    pinMode(static_cast<uint8_t>(KLINE_RX_PIN), INPUT);
    kline_serial.begin(KLINE_BAUD_RATE, SERIAL_8N1, static_cast<int>(KLINE_RX_PIN), -1); // ค่า -1 คือการตัดฮาร์ดแวร์ TX ทิ้ง
    reset_response_capture();
    Serial.println("K-Line Initialized in SNIFFER MODE: [SIMULATOR DECODING REVERSED - 26 BYTES]");
}

void KLine_Poll(void) {
    if (capturing_response && millis() - response_started_at > RESPONSE_TIMEOUT_MS) {
        reset_response_capture();
    }
    while (kline_serial.available() > 0) {
        process_bus_byte(static_cast<uint8_t>(kline_serial.read()));
    }
}

KLineData KLine_GetData(void) {
    return latest_data;
}

#endif // KLINE_MODE == 1