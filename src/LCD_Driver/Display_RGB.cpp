#include <Arduino.h>
#include "esp_idf_version.h"
#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"
#include "Display_RGB.h"

static const char *TAG = "LCD_RGB";
esp_lcd_panel_handle_t rgb_panel_handle = NULL;
uint8_t LCD_Backlight = 100;

void LCD_RGB_Reset()
{
    gpio_set_level(LCD_RESET_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(LCD_RESET_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(120));
}

void LCD_RGB_Init(void)
{
    printf("[LCD] init start\n");

    gpio_config_t io_conf = {};
    io_conf.pin_bit_mask =  (1ULL << LCD_RESET_PIN);
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&io_conf);
    gpio_set_level(LCD_RESET_PIN, 1);
 
    printf("[LCD] hardware reset\n");
    LCD_RGB_Reset();

    esp_lcd_rgb_panel_config_t rgb_config = {
        .clk_src = LCD_CLK_SRC_PLL240M,
        .timings = {
            .pclk_hz = 10 * 1000 * 1000,
            .h_res = LCD_WIDTH,
            .v_res = LCD_HEIGHT,
            .hsync_pulse_width = 5,
            .hsync_back_porch = 45,
            .hsync_front_porch = 9,
            .vsync_pulse_width = 4,
            .vsync_back_porch = 8,
            .vsync_front_porch = 12,
            .flags = {.pclk_active_neg = 0},
        },
        .data_width = ESP_PANEL_LCD_RGB_DATA_WIDTH,
#if ESP_IDF_VERSION_MAJOR >= 5
        .bounce_buffer_size_px = ESP_PANEL_LCD_WIDTH * 34,
#else
        .sram_trans_align  = 4,
        .psram_trans_align = 64,
#endif
        .hsync_gpio_num = LCD_PIN_HSYNC,
        .vsync_gpio_num = LCD_PIN_VSYNC,
        .de_gpio_num = LCD_PIN_DE,
        .pclk_gpio_num = LCD_PIN_PCLK,
        .disp_gpio_num = -1,
        .data_gpio_nums = {
            LCD_PIN_B0, LCD_PIN_B1, LCD_PIN_B2, LCD_PIN_B3, LCD_PIN_B4,
            LCD_PIN_G0, LCD_PIN_G1, LCD_PIN_G2, LCD_PIN_G3, LCD_PIN_G4, LCD_PIN_G5,
            LCD_PIN_R0, LCD_PIN_R1, LCD_PIN_R2, LCD_PIN_R3, LCD_PIN_R4,
        },
        .flags = {
            .fb_in_psram = 1,
        },
    };

    // ESP-IDF 4.4.x triggers TG1WDT (interrupt watchdog) during PSRAM DMA
    // setup inside esp_lcd_new_rgb_panel(). Disable it for the duration.
    // TIMERG1.wdtwprotect.val = TIMG_WDT_WKEY_VALUE;
    // TIMERG1.wdtconfig0.val  = 0;
    // TIMERG1.wdtwprotect.val = 0;

    printf("[LCD] esp_lcd_new_rgb_panel\n");
    esp_err_t ret = esp_lcd_new_rgb_panel(&rgb_config, &rgb_panel_handle);
    if (ret != ESP_OK) {
        printf("[LCD] esp_lcd_new_rgb_panel FAILED: %s\n", esp_err_to_name(ret));
        return;
    }

    printf("[LCD] esp_lcd_panel_reset\n");
    esp_lcd_panel_reset(rgb_panel_handle);

    printf("[LCD] esp_lcd_panel_init\n");
    esp_lcd_panel_init(rgb_panel_handle);

    // Re-apply max drive strength AFTER panel init — the driver reconfigures GPIOs
    // during init and may reset drive capability to default (20 mA).
  

    printf("[LCD] init done\n");
}

void LCD_RGB_BacklightInit(void)
{
    ledcAttach(LCD_BACKLIGHT_PIN, LCD_BL_PWM_FREQ, LCD_BL_PWM_RESOLUTION);
    LCD_RGB_SetBacklight(LCD_Backlight);
}

void LCD_RGB_SetBacklight(uint8_t Light)
{
    if (Light > LCD_BL_MAX_DUTY) Light = LCD_BL_MAX_DUTY;
    uint32_t duty = (Light == LCD_BL_MAX_DUTY)
                        ? (1 << LCD_BL_PWM_RESOLUTION) - 1
                        : Light * ((1 << LCD_BL_PWM_RESOLUTION) / LCD_BL_MAX_DUTY);
    ledcWrite(LCD_BACKLIGHT_PIN, duty);
}
