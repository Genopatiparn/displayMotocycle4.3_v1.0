#include "screen_manager.h"
#include "application/handlers/home_screen_handler.h"
#include "application/handlers/menu_screen_handler.h"
#include "application/handlers/drag_screen_handler.h"
#include "application/handlers/radar_screen_handler.h"
#include "application/handlers/settingScreenHandler.h"
#include "application/handlers/customUIHandler.h"
#include "application/handlers/wifiSettingHandler.h"
#include <Arduino.h>

static screen_type_t current_screen = SCREEN_HOME;
static menu_item_t selected_menu_item = MENU_ITEM_DISPLAY;
static button_status_t last_btn12_state = BTN_NONE;
static button_status_t last_btn34_state = BTN_NONE;
static button_status_t stable_btn12_state = BTN_NONE;
static button_status_t stable_btn34_state = BTN_NONE;
static unsigned long last_btn12_time = 0;
static unsigned long last_btn34_time = 0;
static uint8_t btn12_stable_count = 0;
static uint8_t btn34_stable_count = 0;
static const uint16_t BUTTON_DEBOUNCE_MS = 100;
static const uint8_t BUTTON_STABLE_COUNT = 1;
static lv_obj_t *menu_icons[MENU_ITEM_MAX];

void ScreenManager_Init(void)
{
    HomeScreenHandler_Init();
    MenuScreenHandler_Init();
    RadarScreenHandler_Init();
    DragScreenHandler_Init();
    SettingScreenHandler_Init();

    // Load custom UI config for home screen display
    CustomUIHandler_LoadConfig();

    for (uint8_t index = 0; index < MENU_ITEM_MAX; index++) {
        menu_icons[index] = MenuScreenHandler_GetIcon(index);
    }
    current_screen = SCREEN_HOME;
    lv_screen_load(HomeScreenHandler_GetScreen());
}

void ScreenManager_SwitchTo(screen_type_t screen)
{
    if (screen >= SCREEN_MAX) return;
    current_screen = screen;

    switch (screen) {
        case SCREEN_HOME:
            lv_screen_load(HomeScreenHandler_GetScreen());
            break;
        case SCREEN_MENU:
            lv_screen_load(MenuScreenHandler_GetScreen());
            selected_menu_item = MENU_ITEM_DISPLAY;
            ScreenManager_MenuHighlight(selected_menu_item);
            break;
        case SCREEN_DRAG:
            lv_screen_load(DragScreenHandler_GetScreen());
            break;
        case SCREEN_RADAR:
            lv_screen_load(RadarScreenHandler_GetScreen());
            break;
        case SCREEN_SETTING:
            lv_screen_load(SettingScreenHandler_GetScreen());
            break;
        case SCREEN_CUSTOM_UI:
            CustomUIHandler_Init();
            if (CustomUIHandler_GetScreen() == NULL) {
                Serial.println("[ScreenManager] Custom UI initialization failed");
                current_screen = SCREEN_SETTING;
                lv_screen_load(SettingScreenHandler_GetScreen());
                break;
            }
            lv_screen_load(CustomUIHandler_GetScreen());
            break;
        case SCREEN_WIFI_SETUP:
            WifiSettingHandler_Init();
            if (WifiSettingHandler_GetScreen() == NULL) {
                Serial.println("[ScreenManager] Wi-Fi setup initialization failed");
                current_screen = SCREEN_SETTING;
                lv_screen_load(SettingScreenHandler_GetScreen());
                break;
            }
            lv_screen_load(WifiSettingHandler_GetScreen());
            break;
        default:
            break;
    }
}

screen_type_t ScreenManager_GetCurrentScreen(void)
{
    return current_screen;
}

void ScreenManager_MenuHighlight(menu_item_t item)
{
    if (item >= MENU_ITEM_MAX) return;
    
    // Use the new MenuScreenHandler_HighlightIcon function
    MenuScreenHandler_HighlightIcon(item);
    
    selected_menu_item = item;
}

menu_item_t ScreenManager_GetSelectedMenuItem(void)
{
    return selected_menu_item;
}

