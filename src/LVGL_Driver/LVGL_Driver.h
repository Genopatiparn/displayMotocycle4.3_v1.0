#pragma once

#include <lvgl.h>
#include "lv_conf.h"
#include <esp_heap_caps.h>
#include "Display_RGB.h"

#define LVGL_WIDTH          LCD_WIDTH
#define LVGL_HEIGHT         LCD_HEIGHT
#define LVGL_PARTIAL_ROWS   40
#define LVGL_BUF_LEN        (LVGL_WIDTH * LVGL_PARTIAL_ROWS * sizeof(lv_color_t))

#define EXAMPLE_LVGL_TICK_PERIOD_MS  2

void Lvgl_Set_Task_Handle(TaskHandle_t h);
void Lvgl_print(const char * buf);
void Lvgl_Display_LCD(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);
void Lvgl_Touchpad_Read(lv_indev_t *indev, lv_indev_data_t *data);
void example_increase_lvgl_tick(void *arg);

void Lvgl_Init(void);
void Lvgl_Loop(void);
