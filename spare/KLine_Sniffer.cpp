#include "KLine.h"
#include "hardware_config.h"
#include "driver/uart.h"

// ============================================================================
// HONDA K-LINE PASSIVE AFR SNIFFER
//
// Doctor API remains the K-Line master.
// ESP32 is RX-only:
//   - no TX
//   - no handshake
//   - no request matching
//
// 0x17 -> engine data
// 0x20 -> O2/AFR RAW at response[5]
//
// UI:
//   MAP = RAW value
//   AFR = AFR decoded from formula
// ============================================================================

#if KLINE_MODE == 97

bool kline_use_handshake = false;
bool kline_handshake_done = false;

namespace {

HardwareSerial kline_serial(2);

constexpr uint32_t KLINE_BAUD_RATE = 10400;
constexpr uint32_t FRAME_TIMEOUT_MS = 50;

KLineData latest_data = {
    0, 0, 0, 0, 0,
    0, 0, 0, 0
};

static portMUX_TYPE kline_mux = portMUX_INITIALIZER_UNLOCKED;

// ----------------------------------------------------------------------------
// Doctor API AFR calibration - based only on currently observed points.
//
// Observed points:
//   RAW  0 -> 20.00
//   RAW 18 -> 15.00
//   RAW 19 -> 15.00
//   RAW 21 -> 14.80
//   RAW 23 -> 14.80
//   RAW 24 -> 14.80
//   RAW 31 -> 14.50
//   RAW 38 -> 13.59
//   RAW 42 -> 12.62
//   RAW 47 -> 11.20
//   RAW 54 -> 10.00
//   RAW 55+-> 10.00
//
// Values between known points are linearly interpolated.
// ----------------------------------------------------------------------------
struct DoctorAfrPoint {
    uint8_t raw;
    float afr;
};

static const DoctorAfrPoint DOCTOR_AFR_TABLE[] = {
    { 0, 20.00f},
    { 1, 18.76f},
    { 2, 17.63f},
    { 3, 16.64f},
    { 4, 16.32f},
    { 5, 16.13f},
    { 6, 15.08f},
    { 7, 15.08f},
    { 8, 15.08f},
    { 9, 15.08f},
    {10, 15.20f},
    {11, 15.20f},
    {12, 15.20f},
    {13, 15.20f},
    {14, 15.20f},
    {15, 15.20f},
    {16, 15.00f},
    {17, 15.00f},
    {18, 15.00f},
    {19, 15.00f},
    {21, 14.80f},
    {22, 14.80f},
    {23, 14.80f},
    {24, 14.80f},
    {25, 14.80f},
    {26, 14.50f},
    {27, 14.50f},
    {27, 14.50f},
    {28, 14.50f},
    {29, 14.50f},
    {30, 14.50f},
    {31, 14.50f},
    {32, 14.30f},
    {33, 14.30f},
    {34, 14.30f},
    {35, 14.18f},
    {36, 13.98f},
    {37, 13.79f},
    {38, 13.59f},
    {39, 13.40f},
    {40, 13.20f},
    {41, 13.01f},
    {42, 12.62f},
    {43, 12.42f},
    {44, 12.12f},
    {45, 11.83f},
    {46, 11.63f},
    {47, 11.20f},
    {48, 11.00f},
    {49, 10.58f},
    {50, 10.38f},
    {51, 10.19f},
    {52, 10.00f},
    {53, 10.00f},
    {54, 10.00f},
    {55, 10.00f}
};

float decodeDoctorAfr(uint8_t raw)
{
    constexpr size_t count =
        sizeof(DOCTOR_AFR_TABLE) / sizeof(DOCTOR_AFR_TABLE[0]);

    if (raw >= 55) {
        return 10.0f;
    }

    if (raw <= DOCTOR_AFR_TABLE[0].raw) {
        return DOCTOR_AFR_TABLE[0].afr;
    }

    for (size_t i = 0; i < count - 1; ++i) {
        const DoctorAfrPoint &a = DOCTOR_AFR_TABLE[i];
        const DoctorAfrPoint &b = DOCTOR_AFR_TABLE[i + 1];

        if (raw >= a.raw && raw <= b.raw) {
            if (raw == a.raw) return a.afr;
            if (raw == b.raw) return b.afr;

            const float t =
                (static_cast<float>(raw) - static_cast<float>(a.raw)) /
                (static_cast<float>(b.raw) - static_cast<float>(a.raw));

            return a.afr + t * (b.afr - a.afr);
        }
    }

    return 10.0f;
}

// ----------------------------------------------------------------------------
// Frame parser state
// ----------------------------------------------------------------------------
uint8_t frame[32];
uint8_t frame_len = 0;
uint8_t expected_len = 0;
bool capturing = false;
uint32_t frame_start_ms = 0;

void resetFrame()
{
    frame_len = 0;
    expected_len = 0;
    capturing = false;
    frame_start_ms = 0;
}

bool checksumValid(const uint8_t *data, uint8_t len)
{
    if (data == nullptr || len < 2) {
        return false;
    }

    uint8_t sum = 0;

    for (uint8_t i = 0; i < len; ++i) {
        sum = static_cast<uint8_t>(sum + data[i]);
    }

    return (sum == 0x00);
}

// ----------------------------------------------------------------------------
// 0x17 engine decoder
// ----------------------------------------------------------------------------
void decode17(const uint8_t *resp, uint8_t len)
{
    if (len != 21) {
        return;
    }

    const uint16_t rpm =
        (static_cast<uint16_t>(resp[5]) << 8) |
         resp[6];

    // TPS scale corrected to match Doctor API:
    // RAW 10 -> 50.0%
    // RAW 20 -> 100.0%
    // Equivalent to resp[8] / 0.20f
    const int16_t tps =
        static_cast<int16_t>(resp[8] * 5);

    const uint8_t raw_temp = resp[10];

    const int16_t engine_temp =
        (raw_temp == 0xFF)
        ? -41
        : static_cast<int16_t>(raw_temp) - 40;

    const uint16_t raw_tinj =
        (static_cast<uint16_t>(resp[16]) << 8) |
         resp[17];

    const int16_t tinj =
        static_cast<int16_t>(raw_tinj / 25.0f);

    const int16_t ignition =
        (static_cast<int16_t>(resp[18]) * 5) - 640;

    uint16_t inj_duty = 0;

    if (rpm > 0) {
        uint32_t duty =
            (static_cast<uint32_t>(rpm) * raw_tinj) /
            300000UL;

        if (duty > 100) {
            duty = 100;
        }

        inj_duty = static_cast<uint16_t>(duty);
    }

    portENTER_CRITICAL(&kline_mux);

    latest_data.rpm = rpm;
    latest_data.tps = tps;
    latest_data.engine_temp = engine_temp;
    latest_data.tinj = tinj;
    latest_data.ignition = ignition;
    latest_data.inj_duty = inj_duty;
    latest_data.speed = 0;

    // MAP is reserved for AFR RAW in this test.
    portEXIT_CRITICAL(&kline_mux);
}

// ----------------------------------------------------------------------------
// 0x20 AFR/O2 decoder
// Uses Doctor API measured points + interpolation.
// ----------------------------------------------------------------------------
void decode20(const uint8_t *resp, uint8_t len)
{
    if (len != 8) {
        return;
    }

    const uint8_t afr_raw = resp[5];
    const float afr = decodeDoctorAfr(afr_raw);

    const int16_t afr_x10 =
        static_cast<int16_t>(afr * 10.0f + 0.5f);

    portENTER_CRITICAL(&kline_mux);

    latest_data.map =
        static_cast<int16_t>(afr_raw);

    latest_data.afr =
        afr_x10;

    portEXIT_CRITICAL(&kline_mux);

}

void decodeFrame(const uint8_t *resp, uint8_t len)
{
    if (len < 4) {
        return;
    }

    if (resp[0] != 0x02) {
        return;
    }

    if (resp[2] != 0x72) {
        return;
    }

    if (resp[3] == 0x17) {
        decode17(resp, len);
        return;
    }

    if (resp[3] == 0x20) {
        decode20(resp, len);
        return;
    }
}

void processByte(uint8_t b)
{
    if (!capturing) {
        if (b == 0x02) {
            frame[0] = 0x02;
            frame_len = 1;
            expected_len = 0;
            capturing = true;
            frame_start_ms = millis();
        }
        return;
    }

    if (frame_len >= sizeof(frame)) {
        resetFrame();

        if (b == 0x02) {
            frame[0] = 0x02;
            frame_len = 1;
            capturing = true;
            frame_start_ms = millis();
        }

        return;
    }

    frame[frame_len++] = b;

    if (frame_len == 2) {
        expected_len = frame[1];

        if (expected_len < 4 ||
            expected_len > sizeof(frame)) {

            resetFrame();
            return;
        }
    }

    if (expected_len == 0 ||
        frame_len < expected_len) {
        return;
    }

    if (checksumValid(frame, expected_len)) {
        decodeFrame(frame, expected_len);
    } else {
        Serial.print("[BAD CHECKSUM]");

    }

    resetFrame();
}

} // namespace