void ScreenManager_HandleButton(button_status_t btn12, button_status_t btn34)
{
    unsigned long now = millis();
    if (btn12 == last_btn12_state) btn12_stable_count++;
    else { btn12_stable_count = 0; last_btn12_state = btn12; }
    if (btn34 == last_btn34_state) btn34_stable_count++;
    else { btn34_stable_count = 0; last_btn34_state = btn34; }

    button_status_t btn12_edge = BTN_NONE;
    if (btn12_stable_count >= BUTTON_STABLE_COUNT && btn12 != stable_btn12_state &&
        now - last_btn12_time >= BUTTON_DEBOUNCE_MS) {
        if (btn12 != BTN_NONE) btn12_edge = btn12;
        stable_btn12_state = btn12;
        last_btn12_time = now;
    }
    button_status_t btn34_edge = BTN_NONE;
    if (btn34_stable_count >= BUTTON_STABLE_COUNT && btn34 != stable_btn34_state &&
        now - last_btn34_time >= BUTTON_DEBOUNCE_MS) {
        if (btn34 != BTN_NONE) btn34_edge = btn34;
        stable_btn34_state = btn34;
        last_btn34_time = now;
    }

    if (current_screen == SCREEN_HOME) {
        if (btn34_edge == BTN_SW3) ScreenManager_SwitchTo(SCREEN_MENU);
    } else if (current_screen == SCREEN_MENU) {
        if (btn12_edge == BTN_SW1) {
             selected_menu_item = (menu_item_t)((selected_menu_item + 1) % MENU_ITEM_MAX);
            ScreenManager_MenuHighlight(selected_menu_item);

        } else if (btn12_edge == BTN_SW2) {
            selected_menu_item = selected_menu_item == 0 ? (menu_item_t)(MENU_ITEM_MAX - 1) : (menu_item_t)(selected_menu_item - 1);
            ScreenManager_MenuHighlight(selected_menu_item);
        }
        if (btn34_edge == BTN_SW3) {
            if (selected_menu_item == MENU_ITEM_RADAR) ScreenManager_SwitchTo(SCREEN_RADAR);
            else if (selected_menu_item == MENU_ITEM_DISPLAY) ScreenManager_SwitchTo(SCREEN_HOME);
            else if (selected_menu_item == MENU_ITEM_DRAG) ScreenManager_SwitchTo(SCREEN_DRAG);
            else if (selected_menu_item == MENU_ITEM_SETTING) ScreenManager_SwitchTo(SCREEN_SETTING);
        } else if (btn34_edge == BTN_SW4) {
            ScreenManager_SwitchTo(SCREEN_HOME);
        }
    } else if ((current_screen == SCREEN_DRAG || current_screen == SCREEN_RADAR || current_screen == SCREEN_SETTING) && btn34_edge == BTN_SW4) {
        ScreenManager_SwitchTo(SCREEN_HOME);
    } else if (current_screen == SCREEN_SETTING) {
        if (btn12_edge == BTN_SW2) {
            SettingScreenHandler_NavDown();
        } else if (btn12_edge == BTN_SW1) {
            SettingScreenHandler_NavUp();
        }

        if (btn34_edge == BTN_SW3) {
            SettingScreenHandler_Select();
        }
    } else if (current_screen == SCREEN_CUSTOM_UI) {
        if (btn12_edge == BTN_SW2) {
            CustomUIHandler_NavDown();
        } else if (btn12_edge == BTN_SW1) {
            CustomUIHandler_NavUp();
        } else if (btn34_edge == BTN_SW3) {
            CustomUIHandler_Select();
        } else if (btn34_edge == BTN_SW4) {
            CustomUIHandler_Exit();
            if (CustomUIHandler_GetScreen() == NULL) {
                ScreenManager_SwitchTo(SCREEN_SETTING);
            }
        }
    } else if (current_screen == SCREEN_WIFI_SETUP) {
        if (btn34_edge == BTN_SW4) {
            WifiSettingHandler_Exit();
        }
    } else if (current_screen == SCREEN_DRAG) {
        // Drag Screen controls
        if (btn34_edge == BTN_SW3) {
            // SW3: Start/Reset timer
            DragScreenHandler_Start();
        } else if (btn12_edge == BTN_SW1) {
            // SW1: Reset timer
            DragScreenHandler_Reset();
        }
    }
}
