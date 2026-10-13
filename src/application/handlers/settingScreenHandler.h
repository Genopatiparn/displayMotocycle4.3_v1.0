#ifndef SETTING_SCREEN_HANDLER_H
#define SETTING_SCREEN_HANDLER_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SETTING_ITEM_COUNT 13

// Lifecycle
void SettingScreenHandler_Init(void);
void SettingScreenHandler_Update(void);
void SettingScreenHandler_Destroy(void);
lv_obj_t* SettingScreenHandler_GetScreen(void);

// Navigation
void SettingScreenHandler_NavUp(void);
void SettingScreenHandler_NavDown(void);
void SettingScreenHandler_Select(void);
void SettingScreenHandler_Exit(void);

#ifdef __cplusplus
}
#endif

#endif // SETTING_SCREEN_HANDLER_H
