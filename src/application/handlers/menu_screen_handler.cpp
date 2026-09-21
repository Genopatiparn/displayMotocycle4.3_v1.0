#include "menu_screen_handler.h"
extern "C" {
extern lv_obj_t *menu_ui_Screen1;
extern void menu_ui_Screen1_screen_init(void);
extern lv_obj_t *menu_ui_Image5;
extern lv_obj_t *ui_Image6;
extern lv_obj_t *ui_Image7;
extern lv_obj_t *ui_Image8;
extern lv_obj_t *ui_Image9;
extern lv_obj_t *ui_Image10;
extern lv_obj_t *ui_Image11;
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
        case 0: return menu_ui_Image5;
        case 1: return ui_Image6;
        case 2: return ui_Image7;
        case 3: return ui_Image8;
        case 4: return ui_Image9;
        case 5: return ui_Image11;
        case 6: return ui_Image10;
        default: return nullptr;
    }
}
