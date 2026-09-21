/**
 * @file ws2811_control.cpp
 * @brief WS2811 addressable LED control for RPM display
 */

#include "ws2811_control.h"
#include <Adafruit_NeoPixel.h>

static Adafruit_NeoPixel* strip = nullptr;
static unsigned long last_blink_time = 0;
static bool blink_state = false;
static uint8_t last_num_lit = 0;
static bool last_was_warning = false;
static uint8_t brightness = 2;

bool WS2811_Init(void) {
    if (strip != nullptr) {
        delete strip;
    }

    strip = new Adafruit_NeoPixel(WS2811_NUM_LEDS, WS2811_PIN, NEO_RGB + NEO_KHZ400);
    
    if (strip == nullptr) {
        Serial.println("Failed to create WS2811 strip object");
        return false;
    }

    strip->begin();
    #include "driver/gpio.h"
    gpio_set_drive_capability((gpio_num_t)WS2811_PIN, GPIO_DRIVE_CAP_3);
    strip->setBrightness(brightness);
    strip->show(); // Initialize all pixels to 'off'

    last_blink_time = millis();
    blink_state = false;
    last_num_lit = 0;
    last_was_warning = false;

    Serial.printf("WS2811 initialized: %d LEDs on GPIO%d\n", WS2811_NUM_LEDS, WS2811_PIN);
    return true;
}

void WS2811_SetBrightness(uint8_t value) {
    brightness = value;
    if (strip != nullptr) {
        strip->setBrightness(brightness);
    }
}

void WS2811_ShowRPM(uint16_t rpm, uint16_t max_rpm) {
    if (strip == nullptr || max_rpm == 0) {
        return;
    }

    constexpr uint16_t first_led_rpm = 2;
    constexpr uint16_t last_led_rpm = 10000;
    constexpr uint16_t warning_rpm = 11000;

    if (rpm > warning_rpm) {
        unsigned long current_time = millis();

        if (!last_was_warning) {
            blink_state = false;
            last_blink_time = current_time;
            last_was_warning = true;
        }

        if (current_time - last_blink_time >= 50) {
            last_blink_time = current_time;
            blink_state = !blink_state;

            for (uint8_t i = 0; i < WS2811_NUM_LEDS; i++) {
                strip->setPixelColor(
                    i, blink_state ? strip->Color(255, 255, 255) : strip->Color(0, 0, 0));
            }
            strip->show();
        }
        return;
    }

    uint8_t num_lit = 0;
    if (rpm >= first_led_rpm) {
        for (uint8_t i = 0; i < WS2811_NUM_LEDS; i++) {
            uint16_t led_rpm = first_led_rpm +
                ((last_led_rpm - first_led_rpm) * i) / (WS2811_NUM_LEDS - 1);
            if (rpm >= led_rpm) {
                num_lit = i + 1;
            }
        }
    }

    if (num_lit == last_num_lit && !last_was_warning) {
        return;
    }

    const uint8_t led_colors[WS2811_NUM_LEDS][3] = {
    //  [ R,   G,   B]
        {0, 255, 0},
        {0, 255, 0},
        {0, 255, 0},
        {0, 255, 0},
        {255, 0, 0},
        {255, 0, 0},
        {255, 0, 0},
        {0, 0, 255},
        {0, 0, 255},
        {0, 0, 255}
    };

    for (uint8_t i = 0; i < WS2811_NUM_LEDS; i++) {
        if (i < num_lit) {
            strip->setPixelColor(i, strip->Color(
                led_colors[i][0], led_colors[i][1], led_colors[i][2]));
        } else {
            strip->setPixelColor(i, strip->Color(0, 0, 0));
        }
    }
    delay(1);
    strip->show();
    last_num_lit = num_lit;
    last_was_warning = false;
}

void WS2811_Test(void) {
    const uint8_t spectrum[WS2811_NUM_LEDS][3] = {
     //  [ R, G, B]
        {255, 0, 0},
        {255, 127, 0},
        {255, 255, 0},
        {127, 255, 0},
        {0, 255, 0},
        {0, 255, 255},
        {0, 127, 255},
        {0, 0, 255},
        {127, 0, 255},
        {255, 0, 255}
    };

    for (uint8_t cycle = 0; cycle < 3; cycle++) {
        for (int8_t offset = 0; offset < WS2811_NUM_LEDS; offset++) {
            for (uint8_t led = 0; led < WS2811_NUM_LEDS; led++) {
                uint8_t color_index = (led + offset) % WS2811_NUM_LEDS;
                strip->setPixelColor(led, strip->Color(
                    spectrum[color_index][0],
                    spectrum[color_index][1],
                    spectrum[color_index][2]));
            }
            strip->show();
            delay(80);
        }

        for (int8_t offset = WS2811_NUM_LEDS - 2; offset > 0; offset--) {
            for (uint8_t led = 0; led < WS2811_NUM_LEDS; led++) {
                uint8_t color_index = (led + offset) % WS2811_NUM_LEDS;
                strip->setPixelColor(led, strip->Color(
                    spectrum[color_index][0],
                    spectrum[color_index][1],
                    spectrum[color_index][2]));
            }
            strip->show();
            delay(80);
        }
    }

    for (uint8_t i = 0; i < WS2811_NUM_LEDS; i++) {
        strip->setPixelColor(i, strip->Color(0, 0, 0));
    }
    strip->show();
}
