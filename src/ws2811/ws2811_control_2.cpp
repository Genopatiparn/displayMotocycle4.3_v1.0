#include "ws2811_control_2.h"

WS2811Strip* strip = nullptr;

static unsigned long last_blink_time = 0;
static bool blink_state = false;
static uint8_t last_num_lit = 0;
static bool last_was_warning = false;
static uint8_t brightness = 255;

static RgbColor BrightColor(uint8_t red, uint8_t green, uint8_t blue) {
    return RgbColor(
        (static_cast<uint16_t>(red) * brightness) / 255,
        (static_cast<uint16_t>(green) * brightness) / 255,
        (static_cast<uint16_t>(blue) * brightness) / 255);
}

bool WS2811_Init(void) {
    delete strip;
    strip = new WS2811Strip(WS2811_NUM_LEDS, WS2811_PIN);

    if (strip == nullptr) {
        return false;
    }

    strip->Begin();
    strip->ClearTo(RgbColor(0, 0, 0));
    strip->Show();

    last_blink_time = millis();
    blink_state = false;
    last_num_lit = 0;
    last_was_warning = false;
    return true;
}

void WS2811_SetBrightness(uint8_t value) {
    brightness = value;
}

void WS2811_Test(void) {
    static const RgbColor rpm_colors[WS2811_NUM_LEDS] = {
        RgbColor(255, 0, 0), RgbColor(255, 127, 0),
        RgbColor(255, 255, 0), RgbColor(127, 255, 0),
        RgbColor(0, 255, 0), RgbColor(0, 255, 255),
        RgbColor(0, 127, 255), RgbColor(0, 0, 255),
        RgbColor(127, 0, 255), RgbColor(255, 0, 255)
    };

    if (strip == nullptr) {
        return;
    }

    for (uint8_t cycle = 0; cycle < 3; ++cycle) {
        for (uint8_t offset = 0; offset < WS2811_NUM_LEDS; ++offset) {
            for (uint8_t led = 0; led < WS2811_NUM_LEDS; ++led) {
                const uint8_t color_index = (led + offset) % WS2811_NUM_LEDS;
                const RgbColor color = rpm_colors[color_index];
                strip->SetPixelColor(led, BrightColor(color.R, color.G, color.B));
            }
            strip->Show();
            delay(80);
        }

        for (int8_t offset = WS2811_NUM_LEDS - 2; offset > 0; --offset) {
            for (uint8_t led = 0; led < WS2811_NUM_LEDS; ++led) {
                const uint8_t color_index = (led + offset) % WS2811_NUM_LEDS;
                const RgbColor color = rpm_colors[color_index];
                strip->SetPixelColor(led, BrightColor(color.R, color.G, color.B));
            }
            strip->Show();
            delay(80);
        }
    }

    strip->ClearTo(RgbColor(0, 0, 0));
    strip->Show();
    last_num_lit = 0;
    last_was_warning = false;
}

void WS2811_ShowRPM(uint16_t rpm, uint16_t max_rpm) {
    if (strip == nullptr || max_rpm == 0) {
        return;
    }

    constexpr uint16_t rpm_per_led = 1000;
    constexpr uint16_t warning_rpm = 11000;

    if (rpm > warning_rpm) {
        const unsigned long current_time = millis();

        if (!last_was_warning) {
            last_blink_time = current_time;
            blink_state = false;
            last_was_warning = true;
        }

        if (current_time - last_blink_time >= 50) {
            last_blink_time = current_time;
            blink_state = !blink_state;
            strip->ClearTo(blink_state ? BrightColor(255, 255, 255) : BrightColor(0, 0, 0));
            strip->Show();
        }
        return;
    }

    uint8_t num_lit = static_cast<uint8_t>(rpm / rpm_per_led);
    if (num_lit > WS2811_NUM_LEDS) {
        num_lit = WS2811_NUM_LEDS;
    }

    if (num_lit == last_num_lit && !last_was_warning) {
        return;
    }

    for (uint8_t i = 0; i < WS2811_NUM_LEDS; i++) {
        RgbColor color = BrightColor(0, 0, 0);
        if (i < num_lit) {
            if (i <= 3) {
                color = BrightColor(0, 255, 0);
            } else if (i <= 6) {
                color = BrightColor(255, 0, 0);
            } else {
                color = BrightColor(0, 0, 255);
            }
        }
        strip->SetPixelColor(i, color);
    }

    strip->Show();
    last_num_lit = num_lit;
    last_was_warning = false;
}

