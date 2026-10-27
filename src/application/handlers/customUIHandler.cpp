#include "customUIHandler.h"
#include "Preferences.h"
#include "ui_home/ui.h"
#include "ui_home/ui_Screen1.h"
#include "KLine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CONTAINERS 9

// Position mapping + Container positions from ui_home/ui_Screen1.c
typedef struct {
    int16_t x;
    int16_t y;
    int16_t w;
    int16_t h;
} ContainerBounds_t;

// Container bounds from ui_home/ui_Screen1.c
static const ContainerBounds_t CONTAINER_BOUNDS[MAX_CONTAINERS] = {
    {4, 32, 100, 90},      // Container1 ( SPEED ซ้ายบน)
    {59, 215, 70, 53},     // Container2 (T.inj ซ้ายสุดล่าง)
    {0, 0, 0, 0},          // Container3 - unused
    {375, 32, 100, 90},    // Container4 (AFR ขวาบน)
    {4, 125, 100, 90},     // Container5 (TPS ซ้ายกลาง)
    {375, 125, 100, 90},   // Container6 (MAP ขวากลาง)
    {130, 215, 70, 53},    // Container7 (Ign ซ้ายล่าง)
    {275, 215, 70, 53},    // Container8 (Duty ขวาล่าง)
    {350, 215, 70, 53}     // Container9 (Temp ขวาสุดล่าง)
};

static const uint8_t CONTAINER_TO_POSITION[MAX_CONTAINERS] = {1, 3, 0, 5, 2, 6, 4, 8, 7 };

static const char* DATA_TYPE_NAMES[DATA_TYPE_COUNT] = {
    "MAP", "Battery", "RPM", "Speed", "Eng Temp",
    "TPS", "AFR", "Ignition", "Inj Duty", "T Inj",
    "Oil Temp", "STF", "LTF", "IAT"
};

#define DATA_TYPE_OPTION_COUNT 14
static const DataType_t DATA_TYPE_OPTIONS[DATA_TYPE_OPTION_COUNT] = {
    DATA_TYPE_SPEED,
    DATA_TYPE_RPM,
    DATA_TYPE_TPS,
    DATA_TYPE_TINJ,
    DATA_TYPE_IGNITION,
    DATA_TYPE_AFR,
    DATA_TYPE_OIL_TEMP,
    DATA_TYPE_BATTERY,
    DATA_TYPE_MAP,
    DATA_TYPE_ENGINE_TEMP,
    DATA_TYPE_INJ_DUTY,
    DATA_TYPE_STF,
    DATA_TYPE_LTF,
    DATA_TYPE_IAT
};

static const lv_coord_t DATA_TYPE_BUTTON_Y[DATA_TYPE_OPTION_COUNT] = {
    -109, -75, -41, -7, 27, 61, 95, -109, -75, -41, -7, 27, 61, 95
};

static const bool DATA_TYPE_BUTTON_RIGHT[DATA_TYPE_OPTION_COUNT] = {
    false, false, false, false, false, false, false, 
    true, true, true, true, true, true, true
};

static lv_obj_t *ui_customUIScreen = NULL;
static lv_obj_t *_selectionFrame = NULL;

static ContainerConfig_t s_containerConfigs[MAX_CONTAINERS];

static const uint8_t POSITION_TO_CONTAINER[8] = { 0, 4, 1, 6, 7, 8, 5, 3 };

static int8_t s_selectedPosition = 0;  // Navigate by position (0-7)
static int8_t s_selectedDataType = 0;
static int8_t s_activeWindow = -1;
static bool s_configLoaded = false;

static Preferences s_prefs;

static lv_obj_t *_typeWindows[2] = {NULL, NULL};
static lv_obj_t *_typeRows[2][DATA_TYPE_OPTION_COUNT] = {};
static lv_obj_t *s_container4Unit = NULL;
static const lv_font_t *s_unitFonts[MAX_CONTAINERS] = {};
static const lv_font_t *s_valueFonts[MAX_CONTAINERS] = {};

typedef struct {
    lv_obj_t *title;
    lv_obj_t *value;
    lv_obj_t *unit;
} ContainerLabels_t;

