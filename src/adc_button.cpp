#include "adc_button.h"
#include "hardware_config.h"
#include <Adafruit_ADS1X15.h>
#include <Wire.h>

static Adafruit_ADS1115 ads;
static int16_t switch_raw[2] = {0, 0};
static bool adc_ready = false;
static constexpr float ADC_MV_PER_BIT = 0.1875f;

static button_status_t buttonFromVoltage(float voltage_mV,
                                         button_status_t firstButton,
                                         button_status_t secondButton,
                                         button_status_t bothButtons)
{
    // Released is near the pull-up voltage. Lower ranges are switch levels.
    if (voltage_mV < 3800.0f) {
        return firstButton;
    }
    if (voltage_mV < 4700.0f) {
        return secondButton;
    }
    return voltage_mV < 5200.0f ? BTN_NONE : bothButtons;
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
    return buttonFromVoltage(ADC_ReadVoltage_mV(ADS1115_CH_SW1),
                             BTN_SW1, BTN_SW2, BTN_SW1_SW2);
}

button_status_t ADC_ReadButton_SW34(void)
{
    return buttonFromVoltage(ADC_ReadVoltage_mV(ADS1115_CH_SW2),
                             BTN_SW3, BTN_SW4, BTN_SW3_SW4);
}

void ADC_PrintButtonStatus(void)
{
    Serial.printf("ADC: SW1=%.1fmV [%d], SW2=%.1fmV [%d]\n",
                  ADC_ReadVoltage_mV(ADS1115_CH_SW1), ADC_ReadButton_SW12(),
                  ADC_ReadVoltage_mV(ADS1115_CH_SW2), ADC_ReadButton_SW34());
}
