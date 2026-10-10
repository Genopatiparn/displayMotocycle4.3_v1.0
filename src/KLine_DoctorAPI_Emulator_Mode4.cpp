#include "KLine.h"
#include "hardware_config.h"
#include "driver/uart.h"
#include <string.h>

// ============================================================================
// KLINE_MODE == 4
// Doctor API / Honda K-Line ECU Emulator
//
// ใช้ ESP32 จำลอง ECU เพื่อป้อนค่า RAW 0x20 ให้ Doctor API
// โดยไม่ต้องใช้รถจริง / ไดโน
//
// รองรับ:
//   - Handshake STEP1: FE 04 72 8C
//   - Handshake STEP2: 72 05 00 F0 99
//       -> ตอบ 02 04 00 FA
//   - CMD 0x17:
//       72 07 72 17 00 0F EF
//       -> ตอบ Engine frame ค่าคงที่
//   - CMD 0x20:
//       72 07 72 20 00 02 F3
//       -> ตอบ 02 08 72 20 00 RAW 00 CHECKSUM
//   - CMD 0x11:
//       72 07 72 11 00 14 F0
//
// Serial Monitor commands:
//   r32           -> RAW = 32
//   r0            -> RAW = 0
//   r255          -> RAW = 255
//   range 0 80    -> ตั้งช่วง Auto Sweep
//   auto           -> เริ่ม Auto Sweep
//                     RAW เดิมถูกส่งซ้ำ 5 request แล้วค่อยเพิ่ม +1
//   stop           -> หยุด
//   show           -> ดูสถานะ
//
// หมายเหตุ:
//   - TX stage ใช้ BC817 แบบเดิม จึง invert TX
//   - RX ไม่ invert
// ============================================================================

#if KLINE_MODE == 4

bool kline_use_handshake = false;
bool kline_handshake_done = true;

