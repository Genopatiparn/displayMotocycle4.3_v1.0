#include "home_screen_handler.h"
#include "ui_home/ui.h"

void HomeScreenHandler_Init(void)
{
    if (!ui_Screen1) {
        ui_init();
    }
}

lv_obj_t *HomeScreenHandler_GetScreen(void)
{
    return ui_Screen1;
}
