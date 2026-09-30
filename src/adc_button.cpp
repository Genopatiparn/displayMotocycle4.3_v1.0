#include "adc_button.h"
#include "hardware_config.h"
#include <Adafruit_ADS1X15.h>
#include <Wire.h>

static Adafruit_ADS1115 ads;
static int16_t switch_raw[2] = {0, 0};
static bool adc_ready = false;
static constexpr float ADC_MV_PER_BIT = 0.1875f;

// Button debounce
static button_status_t last_sw12 = BTN_NONE;
static button_status_t last_sw34 = BTN_NONE;
static uint32_t last_sw12_change = 0;
static uint32_t last_sw34_change = 0;
static constexpr uint32_t DEBOUNCE_MS = 50;  // 50ms debounce

static button_status_t buttonFromVoltage(float voltage_mV,
                                         button_status_t firstButton,
                                         button_status_t secondButton,
                                         button_status_t bothButtons)
{
    // Voltage thresholds based on field measurements:
    // SW3+SW4 (both): ~3588mV (lowest)
    // SW3 alone:      ~3739mV 
    // SW4 alone:      ~4406mV
    // NONE (idle):    ~4945mV (VCC/pull-up)
    
    if (voltage_mV < 3650.0f) {
        return bothButtons;  // SW3+SW4 (~3588mV)
    }
    if (voltage_mV < 4050.0f) {
        return firstButton;  // SW3 alone (~3739mV)
    }
    if (voltage_mV < 4700.0f) {
        return secondButton;  // SW4 alone (~4406mV)
    }
    return BTN_NONE;  // >= 4700mV (~4945mV)
}

bool ADC_Button_Init(void)
{
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN, I2C_FREQ_HZ);

    if (!ads.begin(ADS1115_I2C_ADDR, &Wire)) {
        Serial.println("ADS1115 not found");
        return false;
    }

    ads.setGain(GAIN_TWOTHIRDS);
    ads.setDataRate(RATE_ADS1115_860SPS);
    adc_ready = true;
    ADC_Update();
    return true;
}

void ADC_Update(void)
{
    if (!adc_ready) {
        return;
    }

    switch_raw[0] = ads.readADC_SingleEnded(ADS1115_CH_SW1);
    switch_raw[1] = ads.readADC_SingleEnded(ADS1115_CH_SW2);
}

void ADC_UpdateAuxiliary(void)
{
}

int16_t ADC_ReadRaw(uint8_t channel)
{
    if (channel > ADS1115_CH_SW2) {
        return 0;
    }

    return switch_raw[channel];
}

float ADC_ReadVoltage_mV(uint8_t channel)
{
    return ADC_ReadRaw(channel) * ADC_MV_PER_BIT;
}

button_status_t ADC_ReadButton_SW12(void)
{
    button_status_t current = buttonFromVoltage(ADC_ReadVoltage_mV(ADS1115_CH_SW1),
                                                BTN_SW1, BTN_SW2, BTN_SW1_SW2);
    
    // Debounce: only update if stable for DEBOUNCE_MS
    uint32_t now = millis();
    if (current != last_sw12) {
        if (now - last_sw12_change > DEBOUNCE_MS) {
            last_sw12 = current;
            last_sw12_change = now;
        }
    } else {
        last_sw12_change = now;
    }
    
    return last_sw12;
}

button_status_t ADC_ReadButton_SW34(void)
{
    float voltage = ADC_ReadVoltage_mV(ADS1115_CH_SW2);
    button_status_t current = buttonFromVoltage(voltage,
                                                BTN_SW3, BTN_SW4, BTN_SW3_SW4);
    
    // Debounce: only update if stable for DEBOUNCE_MS
    uint32_t now = millis();
    if (current != last_sw34) {
        if (now - last_sw34_change > DEBOUNCE_MS) {
            last_sw34 = current;
            last_sw34_change = now;
        }
    } else {
        last_sw34_change = now;
    }
    
    return last_sw34;
}

void ADC_PrintButtonStatus(void)
{
    Serial.printf("ADC: SW1=%.1fmV [%d], SW2=%.1fmV [%d]\n",
                  ADC_ReadVoltage_mV(ADS1115_CH_SW1), ADC_ReadButton_SW12(),
                  ADC_ReadVoltage_mV(ADS1115_CH_SW2), ADC_ReadButton_SW34());
}
