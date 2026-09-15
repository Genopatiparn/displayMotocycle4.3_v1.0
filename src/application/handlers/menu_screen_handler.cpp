#include "menu_screen_handler.h"
extern "C" {
extern lv_obj_t *menu_ui_Screen1;
extern void menu_ui_Screen1_screen_init(void);
extern lv_obj_t *menu_ui_Image2;
extern lv_obj_t *menu_ui_Image3;
extern lv_obj_t *menu_ui_Image4;
extern lv_obj_t *menu_ui_Image5;
}

void MenuScreenHandler_Init(void)
{
    if (!menu_ui_Screen1) {
        menu_ui_Screen1_screen_init();
    }
}

lv_obj_t *MenuScreenHandler_GetScreen(void)
{
    return menu_ui_Screen1;
}

lv_obj_t *MenuScreenHandler_GetIcon(uint8_t index)
{
    switch (index) {
        case 0: return menu_ui_Image2;
        case 1: return menu_ui_Image3;
        case 2: return menu_ui_Image4;
        case 3: return menu_ui_Image5;
        default: return nullptr;
    }
}
