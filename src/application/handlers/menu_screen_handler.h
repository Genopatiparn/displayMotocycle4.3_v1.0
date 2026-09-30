#ifndef MENU_SCREEN_HANDLER_H
#define MENU_SCREEN_HANDLER_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

void MenuScreenHandler_Init(void);
lv_obj_t *MenuScreenHandler_GetScreen(void);
lv_obj_t *MenuScreenHandler_GetIcon(uint8_t index);
void MenuScreenHandler_UpdateGPS(void);  

#ifdef __cplusplus
}
#endif

#endif
