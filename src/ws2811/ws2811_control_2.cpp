#include "ws2811_control_2.h"

WS2811Strip* strip = nullptr;

static unsigned long last_blink_time = 0;
static bool blink_state = false;
static uint8_t last_num_lit = 0;
static bool last_was_warning = false;
static uint8_t brightness = 255
;
static WS2811Style s_style = WS2811Style::RPM;

static RgbColor BrightColor(uint8_t red, uint8_t green, uint8_t blue) {
    return RgbColor(
        (static_cast<uint16_t>(red) * brightness) / 255,
        (static_cast<uint16_t>(green) * brightness) / 255,
        (static_cast<uint16_t>(blue) * brightness) / 255);
}

static RgbColor ColorWheel(uint8_t position) {
    if (position < 85) {
        return BrightColor(255 - position * 3, position * 3, 0);
    }
    if (position < 170) {
        position -= 85;
        return BrightColor(0, 255 - position * 3, position * 3);
    }
    position -= 170;
    return BrightColor(position * 3, 0, 255 - position * 3);
}

static RgbColor ScaleColor(const RgbColor &color, uint8_t level) {
    return RgbColor(
        (static_cast<uint16_t>(color.R) * level) / 255,
        (static_cast<uint16_t>(color.G) * level) / 255,
        (static_cast<uint16_t>(color.B) * level) / 255);
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

void WS2811_SetStyle(WS2811Style style) {
    s_style = style;
    last_num_lit = UINT8_MAX;
    last_was_warning = false;
    last_blink_time = millis();
    blink_state = false;
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
    if (strip == nullptr) {
        return;
    }

    const unsigned long now = millis();
    if (s_style == WS2811Style::OFF) {
        if (last_num_lit != 0 || last_was_warning) {
            strip->ClearTo(RgbColor(0, 0, 0));
            strip->Show();
            last_num_lit = 0;
            last_was_warning = false;
        }
        return;
    }

    if (s_style == WS2811Style::FLASH) {
        constexpr uint32_t color_cycle_ms = 6000;
        if (!strip->CanShow()) {
            return;
        }
        const uint8_t hue = static_cast<uint8_t>(
            ((now % color_cycle_ms) * 256UL) / color_cycle_ms);
        strip->ClearTo(ColorWheel(hue));
        strip->Show();
        return;
    }

    if (s_style == WS2811Style::STROBE) {
        constexpr uint32_t color_cycle_ms = 3000;
        if (!strip->CanShow()) {
            return;
        }
        const uint32_t phase = now % color_cycle_ms;
        const uint8_t hue = static_cast<uint8_t>(
            ((now / color_cycle_ms) * 43UL) & 0xFF);
        const uint8_t level = static_cast<uint8_t>(
            (phase * 255UL) / color_cycle_ms);
        strip->ClearTo(ScaleColor(ColorWheel(hue), level));
        strip->Show();
        return;
    }

    if (s_style == WS2811Style::FADE) {
        constexpr uint32_t color_cycle_ms = 12000;
        const uint8_t hue = static_cast<uint8_t>(
            ((now % color_cycle_ms) * 256) / color_cycle_ms);
        strip->ClearTo(ColorWheel(hue));
        strip->Show();
        return;
    }

    if (s_style == WS2811Style::SMOOTH) {
        static uint32_t lastUpdate = 0;
        static uint8_t lastHue = 255;

        constexpr uint32_t updateInterval = 100;
        constexpr uint32_t color_cycle_ms = 450;

        if (now - lastUpdate < updateInterval) {
            return;
        }
        if (!strip->CanShow()) {
            return;
        }

        const uint8_t baseHue = static_cast<uint8_t>(
            ((now % color_cycle_ms) * 256UL) / color_cycle_ms);
        if (baseHue == lastHue) {
            return;
        }

        lastUpdate = now;
        lastHue = baseHue;

        for (uint8_t i = 0; i < WS2811_NUM_LEDS; ++i) {
            const uint8_t hue = static_cast<uint8_t>(
                baseHue + (static_cast<uint16_t>(i) * 256U) / WS2811_NUM_LEDS);
            strip->SetPixelColor(i, ColorWheel(hue));
        }
        strip->Show();
        return;
    }

    if (s_style == WS2811Style::RPM && max_rpm == 0) {
        return;
    }

    const uint16_t warning_rpm = static_cast<uint16_t>((static_cast<uint32_t>(max_rpm) * 92) / 100);
    if (s_style == WS2811Style::RPM && rpm > warning_rpm) {

        if (!last_was_warning) {
            last_blink_time = now;
            blink_state = false;
            last_was_warning = true;
        }

        if (now - last_blink_time >= 50) {
            last_blink_time = now;
            blink_state = !blink_state;
            strip->ClearTo(blink_state ? BrightColor(255, 255, 255) : BrightColor(0, 0, 0));
            strip->Show();
        }
        return;
    }

    const uint32_t scaled_rpm = static_cast<uint32_t>(rpm) * WS2811_NUM_LEDS;
    uint8_t num_lit = max_rpm == 0
        ? 0
        : static_cast<uint8_t>(scaled_rpm / max_rpm);
    if (num_lit > WS2811_NUM_LEDS) {
        num_lit = WS2811_NUM_LEDS;
    }

    if (num_lit == last_num_lit && !last_was_warning && s_style == WS2811Style::RPM) {
        return;
    }

    for (uint8_t i = 0; i < WS2811_NUM_LEDS; i++) {
        RgbColor color = BrightColor(0, 0, 0);
        if (i < num_lit) {
            if (s_style == WS2811Style::CUSTOM) {
                if (i <= 3) {
                    color = BrightColor(0, 180, 255);
                } else if (i <= 6) {
                    color = BrightColor(255, 80, 0);
                } else {
                    color = BrightColor(255, 0, 40);
                }
            } else if (i <= 3) {
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
