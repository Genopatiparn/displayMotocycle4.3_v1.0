#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H

#include "lvgl.h"
#include "adc_button.h"

#ifdef __cplusplus

// Screen types
typedef enum {
    SCREEN_HOME = 0,
    SCREEN_MENU,
    SCREEN_DRAG,
    SCREEN_RADAR,
    SCREEN_MAX
} screen_type_t;

// Menu items
typedef enum {
    MENU_ITEM_DISPLAY = 0,
    MENU_ITEM_ARTBOARD_4,
    MENU_ITEM_DRAG,
    MENU_ITEM_ARTBOARD_3,
    MENU_ITEM_ARTBOARD_5,
    MENU_ITEM_SETTING,
    MENU_ITEM_RADAR,
    MENU_ITEM_MAX
} menu_item_t;

// Functions
void ScreenManager_Init(void);
void ScreenManager_SwitchTo(screen_type_t screen);
screen_type_t ScreenManager_GetCurrentScreen(void);
void ScreenManager_HandleButton(button_status_t btn12, button_status_t btn34);
void ScreenManager_MenuHighlight(menu_item_t item);
menu_item_t ScreenManager_GetSelectedMenuItem(void);

#endif // __cplusplus

#endif // SCREEN_MANAGER_H
