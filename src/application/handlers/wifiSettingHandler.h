#ifndef WIFI_SETTING_HANDLER_H
#define WIFI_SETTING_HANDLER_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

void WifiSettingHandler_Init(void);
void WifiSettingHandler_Update(void);
void WifiSettingHandler_Exit(void);
void WifiSettingHandler_ConnectSavedNetwork(void);
uint16_t WifiSettingHandler_GetMaxRpm(void);
lv_obj_t* WifiSettingHandler_GetScreen(void);

#ifdef __cplusplus
}
#endif

#endif
