#include "menu_screen_handler.h"
#include "../../gps_sr1723.h"
#include <Arduino.h>

extern "C" {
extern lv_obj_t *menu_ui_Screen1;
extern void menu_ui_Screen1_screen_init(void);
extern lv_obj_t *menu_ui_Image5;
extern lv_obj_t *menu_ui_Image3;
extern lv_obj_t *ui_Image6;
extern lv_obj_t *ui_Image7;
extern lv_obj_t *ui_Image8;
extern lv_obj_t *ui_Image9;
extern lv_obj_t *ui_Image10;
extern lv_obj_t *ui_Image11;
extern lv_obj_t *ui_Image4;  // Background image

extern lv_obj_t *ui_Label4;  
extern lv_obj_t *ui_Screen1;

// GPS status images - shared with Radar screen
extern const lv_image_dsc_t ui_img_gps_status_png;    // No signal (Bad)
extern const lv_image_dsc_t ui_img_gps_status_1_png;  // Moderate/Fair
extern const lv_image_dsc_t ui_img_gps_status_2_png;  // Good
extern const lv_image_dsc_t ui_img_gps_status_3_png;  // Excellent
}

static lv_obj_t *menu_gps_icon = nullptr;
static bool menu_gps_icon_created = false;

void MenuScreenHandler_Init(void)
{
    if (!menu_ui_Screen1) {
        menu_ui_Screen1_screen_init();
    }
    
    // Hide the menu's original GPS icon; do not hide the Home RPM needle.
    if (menu_ui_Image3) {
        lv_obj_add_flag(menu_ui_Image3, LV_OBJ_FLAG_HIDDEN);
    }

    if (!menu_gps_icon_created && menu_ui_Screen1) {
        menu_gps_icon = lv_image_create(menu_ui_Screen1);
        lv_image_set_src(menu_gps_icon, &ui_img_gps_status_png);

        // Position at top-right (near date/time)
        lv_obj_set_width(menu_gps_icon, LV_SIZE_CONTENT);
        lv_obj_set_height(menu_gps_icon, LV_SIZE_CONTENT);
        lv_obj_set_x(menu_gps_icon, 154);    // Right side
        lv_obj_set_y(menu_gps_icon, -123);   // Top side
        lv_obj_set_align(menu_gps_icon, LV_ALIGN_CENTER);
        
        lv_obj_remove_flag(menu_gps_icon, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(menu_gps_icon, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_remove_flag(menu_gps_icon, LV_OBJ_FLAG_SCROLLABLE);
        
        // DON'T move to foreground! Keep it at its natural position
        // lv_obj_move_foreground(menu_gps_icon);  // REMOVED
        
        menu_gps_icon_created = true;
        
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

void MenuScreenHandler_UpdateGPS(void)
{
    if (!menu_ui_Screen1) {
        return;
    }
    
    if (!menu_gps_icon) {
        return;
    }
    
    const char *quality = GPS_GetQualityStatus();
    bool has_fix = GPS_HasFix();
    
    // Update GPS time (ui_Label4)
    if (ui_Label4) {
        gps_debug_info_t gps_info;
        GPS_GetDebugInfo(&gps_info);
        
        if (gps_info.utc_time[0] != '\0') {
            int hour = 0, minute = 0, second = 0;
            if (sscanf(gps_info.utc_time, "%d:%d:%d", &hour, &minute, &second) == 3) {
                char time_str[8];
                snprintf(time_str, sizeof(time_str), "%02d:%02d", hour, minute);
                lv_label_set_text(ui_Label4, time_str);
            } else {
                lv_label_set_text(ui_Label4, "--:--");
            }
        } else {
            lv_label_set_text(ui_Label4, "--:--");
        }
    }
    
    // Update GPS icon based on signal quality (same logic as Radar screen)
    static const char *last_quality = nullptr;
    
    // Only update icon if quality changed
    if (last_quality != quality) {
        if (!has_fix || strcmp(quality, "Bad") == 0) {
            lv_image_set_src(menu_gps_icon, &ui_img_gps_status_png);  // No bars
        } else if (strcmp(quality, "Fair") == 0 || strcmp(quality, "Moderate") == 0) {
            lv_image_set_src(menu_gps_icon, &ui_img_gps_status_1_png);  // 1 bar
        } else if (strcmp(quality, "Good") == 0) {
            lv_image_set_src(menu_gps_icon, &ui_img_gps_status_2_png);  // 2 bars
        } else if (strcmp(quality, "Excellent") == 0) {
            lv_image_set_src(menu_gps_icon, &ui_img_gps_status_3_png);  // 3 bars
        }
        
        // Force refresh
        lv_obj_invalidate(menu_gps_icon);
        
        last_quality = quality;
    }
}