// ============================================================================
// Passive mode
// ============================================================================
bool KLine_SendHandshake(void)
{
    // Return true only to prevent legacy main task retry logic.
    return true;
}

void KLine_Init(void)
{
    kline_use_handshake = false;
    kline_handshake_done = true;

    pinMode(
        static_cast<uint8_t>(KLINE_TX_PIN),
        INPUT
    );

    // Keep same RX setting that successfully received traffic.
    pinMode(
        static_cast<uint8_t>(KLINE_RX_PIN),
        INPUT_PULLUP
    );

    kline_serial.end();
    kline_serial.setRxBufferSize(512);

    kline_serial.begin(
        KLINE_BAUD_RATE,
        SERIAL_8N1,
        static_cast<int>(KLINE_RX_PIN),
        -1
    );

    // RX inversion OFF.
    uart_set_line_inverse(
        UART_NUM_2,
        0
    );

    resetFrame();

    portENTER_CRITICAL(&kline_mux);
    latest_data = {
        0, 0, 0, 0, 0,
        0, 0, 0, 0
    };
    portEXIT_CRITICAL(&kline_mux);

}

void KLine_Poll(void)
{
    kline_handshake_done = true;

    if (capturing &&
        (millis() - frame_start_ms) >
        FRAME_TIMEOUT_MS) {

        resetFrame();
    }

    while (kline_serial.available() > 0) {
        const uint8_t b =
            static_cast<uint8_t>(
                kline_serial.read()
            );

        processByte(b);
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

#endif // KLINE_MODE == 1
