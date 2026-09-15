#include "drag_screen_handler.h"

extern "C" {
extern lv_obj_t *drag_ui_dragScreen;
extern void drag_ui_dragScreen_screen_init(void);
extern void drag_ui_dragScreen_screen_destroy(void);
}

void DragScreenHandler_Init(void)
{
    if (!drag_ui_dragScreen) {
        drag_ui_dragScreen_screen_init();
    }
}

lv_obj_t *DragScreenHandler_GetScreen(void)
{
    return drag_ui_dragScreen;
}

void DragScreenHandler_Destroy(void)
{
    if (drag_ui_dragScreen) {
        drag_ui_dragScreen_screen_destroy();
    }
}
