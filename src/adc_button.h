/**
 * @file adc_button.h
 * @brief ADS1115 ADC driver for button input detection
 * 
 * Hardware: 4 buttons connected via voltage divider to ADS1115
 * - SW1, SW2 on AN0 (ADC_SW1)
 * - SW3, SW4 on AN1 (ADC_SW2)
 */

#ifndef ADC_BUTTON_H
#define ADC_BUTTON_H

#include <Arduino.h>

#ifdef __cplusplus
extern "C" {
#endif

// Button status enumeration
typedef enum {
    BTN_NONE = 0,      // No button pressed
    BTN_SW1,           // SW1 pressed
    BTN_SW2,           // SW2 pressed
    BTN_SW1_SW2,       // SW1 + SW2 pressed
    BTN_SW3,           // SW3 pressed
    BTN_SW4,           // SW4 pressed
    BTN_SW3_SW4        // SW3 + SW4 pressed
} button_status_t;

/**
 * @brief Initialize ADS1115 ADC and I2C
 * @return true if successful, false otherwise
 */
bool ADC_Button_Init(void);

/**
 * @brief Refresh cached ADS1115 readings for all channels
 */
void ADC_Update(void);

/**
 * @brief Refresh the slower auxiliary ADC channels
 */
void ADC_UpdateAuxiliary(void);

/**
 * @brief Read button status from SW1/SW2 (AN0)
 * @return Button status
 */
button_status_t ADC_ReadButton_SW12(void);

/**
 * @brief Read button status from SW3/SW4 (AN1)
 * @return Button status
 */
button_status_t ADC_ReadButton_SW34(void);

/**
 * @brief Read raw ADC value from channel
 * @param channel ADS1115 channel (0-3)
 * @return ADC value (0-65535 for 16-bit)
 */
int16_t ADC_ReadRaw(uint8_t channel);

/**
 * @brief Read voltage from channel in millivolts
 * @param channel ADS1115 channel (0-3)
 * @return Voltage in mV
 */
float ADC_ReadVoltage_mV(uint8_t channel);

/**
 * @brief Print button status for debugging
 */
void ADC_PrintButtonStatus(void);

#ifdef __cplusplus
}
#endif

#endif // ADC_BUTTON_H
