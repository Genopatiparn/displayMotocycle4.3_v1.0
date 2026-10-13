#include "settingScreenHandler.h"

#define MENU_START_Y 60
#define MENU_ROW_H 30
#define MENU_X 30
#define MENU_Y 10
#define ARROW_X 10
#define SCREEN_W 800
#define SCREEN_H 480
#define MENU_PANE_W 400

#define IDX_KLINE_PROTOCOL 0
#define IDX_CAN_PROTOCOL   1
#define IDX_DYNO_SETTINGS  2
#define IDX_RACING_SETTINGS 3
#define IDX_DEBUG          10
#define IDX_EXIT (SETTING_ITEM_COUNT - 1)

static const char *MENU_ITEMS[SETTING_ITEM_COUNT] = {
    "K-line Protocol",
    "CAN Protocol",
    "Dyno Settings",
    "Racing Settings",
    "Shortcut Keys",
    "Main Display Info",
    "Sub Display Info",
    "Connection",
    "Screen Brightness",
    "Factory Reset",
    "Debug",
    "About",
    "Exit"
};

static lv_obj_t *ui_settingScreen = NULL;
static lv_obj_t *_lblTitle = NULL;
static lv_obj_t *_container = NULL;
static lv_obj_t *_lblArrow = NULL;
static lv_obj_t *_lblItems[SETTING_ITEM_COUNT] = {};
static int8_t s_index = 0;

void SettingScreenHandler_Init(void)
{
    if (ui_settingScreen == NULL) {
        ui_settingScreen = lv_obj_create(NULL);
        lv_obj_remove_flag(ui_settingScreen, LV_OBJ_FLAG_SCROLLABLE);
    }
    
    s_index = 0;

    // Title
    _lblTitle = lv_label_create(ui_settingScreen);
    lv_label_set_text(_lblTitle, "SETTINGS");
    lv_obj_set_width(_lblTitle, LV_SIZE_CONTENT);
    lv_obj_set_height(_lblTitle, LV_SIZE_CONTENT);
    lv_obj_set_pos(_lblTitle, SCREEN_W/2 - 60, MENU_Y);
    lv_obj_set_style_text_color(_lblTitle, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);

    // Scrollable container
    _container = lv_obj_create(ui_settingScreen);
    lv_obj_remove_style_all(_container);
    lv_obj_set_pos(_container, 0, MENU_START_Y);
    lv_obj_set_size(_container, MENU_PANE_W, SCREEN_H - MENU_START_Y);
    lv_obj_set_scrollbar_mode(_container, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_dir(_container, LV_DIR_VER);

    // Menu items
    for (int i = 0; i < SETTING_ITEM_COUNT; i++) {
        _lblItems[i] = lv_label_create(_container);
        lv_label_set_text(_lblItems[i], MENU_ITEMS[i]);
        lv_obj_set_style_text_color(_lblItems[i], lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_width(_lblItems[i], LV_SIZE_CONTENT);
        lv_obj_set_height(_lblItems[i], LV_SIZE_CONTENT);
        lv_obj_set_pos(_lblItems[i], MENU_X, i * MENU_ROW_H);
    }

    // Selection arrow
    _lblArrow = lv_label_create(_container);
    lv_label_set_text(_lblArrow, ">>");
    lv_obj_set_style_text_color(_lblArrow, lv_color_hex(0xFF4040), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_width(_lblArrow, LV_SIZE_CONTENT);
    lv_obj_set_height(_lblArrow, LV_SIZE_CONTENT);
    lv_obj_set_pos(_lblArrow, ARROW_X, 0);
}

void SettingScreenHandler_Update(void)
{
    // Update logic if needed
}

void SettingScreenHandler_NavUp(void)
{
    s_index = (s_index - 1 + SETTING_ITEM_COUNT) % SETTING_ITEM_COUNT;
    lv_obj_set_y(_lblArrow, s_index * MENU_ROW_H);
    lv_obj_scroll_to_view(_lblArrow, LV_ANIM_OFF);
}

void SettingScreenHandler_NavDown(void)
{
    s_index = (s_index + 1) % SETTING_ITEM_COUNT;
    lv_obj_set_y(_lblArrow, s_index * MENU_ROW_H);
    lv_obj_scroll_to_view(_lblArrow, LV_ANIM_OFF);
}

void SettingScreenHandler_Select(void)
{
    switch (s_index) {
        case IDX_KLINE_PROTOCOL:
            // TODO: Open K-Line settings
            break;
            
        case IDX_CAN_PROTOCOL:
            // TODO: Open CAN settings
            break;
            
        case IDX_DYNO_SETTINGS:
            // TODO: Open Dyno settings
            break;
            
        case IDX_RACING_SETTINGS:
            // TODO: Open Racing settings
            break;
            
        case IDX_DEBUG:
            // TODO: Open Debug screen
            break;
            
        case IDX_EXIT:
            SettingScreenHandler_Exit();
            break;
            
        default:
            // TODO: Implement other menu items
            break;
    }
}

void SettingScreenHandler_Exit(void)
{
    // TODO: Return to main screen
    SettingScreenHandler_Destroy();
}

void SettingScreenHandler_Destroy(void)
{
    _lblTitle = NULL;
    _container = NULL;
    _lblArrow = NULL;
    
    for (int i = 0; i < SETTING_ITEM_COUNT; i++) {
        _lblItems[i] = NULL;
    }

    if (ui_settingScreen) {
        lv_obj_del(ui_settingScreen);
    }
    ui_settingScreen = NULL;
}

lv_obj_t* SettingScreenHandler_GetScreen(void)
{
    return ui_settingScreen;
}
