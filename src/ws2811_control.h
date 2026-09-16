/**
 * @file ws2811_control.h
 * @brief WS2811 addressable LED control driver
 * 
 * Controls cascaded WS2811 RGB LEDs via single data line (GPIO2)
 */

#ifndef WS2811_CONTROL_H
#define WS2811_CONTROL_H

#include <Arduino.h>

#ifdef __cplusplus
extern "C" {
#endif

// WS2811 Configuration
#define WS2811_PIN      2       // GPIO2 - Data pin for WS2811
#define WS2811_NUM_LEDS 10      // Number of LEDs in cascade chain

/**
 * @brief Initialize WS2811 LED control
 * @return true if successful
 */
bool WS2811_Init(void);

/**
 * @brief Show RPM on LEDs as bar graph with shift light
 * @param rpm Current RPM value
 * @param max_rpm Maximum RPM value (shift light at 95%)
 */
void WS2811_ShowRPM(uint16_t rpm, uint16_t max_rpm);

/**
 * @brief Run the WS2811 LED test sequence
 */
void WS2811_Test(void);

#ifdef __cplusplus
}
#endif

#endif // WS2811_CONTROL_H
