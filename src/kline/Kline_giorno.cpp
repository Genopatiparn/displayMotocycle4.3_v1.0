#include "KLine.h"
#include "hardware_config.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "soc/gpio_sig_map.h"
#include "esp_rom_gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#if KLINE_MODE == 1

bool kline_use_handshake = true;
bool kline_handshake_done = false;

namespace {

HardwareSerial kline_serial(2);

constexpr uint32_t KLINE_BAUD_RATE = 10400;
constexpr uint32_t KLINE_DATA_TIMEOUT_MS = 2000;

// Polling 
constexpr uint32_t RX_TIMEOUT_MS = 30; 
constexpr uint32_t RX_SILENCE_MS = 4;
constexpr uint32_t ENGINE_REQUEST_INTERVAL_MS = 30;

const uint8_t cmd_17[7] = { 0x72, 0x07, 0x72, 0x17, 0x00, 0x0F, 0xEF };
const uint8_t cmd_20[7] = { 0x72, 0x07, 0x72, 0x20, 0x00, 0x02, 0xF3 };

static portMUX_TYPE kline_mux = portMUX_INITIALIZER_UNLOCKED;
static uint16_t latest_rpm = 0;
static int16_t  latest_engine_temp = 0; 
static int16_t  latest_map = 0;
static int16_t  latest_tps = 0;         
static int16_t  latest_ignition = 0;
static int16_t  latest_afr = 0;
static bool afr_nonzero_sample_seen = false;
static int16_t  latest_tinj = 0;
static int16_t  latest_speed = 0;
static uint16_t latest_inj_duty = 0;
static uint16_t latest_battery_voltage_x10 = 0;
static uint8_t latest_stf_raw = 0;
static uint8_t latest_ltf_raw = 0;
static uint32_t last_valid_data_time = 0;

constexpr bool AFR_USE_SIGNED_RAW = true; 

struct DoctorAfrPoint {
    uint8_t raw;
    float afr;
};

static const DoctorAfrPoint DOCTOR_AFR_TABLE[] = {
    { 0, 20.00f}, { 1, 18.76f}, { 2, 17.63f}, { 3, 16.64f}, { 4, 16.32f}, { 5, 16.13f}, 
    { 6, 15.08f}, { 7, 15.08f}, { 8, 15.08f}, { 9, 15.08f}, {10, 15.20f}, {11, 15.20f}, 
    {12, 15.20f}, {13, 15.20f}, {14, 15.20f}, {15, 15.20f}, {16, 15.00f}, {17, 15.00f},
    {18, 15.00f}, {19, 15.00f}, {21, 14.80f}, {22, 14.80f}, {23, 14.80f}, {24, 14.80f},
    {25, 14.80f}, {26, 14.50f}, {27, 14.50f}, {28, 14.50f}, {29, 14.50f}, {30, 14.50f},
    {31, 14.50f}, {32, 14.30f}, {33, 14.30f}, {34, 14.30f}, {35, 14.18f}, {36, 13.98f},
    {37, 13.79f}, {38, 13.59f}, {39, 13.40f}, {40, 13.20f}, {41, 13.01f}, {42, 12.62f}, 
    {43, 12.42f}, {44, 12.12f}, {45, 11.83f}, {46, 11.63f}, {47, 11.20f}, {48, 11.00f},
    {49, 10.58f}, {50, 10.38f}, {51, 10.19f}, {52, 10.00f}, {53, 10.00f}, {54, 10.00f},
    {55, 10.00f},
};

float calculate_afr_from_table(uint8_t raw) {
    const size_t count = sizeof(DOCTOR_AFR_TABLE) / sizeof(DOCTOR_AFR_TABLE[0]);

    if (raw <= DOCTOR_AFR_TABLE[0].raw)
        return DOCTOR_AFR_TABLE[0].afr;
    if (raw >= DOCTOR_AFR_TABLE[count - 1].raw)
        return DOCTOR_AFR_TABLE[count - 1].afr;

    for (size_t i = 0; i + 1 < count; ++i) {
        const DoctorAfrPoint &a = DOCTOR_AFR_TABLE[i];
        const DoctorAfrPoint &b = DOCTOR_AFR_TABLE[i + 1];

        if (raw >= a.raw && raw <= b.raw) {
            if (raw == a.raw) return a.afr;
            if (raw == b.raw) return b.afr;

            const float ratio =
                static_cast<float>(raw - a.raw) / static_cast<float>(b.raw - a.raw);
            return a.afr + ratio * (b.afr - a.afr);
        }
    }

    return 10.00f;
}

int16_t interpret_afr_raw(uint8_t raw_byte) {
    if (AFR_USE_SIGNED_RAW && raw_byte >= 0x80) {
        return static_cast<int16_t>(raw_byte) - 256;
    }
    return static_cast<int16_t>(raw_byte);
}

bool checksum_is_valid(const uint8_t* data, size_t length) {
    if (data == nullptr || length < 2) return false;

    uint8_t sum = 0;
    for (size_t i = 0; i < length; ++i) {
        sum = static_cast<uint8_t>(sum + data[i]);
    }
    return sum == 0;
}

void clear_uart_rx() {
    while (kline_serial.available()) {
        kline_serial.read();
    }
}

void send_kline_command(const uint8_t* command, size_t length) {
    kline_serial.write(command, length);
    kline_serial.flush();
}

void KLine_BeginUART(void) {
    kline_serial.setRxBufferSize(512);

    kline_serial.begin(
        KLINE_BAUD_RATE,
        SERIAL_8N1,
        static_cast<int>(KLINE_RX_PIN),
        -1
    );

    uart_set_line_inverse(UART_NUM_2, UART_SIGNAL_TXD_INV);

    esp_rom_gpio_connect_out_signal(
        static_cast<uint32_t>(KLINE_TX_PIN),
        U2TXD_OUT_IDX,
        false,
        false
    );
}

void KLine_SendWakeupPulse() {
    kline_serial.end();

    pinMode(static_cast<uint8_t>(KLINE_TX_PIN), OUTPUT);

    digitalWrite(static_cast<uint8_t>(KLINE_TX_PIN), HIGH);
    delay(70);

    digitalWrite(static_cast<uint8_t>(KLINE_TX_PIN), LOW);
    delay(120);

    KLine_BeginUART();
    delay(1);
}

size_t receive_window(
    uint8_t* rx_buf,
    size_t capacity,
    uint32_t timeout_ms
) {
    size_t rx_len = 0;
    const uint32_t start = millis();

    bool got_any = false;
    bool saw_response_start = false;
    uint32_t last_byte_time = start;

    while ((millis() - start) < timeout_ms) {
        bool got_byte_this_pass = false;

        while (kline_serial.available() && rx_len < capacity) {
            const uint8_t byte = static_cast<uint8_t>(kline_serial.read());
            rx_buf[rx_len++] = byte;

            got_any = true;
            if (byte == 0x02 || byte == 0x0E) {
                saw_response_start = true;
            }
            got_byte_this_pass = true;
            last_byte_time = millis();
        }

        if (got_any && saw_response_start &&
            !got_byte_this_pass &&
            (millis() - last_byte_time) >= RX_SILENCE_MS) {
            break;
        }

        vTaskDelay(1);
    }

    return rx_len;
}

const uint8_t* find_valid_response(
    const uint8_t* rx,
    size_t rx_len,
    uint8_t expected_len,
    uint8_t expected_cmd
) {
    if (rx == nullptr || rx_len < expected_len) return nullptr;

    for (size_t i = 0;
         i + expected_len <= rx_len;
         ++i) {

        if (rx[i] != 0x02) continue;
        if (rx[i + 1] != expected_len) continue;
        if (rx[i + 2] != 0x72) continue;
        if (rx[i + 3] != expected_cmd) continue;

        if (!checksum_is_valid(&rx[i], expected_len)) {
            continue;
        }

        return &rx[i];
    }

    return nullptr;
}

} 

