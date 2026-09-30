#ifndef RADAR_SCREEN_HANDLER_H
#define RADAR_SCREEN_HANDLER_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

void RadarScreenHandler_Init(void);
lv_obj_t *RadarScreenHandler_GetScreen(void);
void RadarScreenHandler_UpdateGPS(void);

#ifdef __cplusplus
}
#endif

#endif
