#include "radar_screen_handler.h"
#include "ui_radarmode/ui_Screen1.h"

extern "C" {
extern lv_obj_t *radar_ui_Screen1;
extern void radar_ui_Screen1_screen_init(void);
}

void RadarScreenHandler_Init(void)
{
    if (!radar_ui_Screen1) {
        radar_ui_Screen1_screen_init();
    }
}

lv_obj_t *RadarScreenHandler_GetScreen(void)
{
    return radar_ui_Screen1;
}
