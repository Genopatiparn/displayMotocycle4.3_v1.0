#ifndef WS2811_CONTROL_2_H
#define WS2811_CONTROL_2_H

#include <Arduino.h>
#include <NeoPixelBus.h>

#define WS2811_PIN      2
#define WS2811_NUM_LEDS 10

using WS2811Strip = NeoPixelBus<NeoRgbFeature, NeoEsp32Rmt0800KbpsMethod>;

enum class WS2811Style : uint8_t {
    FLASH,
    STROBE,
    FADE,
    SMOOTH,
    CUSTOM,
    RPM,
    OFF
};

extern WS2811Strip* strip;

bool WS2811_Init(void);
void WS2811_SetBrightness(uint8_t brightness);
void WS2811_SetStyle(WS2811Style style);
void WS2811_Test(void);
void WS2811_ShowRPM(uint16_t rpm, uint16_t max_rpm);

#endif