typedef struct {
    lv_obj_t *container;
    lv_obj_t *title;
    lv_obj_t *value;
    lv_obj_t *unit;
    lv_obj_t *bar;
} ContainerWidgets_t;

static void SetLabelTextIfChanged(lv_obj_t *label, const char *text)
{
    if (label == NULL || text == NULL) return;  // Safety check
    if (strcmp(lv_label_get_text(label), text) != 0) {
        lv_label_set_text(label, text);
    }
}

static void EnsureContainer4Unit(void)
{
    if (s_container4Unit || !ui_Container4) return;

    s_container4Unit = lv_label_create(ui_Container4);
    lv_obj_set_style_text_color(s_container4Unit, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(
        s_container4Unit,
        lv_obj_get_style_text_font(ui_Label13, LV_PART_MAIN),
        LV_PART_MAIN);
    lv_label_set_text(s_container4Unit, "");
}

static void LayoutValueAndUnit(
    uint8_t index,
    const ContainerWidgets_t &widget,
    const char *unit_text)
{
    if (!widget.container || !widget.unit) return;

    if (!s_unitFonts[index]) {
        s_unitFonts[index] = lv_obj_get_style_text_font(widget.unit, LV_PART_MAIN);
    }
    const bool uses_compact_layout = index == 1 || index == 6 || index == 8 || index == 7;
    const lv_font_t *unit_font = strcmp(unit_text, "°") == 0
        ? &lv_font_montserrat_20
        : uses_compact_layout ? &lv_font_montserrat_10 : s_unitFonts[index];
    if (lv_obj_get_style_text_font(widget.unit, LV_PART_MAIN) != unit_font) {
        lv_obj_set_style_text_font(widget.unit, unit_font, LV_PART_MAIN);
    }

    if (index == 0 || index == 3) {
        if (strcmp(unit_text, "°") == 0) {
            lv_obj_set_align(widget.unit, LV_ALIGN_BOTTOM_RIGHT);
            lv_obj_set_pos(widget.unit, 0, -71);
        } else {
            lv_obj_align(widget.unit, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
        }
    } else if (index == 4 || index == 5) {
        if (strcmp(unit_text, "°") == 0) {
            lv_obj_set_align(widget.unit, LV_ALIGN_DEFAULT);
            lv_obj_set_pos(widget.unit, 75, 16);
        } else {
            lv_obj_set_align(widget.unit, LV_ALIGN_DEFAULT);
            lv_obj_set_pos(widget.unit, 75, 36);
        }
    } else if (index == 1 || index == 6 || index == 8 || index == 7) {
        if (strcmp(unit_text, "°") == 0) {
            lv_obj_set_align(widget.unit, LV_ALIGN_RIGHT_MID);
            lv_obj_set_pos(widget.unit, -6, -6);
        } else {
            lv_obj_set_align(widget.unit, LV_ALIGN_BOTTOM_RIGHT);
            lv_obj_set_pos(widget.unit, 0, -5);
        }
    }
    
}

static void ApplyDisplayLabels(void) {
    static const char *const titles[DATA_TYPE_COUNT] = {
        "Map", "Battery", "RPM", "SPEED", "Temp", "Tps", "Afr",
        "Ign", "Duty", "T.inj", "Oil Temp", "STF", "LTF", "IAT"
    };
    static const char *const units[DATA_TYPE_COUNT] = {
        "%", "V", "rpm", "km/h", "°", "%", "",
        "°", "%", "ms", "°", "", "", "°"
    };
    const ContainerLabels_t labels_by_container[MAX_CONTAINERS] = {
        {ui_Label11, ui_Label7, ui_Label13},
        {ui_Label12, ui_Label2, ui_Label3},
        {NULL, ui_Label1, NULL},
        {ui_Label16, ui_Label15, s_container4Unit},
        {ui_Label18, ui_Label17, ui_Label14},
        {ui_Label21, ui_Label20, ui_Label19},
        {ui_Label6, ui_Label10, ui_Label23},
        {ui_Label9, ui_Label5, ui_Label8},
        {ui_Label24, ui_Label22, ui_Label25}
    };

    for (uint8_t index = 0; index < MAX_CONTAINERS; index++) {
        if (index == 2 || !s_containerConfigs[index].enabled) continue;

        const ContainerLabels_t &labels = labels_by_container[index];
        const DataType_t type = s_containerConfigs[index].data_type;
        if (labels.title) lv_label_set_text(labels.title, titles[type]);
        if (labels.unit) lv_label_set_text(labels.unit, units[type]);
    }
}

static void InitDefaultConfig(void) {
    const DataType_t default_types[MAX_CONTAINERS] = {
        DATA_TYPE_SPEED,      // Container1 (Pos 1 - ซ้ายบน)
        DATA_TYPE_TINJ,       // Container2 (Pos 3 - ซ้ายสุดล่าง)
        DATA_TYPE_RPM,        // Container3 (unused) 
        DATA_TYPE_AFR,        // Container4 (Pos 5 - ขวาบน)
        DATA_TYPE_TPS,        // Container5 (Pos 2 - ซ้ายกลาง)
        DATA_TYPE_MAP,        // Container6 (Pos 6 - ขวากลาง)
        DATA_TYPE_IGNITION,   // Container7 (Pos 4 - ซ้ายล่าง)
        DATA_TYPE_INJ_DUTY,   // Container8 (Pos 8 - ขวาล่าง)
        DATA_TYPE_ENGINE_TEMP // Container9 (Pos 7 - ขวาสุดล่าง)
    };
    
    for (int i = 0; i < MAX_CONTAINERS; i++) {
        s_containerConfigs[i].container_id = i + 1;
        s_containerConfigs[i].position = CONTAINER_TO_POSITION[i];
        s_containerConfigs[i].data_type = default_types[i];
        s_containerConfigs[i].enabled = (CONTAINER_TO_POSITION[i] > 0);
    }
}

static void UpdateSelectionFrame(void) {
    if (_selectionFrame == NULL) return;
    
    // Get container index from position
    int8_t container_idx = POSITION_TO_CONTAINER[s_selectedPosition];
    if (!s_containerConfigs[container_idx].enabled) return;
    
    const ContainerBounds_t* bounds = &CONTAINER_BOUNDS[container_idx];
    lv_obj_set_pos(_selectionFrame, bounds->x - 3, bounds->y - 3);
    lv_obj_set_size(_selectionFrame, bounds->w + 6, bounds->h + 6);
}

static void UpdateTypeWindowHighlight(void) {
    if (s_activeWindow < 0) return;

    for (int i = 0; i < DATA_TYPE_OPTION_COUNT; i++) {
        lv_obj_t *row = _typeRows[s_activeWindow][i];
        if (row == NULL) continue;

        const bool selected = i == s_selectedDataType;
        lv_obj_set_style_bg_color(row,
            lv_color_hex(selected ? 0x00C853 : 0xFF0000), LV_PART_MAIN);
        lv_obj_set_style_border_color(row,
            lv_color_hex(selected ? 0xB9F6CA : 0xCC0000), LV_PART_MAIN);
    }
}

static void HideTypeWindows(void) {
    for (int side = 0; side < 2; side++) {
        if (_typeWindows[side]) {
            lv_obj_add_flag(_typeWindows[side], LV_OBJ_FLAG_HIDDEN);
        }
    }
    s_activeWindow = -1;
}

static void CancelTypeWindow(void) {
    if (s_activeWindow < 0) return;

    const uint8_t container = POSITION_TO_CONTAINER[s_selectedPosition];
    const DataType_t current_type = s_containerConfigs[container].data_type;
    for (int i = 0; i < DATA_TYPE_OPTION_COUNT; i++) {
        if (DATA_TYPE_OPTIONS[i] == current_type) {
            s_selectedDataType = i;
            break;
        }
    }

    HideTypeWindows();
}

static void ShowTypeWindowForSelection(void) {
    const uint8_t container = POSITION_TO_CONTAINER[s_selectedPosition];
    const bool selected_on_left = CONTAINER_BOUNDS[container].x < 240;

    s_activeWindow = selected_on_left ? 1 : 0;
    const DataType_t saved_type = s_containerConfigs[container].data_type;
    s_selectedDataType = 0;
    for (int i = 0; i < DATA_TYPE_OPTION_COUNT; i++) {
        if (DATA_TYPE_OPTIONS[i] == saved_type) {
            s_selectedDataType = i;
            break;
        }
    }

    lv_obj_remove_flag(_typeWindows[s_activeWindow], LV_OBJ_FLAG_HIDDEN);
    UpdateTypeWindowHighlight();
}

void CustomUIHandler_Init(void)
{
    CustomUIHandler_LoadConfig();
    
    // Use ui_Screen1 from ui_home (already exists as home screen)
    ui_customUIScreen = ui_Screen1;
    if (ui_customUIScreen == NULL) return;
    EnsureContainer4Unit();
    ApplyDisplayLabels();
    
    // Start at position 1 (Container1)
    s_selectedPosition = 0;

    // Create selection frame (yellow border with rounded corners)
    _selectionFrame = lv_obj_create(ui_customUIScreen);
    lv_obj_remove_style_all(_selectionFrame);
    lv_obj_set_style_border_width(_selectionFrame, 3, LV_PART_MAIN);
    lv_obj_set_style_border_color(_selectionFrame, lv_color_hex(0xFFFF00), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(_selectionFrame, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_radius(_selectionFrame, 10, LV_PART_MAIN);
    lv_obj_remove_flag(_selectionFrame, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_remove_flag(_selectionFrame, LV_OBJ_FLAG_SCROLLABLE);
    UpdateSelectionFrame();
    
    for (int side = 0; side < 2; side++) {
        _typeWindows[side] = lv_obj_create(ui_customUIScreen);
        lv_obj_set_size(_typeWindows[side], 242, 265);
        lv_obj_set_align(_typeWindows[side],
            side == 0 ? LV_ALIGN_LEFT_MID : LV_ALIGN_RIGHT_MID);
        lv_obj_set_style_bg_color(_typeWindows[side], lv_color_hex(0x000000), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(_typeWindows[side], LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_border_width(_typeWindows[side], 0, LV_PART_MAIN);
        lv_obj_remove_flag(_typeWindows[side], LV_OBJ_FLAG_SCROLLABLE);

        for (int i = 0; i < DATA_TYPE_OPTION_COUNT; i++) {
            lv_obj_t *row = lv_button_create(_typeWindows[side]);
            _typeRows[side][i] = row;
            lv_obj_set_size(row, 100, 27);
            lv_obj_set_x(row, 0);
            lv_obj_set_y(row, DATA_TYPE_BUTTON_Y[i]);
            lv_obj_set_align(row,
                DATA_TYPE_BUTTON_RIGHT[i] ? LV_ALIGN_RIGHT_MID : LV_ALIGN_LEFT_MID);
            lv_obj_set_style_bg_color(row, lv_color_hex(0xFF0000), LV_PART_MAIN);
            lv_obj_set_style_bg_opa(row, LV_OPA_COVER, LV_PART_MAIN);
            lv_obj_set_style_border_width(row, 0, LV_PART_MAIN);
            lv_obj_remove_flag(row, LV_OBJ_FLAG_SCROLLABLE);

            lv_obj_t *label = lv_label_create(row);
            lv_label_set_text(label, DATA_TYPE_NAMES[DATA_TYPE_OPTIONS[i]]);
            lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
            lv_obj_set_style_text_font(label, &lv_font_montserrat_10, LV_PART_MAIN);
            lv_obj_center(label);
        }

        lv_obj_t *hint = lv_label_create(_typeWindows[side]);
        lv_label_set_text(hint, "SW1/2 Move  SW3 Confirm  SW4 Cancel");
        lv_obj_set_x(hint, side == 0 ? 00 : 00);
        lv_obj_set_y(hint, 122);
        lv_obj_set_align(hint, LV_ALIGN_CENTER);
        lv_obj_set_style_text_color(hint, lv_color_hex(0xFFFF00), LV_PART_MAIN);
        lv_obj_set_style_text_font(hint, &lv_font_montserrat_10, LV_PART_MAIN);

        lv_obj_add_flag(_typeWindows[side], LV_OBJ_FLAG_HIDDEN);
    }
}

void CustomUIHandler_Update(void)
{
    // Check if UI is initialized
    if (!ui_Container1) {
        return;  // UI not ready yet
    }
    EnsureContainer4Unit();
    
    // Use global variables updated by main loop
    extern uint16_t rpm, speed, afr, tps, map_val, inj_duty, battery;
    extern int16_t ignition, engine_temp;
    extern float stf, ltf;
    extern uint16_t tinj;
    
    const ContainerWidgets_t widgets[MAX_CONTAINERS] = {
        {ui_Container1, ui_Label11, ui_Label7, ui_Label13, NULL},
        {ui_Container2, ui_Label12, ui_Label2, ui_Label3, ui_Bar1},
        {ui_Container3, NULL, ui_Label1, NULL, NULL},
        {ui_Container4, ui_Label16, ui_Label15, s_container4Unit, NULL},
        {ui_Container5, ui_Label18, ui_Label17, ui_Label14, NULL},
        {ui_Container6, ui_Label21, ui_Label20, ui_Label19, NULL},
        {ui_Container7, ui_Label6, ui_Label10, ui_Label23, ui_Bar2},
        {ui_Container8, ui_Label9, ui_Label5, ui_Label8, ui_Bar3},
        {ui_Container9, ui_Label24, ui_Label22, ui_Label25, ui_Bar4}
    };

    for (uint8_t index = 0; index < MAX_CONTAINERS; index++) {
        if (index == 2) continue;

        const ContainerConfig_t &config = s_containerConfigs[index];
        const ContainerWidgets_t &widget = widgets[index];

        if (widget.value) {
            if (!s_valueFonts[index]) {
                s_valueFonts[index] = lv_obj_get_style_text_font(widget.value, LV_PART_MAIN);
            }
            const bool is_fuel_trim =
                config.data_type == DATA_TYPE_STF || config.data_type == DATA_TYPE_LTF;
            const lv_font_t *value_font = is_fuel_trim
                ? &ui_font_uifontAntonio24
                : s_valueFonts[index];
            if (lv_obj_get_style_text_font(widget.value, LV_PART_MAIN) != value_font) {
                lv_obj_set_style_text_font(widget.value, value_font, LV_PART_MAIN);
            }
        }

        if (widget.container) {
            const bool hidden = lv_obj_has_flag(widget.container, LV_OBJ_FLAG_HIDDEN);
            if (config.enabled && hidden) {
                lv_obj_remove_flag(widget.container, LV_OBJ_FLAG_HIDDEN);
            } else if (!config.enabled && !hidden) {
                lv_obj_add_flag(widget.container, LV_OBJ_FLAG_HIDDEN);
            }
        }
        if (!config.enabled) continue;

        const char *title = "";
        const char *unit = "";
        char value[16] = "0";
        int32_t bar_value = 0;
        bool show_bar = false;

        switch (config.data_type) {
            case DATA_TYPE_MAP:
                title = "Map";
                unit = "%";
                snprintf(value, sizeof(value), "%u", map_val);
                break;
            case DATA_TYPE_BATTERY:
                title = "Battery";
                unit = "V";
                snprintf(value, sizeof(value), "%u.%u", battery / 10, battery % 10);
                break;
            case DATA_TYPE_RPM:
                title = "RPM";
                unit = "rpm";
                snprintf(value, sizeof(value), "%u", rpm);
                break;
            case DATA_TYPE_SPEED:
                title = "SPEED";
                unit = "km/h";
                snprintf(value, sizeof(value), "%u", speed);
                break;
            case DATA_TYPE_ENGINE_TEMP:
                title = "Temp";
                unit = "°";
                snprintf(value, sizeof(value), "%d", engine_temp);
                show_bar = widget.bar != NULL;
                bar_value = (engine_temp * 100) / 120;
                break;
            case DATA_TYPE_TPS:
                title = "Tps";
                unit = "%";
                snprintf(value, sizeof(value), "%u.%u", tps / 10, tps % 10);
                break;
            case DATA_TYPE_AFR:
                title = "Afr";
                snprintf(value, sizeof(value), "%u.%u", afr / 10, afr % 10);
                break;
            case DATA_TYPE_IGNITION: {
                title = "Ign";
                unit = "°";
                const int16_t magnitude = ignition < 0 ? -ignition : ignition;
                snprintf(value, sizeof(value), "%d.%d", ignition / 10, magnitude % 10);
                show_bar = widget.bar != NULL;
                bar_value = (ignition * 100) / 600;
                break;
            }
            case DATA_TYPE_INJ_DUTY:
                title = "Duty";
                unit = "%";
                snprintf(value, sizeof(value), "%u", inj_duty);
                show_bar = widget.bar != NULL;
                bar_value = inj_duty;
                break;
            case DATA_TYPE_TINJ:
                title = "T.inj";
                unit = "ms";
                snprintf(value, sizeof(value), "%u.%u", tinj / 10, tinj % 10);
                show_bar = widget.bar != NULL;
                bar_value = (tinj * 100) / 150;
                break;
            case DATA_TYPE_OIL_TEMP:
                title = "Oil Temp";
                unit = "°";
                break;
            case DATA_TYPE_STF:
                title = "STF";
                unit = "%";
                snprintf(value, sizeof(value), "%.3f", static_cast<double>(stf));
                break;
            case DATA_TYPE_LTF:
                title = "LTF";
                unit = "%";
                snprintf(value, sizeof(value), "%.3f", static_cast<double>(ltf));
                break;
            case DATA_TYPE_IAT:
                title = "IAT";
                unit = "°";
                break;
            default:
                break;
        }

        SetLabelTextIfChanged(widget.title, title);
        SetLabelTextIfChanged(widget.value, value);
        SetLabelTextIfChanged(widget.unit, unit);
        LayoutValueAndUnit(index, widget, unit);

        if (widget.bar) {
            const bool hidden = lv_obj_has_flag(widget.bar, LV_OBJ_FLAG_HIDDEN);
            if (show_bar && hidden) {
                lv_obj_remove_flag(widget.bar, LV_OBJ_FLAG_HIDDEN);
            } else if (!show_bar && !hidden) {
                lv_obj_add_flag(widget.bar, LV_OBJ_FLAG_HIDDEN);
            }
            if (show_bar) {
                lv_bar_set_value(widget.bar, bar_value, LV_ANIM_OFF);
            }
        }
    }
}

void CustomUIHandler_NavUp(void)
{
    if (s_activeWindow >= 0) {
        s_selectedDataType = (s_selectedDataType - 1 + DATA_TYPE_OPTION_COUNT) % DATA_TYPE_OPTION_COUNT;
        UpdateTypeWindowHighlight();
        return;
    }

    // Move to previous position
    s_selectedPosition = (s_selectedPosition - 1 + 8) % 8;
    UpdateSelectionFrame();
}

void CustomUIHandler_NavDown(void)
{
    if (s_activeWindow >= 0) {
        s_selectedDataType = (s_selectedDataType + 1) % DATA_TYPE_OPTION_COUNT;
        UpdateTypeWindowHighlight();
        return;
    }

    s_selectedPosition = (s_selectedPosition + 1) % 8;
    UpdateSelectionFrame();
}

void CustomUIHandler_NavLeft(void)
{
    if (s_activeWindow >= 0) {
        s_selectedDataType = (s_selectedDataType - 1 + DATA_TYPE_OPTION_COUNT) % DATA_TYPE_OPTION_COUNT;
        UpdateTypeWindowHighlight();
        return;
    }

    s_selectedPosition = (s_selectedPosition - 1 + 8) % 8;
    UpdateSelectionFrame();
}

void CustomUIHandler_NavRight(void)
{
    if (s_activeWindow >= 0) {
        s_selectedDataType = (s_selectedDataType + 1) % DATA_TYPE_OPTION_COUNT;
        UpdateTypeWindowHighlight();
        return;
    }

    // Move to next position
    s_selectedPosition = (s_selectedPosition + 1) % 8;
    UpdateSelectionFrame();
}

void CustomUIHandler_Select(void)
{
    int8_t container_idx = POSITION_TO_CONTAINER[s_selectedPosition];

    if (s_activeWindow < 0) {
        ShowTypeWindowForSelection();
        return;
    }

    s_containerConfigs[container_idx].data_type = DATA_TYPE_OPTIONS[s_selectedDataType];
    ApplyDisplayLabels();
    CustomUIHandler_SaveConfig();
    HideTypeWindows();
}

void CustomUIHandler_Exit(void)
{
    if (s_activeWindow >= 0) {
        CancelTypeWindow();
        return;
    }

    CustomUIHandler_Destroy();
}

void CustomUIHandler_Destroy(void)
{
    if (_selectionFrame) {
        lv_obj_del(_selectionFrame);
        _selectionFrame = NULL;
    }
    
    for (int side = 0; side < 2; side++) {
        if (_typeWindows[side]) {
            lv_obj_del(_typeWindows[side]);
            _typeWindows[side] = NULL;
        }
        for (int i = 0; i < DATA_TYPE_OPTION_COUNT; i++) {
            _typeRows[side][i] = NULL;
        }
    }
    
    ui_customUIScreen = NULL;
}

lv_obj_t* CustomUIHandler_GetScreen(void)
{
    return ui_customUIScreen;
}

void CustomUIHandler_SaveConfig(void)
{
    Serial.println("[CustomUI] Saving config to NVS...");
    
    s_prefs.begin("custom_ui", false);
    
    for (int i = 0; i < MAX_CONTAINERS; i++) {
        char key[16];
        snprintf(key, sizeof(key), "c%d_type", i + 1);
        s_prefs.putUChar(key, (uint8_t)s_containerConfigs[i].data_type);
        
        snprintf(key, sizeof(key), "c%d_en", i + 1);
        s_prefs.putBool(key, s_containerConfigs[i].enabled);
        
        Serial.printf("[CustomUI] Saved Container %d: type=%d, enabled=%d\n", 
                      i+1, s_containerConfigs[i].data_type, s_containerConfigs[i].enabled);
    }
    
    s_prefs.end();
    Serial.println("[CustomUI] Config saved!");
}

void CustomUIHandler_LoadConfig(void)
{
    InitDefaultConfig();
    
    s_prefs.begin("custom_ui", true);
    
    Serial.println("[CustomUI] Loading config from NVS...");
    
    for (int i = 0; i < MAX_CONTAINERS; i++) {
        char key[16];
        snprintf(key, sizeof(key), "c%d_type", i + 1);
        uint8_t type = s_prefs.getUChar(key, (uint8_t)s_containerConfigs[i].data_type);
        if (type < DATA_TYPE_COUNT) {
            s_containerConfigs[i].data_type = (DataType_t)type;
        }
        
        snprintf(key, sizeof(key), "c%d_en", i + 1);
        bool enabled = s_prefs.getBool(key, s_containerConfigs[i].enabled);
        if (s_containerConfigs[i].position > 0) {
            s_containerConfigs[i].enabled = enabled;
        }
        
        Serial.printf("[CustomUI] Container %d: type=%d, enabled=%d\n", 
                      i+1, s_containerConfigs[i].data_type, s_containerConfigs[i].enabled);
    }
    
    s_prefs.end();
    s_configLoaded = true;
    Serial.println("[CustomUI] Config loaded!");
}

ContainerConfig_t* CustomUIHandler_GetContainerConfig(uint8_t container_id)
{
    if (container_id < 1 || container_id > MAX_CONTAINERS) return NULL;
    return &s_containerConfigs[container_id - 1];
}

void CustomUIHandler_SetContainerDataType(uint8_t container_id, DataType_t data_type)
{
    if (container_id < 1 || container_id > MAX_CONTAINERS) return;
    if (data_type >= DATA_TYPE_COUNT) return;
    
    s_containerConfigs[container_id - 1].data_type = data_type;
}

const char* CustomUIHandler_GetDataTypeName(DataType_t data_type)
{
    if (data_type >= DATA_TYPE_COUNT) return "Unknown";
    return DATA_TYPE_NAMES[data_type];
}