namespace {

HardwareSerial kline_serial(2);

constexpr uint32_t KLINE_BAUD_RATE = 10400;
constexpr uint32_t REQUEST_GAP_MS = 25;

// --------------------------------------------------------------------------
// Emulator state
// --------------------------------------------------------------------------
static uint8_t test_raw = 0;

static bool auto_sweep = true;
static uint8_t auto_min = 0;
static uint8_t auto_max = 255;

// ส่ง RAW ค่าเดิมซ้ำกี่ request ก่อนขยับ +1  
static uint8_t raw_repeat_target = 5;
static uint8_t raw_repeat_count = 0;

// --------------------------------------------------------------------------
// Request RX buffer
// --------------------------------------------------------------------------
static uint8_t rx_buf[64];
static size_t rx_len = 0;
static uint32_t last_rx_byte_ms = 0;

// --------------------------------------------------------------------------
// UI data
// ใช้แสดงสถานะ emulator บนจอเดิม
//
// MAP = RAW ที่กำลังส่ง
// AFR = RAW x10 เพื่อเห็นง่าย เช่น RAW32 -> 32.0
// RPM = 1500
// TPS = 20.0%
// --------------------------------------------------------------------------
static portMUX_TYPE kline_mux = portMUX_INITIALIZER_UNLOCKED;

static KLineData latest_data = {
    1500,  // rpm
    40,    // engine_temp
    0,     // map = RAW
    200,   // tps = 20.0
    60,    // ignition = 6.0
    320,   // afr = RAW display 32.0
    20,    // tinj = 2.0
    0,     // speed
    0      // inj_duty
};

// --------------------------------------------------------------------------
// Checksum
// sum ของทุก byte รวม checksum = 0x00
// --------------------------------------------------------------------------
uint8_t make_checksum(const uint8_t* data, size_t len_without_checksum)
{
    uint8_t sum = 0;

    for (size_t i = 0; i < len_without_checksum; ++i) {
        sum = static_cast<uint8_t>(sum + data[i]);
    }

    return static_cast<uint8_t>(0x00 - sum);
}

void print_frame(const char* prefix, const uint8_t* data, size_t len)
{
    Serial.print(prefix);

}

void send_frame(uint8_t* data, size_t len)
{
    if (data == nullptr || len < 2) {
        return;
    }

    data[len - 1] =
        make_checksum(data, len - 1);

    kline_serial.write(data, len);
    kline_serial.flush();

    print_frame("[TX]", data, len);
}

// --------------------------------------------------------------------------
// Responses
// --------------------------------------------------------------------------
void send_handshake_response()
{
    uint8_t resp[4] = {
        0x02,
        0x04,
        0x00,
        0x00
    };

    send_frame(resp, sizeof(resp));
}

void send_afr_response()
{
    // ส่ง RAW ปัจจุบันตอบ request 0x20 ก่อน
    const uint8_t raw_sent = test_raw;

    uint8_t resp[8] = {
        0x02,
        0x08,
        0x72,
        0x20,
        0x00,
        raw_sent,
        0x00,
        0x00
    };

    send_frame(resp, sizeof(resp));

    portENTER_CRITICAL(&kline_mux);

    // จอแสดง RAW ที่เพิ่งส่งจริง
    latest_data.map = raw_sent;
    latest_data.afr =
        static_cast<int16_t>(raw_sent) * 10;

    portEXIT_CRITICAL(&kline_mux);

    // ส่ง RAW เดิมซ้ำตามจำนวน request ที่กำหนด
    // แล้วค่อยขยับ RAW +1
    if (auto_sweep) {
        ++raw_repeat_count;

        if (raw_repeat_count >= raw_repeat_target) {
            raw_repeat_count = 0;

            if (test_raw >= auto_max) {
                test_raw = auto_min;
            } else {
                ++test_raw;
            }
        }
    }
}

void send_engine_response()
{
    constexpr uint16_t rpm = 1500;
    constexpr uint8_t tps_raw = 4;        // TPS = raw*5 = 20.0
    constexpr uint8_t temp_raw = 80;      // current decoder -> 40C
    constexpr uint16_t inj_raw = 50;
    constexpr uint8_t ign_raw = 140;

    uint8_t resp[21] = {
        0x02,
        0x15,
        0x72,
        0x17,
        0x00,

        static_cast<uint8_t>((rpm >> 8) & 0xFF),
        static_cast<uint8_t>(rpm & 0xFF),

        0x00,
        tps_raw,
        0xFF,

        temp_raw,
        0xFF,
        0xFF,
        0x00,
        temp_raw,

        0x7D,

        static_cast<uint8_t>((inj_raw >> 8) & 0xFF),
        static_cast<uint8_t>(inj_raw & 0xFF),

        ign_raw,
        0x00,
        0x00
    };

    send_frame(resp, sizeof(resp));
}

void send_11_response()
{
    uint8_t resp[8] = {
        0x02,
        0x08,
        0x72,
        0x11,
        0x00,
        0x00,
        0x00,
        0x00
    };

    send_frame(resp, sizeof(resp));
}

// --------------------------------------------------------------------------
// Request matcher
// --------------------------------------------------------------------------
bool ends_with(const uint8_t* pattern, size_t pattern_len)
{
    if (pattern == nullptr ||
        pattern_len == 0 ||
        rx_len < pattern_len) {
        return false;
    }

    const size_t start =
        rx_len - pattern_len;

    return memcmp(
        &rx_buf[start],
        pattern,
        pattern_len
    ) == 0;
}

void handle_kline_request()
{
    static const uint8_t STEP1[] = {
        0xFE, 0x04, 0x72, 0x8C
    };

    static const uint8_t STEP2[] = {
        0x72, 0x05, 0x00, 0xF0, 0x99
    };

    static const uint8_t CMD17[] = {
        0x72, 0x07, 0x72, 0x17,
        0x00, 0x0F, 0xEF
    };

    static const uint8_t CMD20[] = {
        0x72, 0x07, 0x72, 0x20,
        0x00, 0x02, 0xF3
    };

    static const uint8_t CMD11[] = {
        0x72, 0x07, 0x72, 0x11,
        0x00, 0x14, 0xF0
    };

    if (ends_with(STEP1, sizeof(STEP1))) {

        rx_len = 0;
        return;
    }

    if (ends_with(STEP2, sizeof(STEP2))) {

        delay(2);
        send_handshake_response();

        rx_len = 0;
        return;
    }

    if (ends_with(CMD20, sizeof(CMD20))) {

        delay(2);
        send_afr_response();

        rx_len = 0;
        return;
    }

    if (ends_with(CMD17, sizeof(CMD17))) {

        delay(2);
        send_engine_response();

        rx_len = 0;
        return;
    }

    if (ends_with(CMD11, sizeof(CMD11))) {

        delay(2);
        send_11_response();

        rx_len = 0;
        return;
    }
}

// --------------------------------------------------------------------------
// USB Serial control
// --------------------------------------------------------------------------
void update_ui_raw()
{
    portENTER_CRITICAL(&kline_mux);

    latest_data.map = test_raw;
    latest_data.afr =
        static_cast<int16_t>(test_raw) * 10;

    portEXIT_CRITICAL(&kline_mux);
}

void process_serial_command(const String& command)
{
    String line = command;
    line.trim();

    if (line.length() == 0) {
        return;
    }

    if (line.startsWith("r")) {

        const int value =
            line.substring(1).toInt();

        if (value >= 0 && value <= 255) {

            test_raw =
                static_cast<uint8_t>(value);

            raw_repeat_count = 0;
            auto_sweep = false;
            update_ui_raw();

        }

        return;
    }

    if (line == "auto") {

        auto_sweep = true;
        test_raw = auto_min;
        raw_repeat_count = 0;

        update_ui_raw();

        return;
    }

    if (line == "stop") {

        auto_sweep = false;

        return;
    }

    if (line.startsWith("range ")) {

        int min_value = -1;
        int max_value = -1;

        if (sscanf(
                line.c_str(),
                "range %d %d",
                &min_value,
                &max_value
            ) == 2 &&
            min_value >= 0 &&
            max_value <= 255 &&
            min_value <= max_value) {

            auto_min =
                static_cast<uint8_t>(min_value);

            auto_max =
                static_cast<uint8_t>(max_value);

            raw_repeat_count = 0;

        }

        return;
    }

    if (line == "show") {

        return;
    }

}

void handle_usb_serial()
{
    static String line;

    while (Serial.available() > 0) {

        const char c =
            static_cast<char>(
                Serial.read()
            );

        if (c == '\r') {
            continue;
        }

        if (c != '\n') {

            line += c;

            if (line.length() > 80) {
                line = "";
            }

            continue;
        }

        process_serial_command(line);
        line = "";
    }
}


} // namespace

