#pragma once
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"
#include "../hardware_config.h"

extern esp_lcd_panel_handle_t rgb_panel_handle;
extern uint8_t LCD_Backlight;

// Panel dimensions — from hardware_config.h
#define ESP_PANEL_LCD_WIDTH             LCD_WIDTH
#define ESP_PANEL_LCD_HEIGHT            LCD_HEIGHT

// RGB 565 timing — from hardware_config.h
#define ESP_PANEL_LCD_RGB_TIMING_PCLK_HZ  LCD_PCLK_HZ
#define ESP_PANEL_LCD_RGB_TIMING_HPW       LCD_HPW
#define ESP_PANEL_LCD_RGB_TIMING_HBP       LCD_HBP
#define ESP_PANEL_LCD_RGB_TIMING_HFP       LCD_HFP
#define ESP_PANEL_LCD_RGB_TIMING_VPW       LCD_VPW
#define ESP_PANEL_LCD_RGB_TIMING_VBP       LCD_VBP
#define ESP_PANEL_LCD_RGB_TIMING_VFP       LCD_VFP
#define ESP_PANEL_LCD_RGB_PCLK_ACTIVE_NEG  LCD_PCLK_ACTIVE_NEG
#define ESP_PANEL_LCD_RGB_DATA_WIDTH       LCD_COLOR_DEPTH

void LCD_RGB_Init(void);
void LCD_RGB_Reset(void);
void LCD_RGB_SetBacklight(uint8_t Light);
void LCD_RGB_BacklightInit(void);
