#ifndef APPLICATION_DRAG_SCREEN_HANDLER_H
#define APPLICATION_DRAG_SCREEN_HANDLER_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

void DragScreenHandler_Init(void);
void DragScreenHandler_Update(void);
void DragScreenHandler_RefreshTimes(void);  // Refresh recorded times on screen
void DragScreenHandler_Start(void);
void DragScreenHandler_Reset(void);
lv_obj_t *DragScreenHandler_GetScreen(void);
void DragScreenHandler_Destroy(void);

#ifdef __cplusplus
}
#endif

#endif
