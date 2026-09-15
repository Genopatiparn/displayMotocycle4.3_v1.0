#ifndef HOME_SCREEN_HANDLER_H
#define HOME_SCREEN_HANDLER_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

void HomeScreenHandler_Init(void);
lv_obj_t *HomeScreenHandler_GetScreen(void);

#ifdef __cplusplus
}
#endif

#endif