// ============================================================================
// KLine API
// ============================================================================
bool KLine_SendHandshake(void)
{
    // Emulator does not initiate handshake.
    // Doctor API is the master.
    return true;
}

void KLine_Init(void)
{
    kline_use_handshake = false;
    kline_handshake_done = true;

    rx_len = 0;

    pinMode(
        static_cast<uint8_t>(KLINE_RX_PIN),
        INPUT
    );

    kline_serial.end();
    kline_serial.setRxBufferSize(512);

    kline_serial.begin(
        KLINE_BAUD_RATE,
        SERIAL_8N1,
        static_cast<int>(KLINE_RX_PIN),
        static_cast<int>(KLINE_TX_PIN)
    );

    // BC817 TX stage inverts physical K-Line.
    // RX is not inverted.
    uart_set_line_inverse(
        UART_NUM_2,
        UART_SIGNAL_TXD_INV
    );

    update_ui_raw();

}

void KLine_Poll(void)
{
    kline_handshake_done = true;

    handle_usb_serial();

    while (kline_serial.available() > 0) {

        const uint8_t b =
            static_cast<uint8_t>(
                kline_serial.read()
            );

        last_rx_byte_ms = millis();

        if (rx_len < sizeof(rx_buf)) {

            rx_buf[rx_len++] = b;

        } else {

            memmove(
                rx_buf,
                rx_buf + 1,
                sizeof(rx_buf) - 1
            );

            rx_buf[
                sizeof(rx_buf) - 1
            ] = b;

            rx_len =
                sizeof(rx_buf);
        }

        handle_kline_request();
    }

    if (rx_len > 0 &&
        (millis() - last_rx_byte_ms) >
        REQUEST_GAP_MS) {

        print_frame(
            "[RX UNKNOWN]",
            rx_buf,
            rx_len
        );

        rx_len = 0;
    }
}

KLineData KLine_GetData(void)
{
    KLineData copy;

    portENTER_CRITICAL(&kline_mux);
    copy = latest_data;
    portEXIT_CRITICAL(&kline_mux);

    return copy;
}

#endif // KLINE_MODE == 4