bool KLine_SendHandshake() {
    const uint8_t cmd_step1[4] = { 0xFE, 0x04, 0x72, 0x8C };
    const uint8_t cmd_step2[5] = { 0x72, 0x05, 0x00, 0xF0, 0x99 };
    const uint8_t expected_step2[4] = { 0x02, 0x04, 0x00, 0xFA };

    constexpr uint8_t TOTAL_ROUNDS = 10;
    constexpr uint32_t HANDSHAKE_RX_MS = 40;

    for (uint8_t round = 0; round < TOTAL_ROUNDS; ++round) {
        const uint8_t fe_count = ((round % 2) == 0) ? 5U : 1U;
        KLine_SendWakeupPulse();

        // STEP 1
        for (uint8_t attempt = 0; attempt < fe_count; ++attempt) {
            send_kline_command( cmd_step1, sizeof(cmd_step1) );
            uint8_t rx_buf[128] = {};
            const size_t rx_len = receive_window( rx_buf, sizeof(rx_buf), HANDSHAKE_RX_MS );
        }
        delay(1);

        // STEP 2
        send_kline_command( cmd_step2, sizeof(cmd_step2) );
        uint8_t rx_buf[128] = {};
        const size_t rx_len = receive_window( rx_buf, sizeof(rx_buf), HANDSHAKE_RX_MS );
        bool step2_ok = false;

        for (size_t i = 0; i + sizeof(expected_step2) <= rx_len;++i) {
            if (memcmp( &rx_buf[i], expected_step2, sizeof(expected_step2) ) == 0) {
                step2_ok = true;
                break;
            }
        }

        if (step2_ok) {
            return true;
        }

        delay(1000);
    }
    return false;
}

void KLine_Init(void) {
    pinMode(
        static_cast<uint8_t>(KLINE_RX_PIN),
        INPUT
    );

    if (kline_use_handshake &&
        !kline_handshake_done) {

        if (KLine_SendHandshake()) {
            kline_handshake_done = true;
            clear_uart_rx();
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
    clear_uart_rx();
}

void KLine_Poll(void) {
    if (kline_use_handshake &&
        !kline_handshake_done) {
        return;
    }

    static uint32_t last_request_ms = 0;
    static bool request_afr_next = true; // สลับอ่าน Engine 1 ครั้ง : AFR 1 ครั้ง (1:1)
    static uint8_t failed_engine_frames = 0;
    const uint32_t now = millis();

    if ((now - last_request_ms) < ENGINE_REQUEST_INTERVAL_MS) {
        return;
    }

    last_request_ms = now;

    // AFR 0x20
    if (request_afr_next) {
        request_afr_next = false;
        clear_uart_rx();
        send_kline_command( cmd_20, sizeof(cmd_20) );

        uint8_t rx_buf[128] = {};
        const size_t rx_len = receive_window( rx_buf, sizeof(rx_buf), RX_TIMEOUT_MS );
        const uint8_t* resp = find_valid_response( rx_buf, rx_len, 8, 0x20 );

        if (resp != nullptr) {
            const uint8_t stf_raw = resp[6];
            portENTER_CRITICAL(&kline_mux);
            latest_stf_raw = stf_raw;
            portEXIT_CRITICAL(&kline_mux);

            const uint8_t afr_raw_byte = resp[5];
            const int16_t afr_raw = interpret_afr_raw(afr_raw_byte);

            if (afr_raw == 0 && !afr_nonzero_sample_seen) {
                return;
            }

            const uint8_t table_raw = static_cast<uint8_t>(
                afr_raw < 0 ? 0 : afr_raw > 55 ? 55 : afr_raw);
            const float afr_value = calculate_afr_from_table(table_raw);
            
            if (afr_value > 0.0f && afr_value <= 99.9f) {
                const int16_t afr_new = static_cast<int16_t>(afr_value * 10.0f);

                portENTER_CRITICAL(&kline_mux);
                latest_afr = afr_new;
                last_valid_data_time = millis();
                portEXIT_CRITICAL(&kline_mux);

                if (afr_raw != 0) {
                    afr_nonzero_sample_seen = true;
                }
            }
        }
        return;
    }

    // ENGINE 0x17
    request_afr_next = true;
    clear_uart_rx();
    send_kline_command( cmd_17, sizeof(cmd_17) );

    uint8_t rx_buf[128] = {};
    const size_t rx_len = receive_window( rx_buf, sizeof(rx_buf), RX_TIMEOUT_MS );
    const uint8_t* resp = find_valid_response( rx_buf, rx_len, 21, 0x17 );

    if (resp != nullptr) {
        const uint16_t rpm = (static_cast<uint16_t>(resp[5]) << 8) | resp[6];
        const int16_t tps = static_cast<int16_t>( resp[8] / 0.20f );
        const uint8_t raw_temp = resp[14];
        const int16_t engine_temp = (raw_temp == 0xFF) ? -41 : static_cast<int16_t>(raw_temp) - 40;
        const int16_t map_val = 0;
        const int16_t speed_val = 0;
        const uint16_t battery_voltage_x10 = resp[15];
        const uint16_t raw_tinj = (static_cast<uint16_t>(resp[16]) << 8) | resp[17];
        const uint8_t ltf_raw = resp[19];
        const int16_t tinj = static_cast<int16_t>( raw_tinj / 25.0f );
        const int16_t ignition = (static_cast<int16_t>(resp[18]) * 5) - 640;
        uint16_t inj_duty = 0;

        if (rpm > 0) {
            uint32_t calc_duty = (static_cast<uint32_t>(rpm) * raw_tinj) / 300000.0f;
            inj_duty = (calc_duty > 100) ? 100 : static_cast<uint16_t>(calc_duty);
        }

        const uint32_t stamp = millis();

        portENTER_CRITICAL(&kline_mux);
        latest_rpm = rpm;
        latest_engine_temp = engine_temp;
        latest_map = map_val;
        latest_tps = tps;
        latest_ignition = ignition;
        latest_tinj = tinj;
        latest_speed = speed_val;
        latest_inj_duty = inj_duty;
        latest_battery_voltage_x10 = battery_voltage_x10;
        latest_ltf_raw = ltf_raw;
        last_valid_data_time = stamp;
        portEXIT_CRITICAL(&kline_mux);

        failed_engine_frames = 0;

    } else {
        ++failed_engine_frames;

        if (failed_engine_frames >= 20) {
            failed_engine_frames = 0;
            reset_kline_uart();
            return;
        }
    }
}

KLineData KLine_GetData(void) {
    uint16_t rpm;
    int16_t engine_temp;
    int16_t map_val;
    int16_t tps;
    int16_t ignition;
    int16_t afr;
    int16_t tinj;
    int16_t speed;
    uint16_t inj_duty;
    uint16_t battery_voltage_x10;
    uint8_t stf_raw;
    uint8_t ltf_raw;

    portENTER_CRITICAL(&kline_mux);

    rpm = latest_rpm;
    engine_temp = latest_engine_temp;
    map_val = latest_map;
    tps = latest_tps;
    ignition = latest_ignition;
    afr = latest_afr;
    tinj = latest_tinj;
    speed = latest_speed;
    inj_duty = latest_inj_duty;
    battery_voltage_x10 = latest_battery_voltage_x10;
    stf_raw = latest_stf_raw;
    ltf_raw = latest_ltf_raw;

    portEXIT_CRITICAL(&kline_mux);

    return {
        rpm, engine_temp, map_val, tps, ignition, afr, tinj, speed, inj_duty,
        battery_voltage_x10, stf_raw, ltf_raw };
}

#endif // KLINE_MODE == 1