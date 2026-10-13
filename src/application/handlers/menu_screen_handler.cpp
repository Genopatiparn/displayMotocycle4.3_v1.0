#include "menu_screen_handler.h"
#include "gps_sr1723.h"
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

// Red icon versions (for highlighting)
extern lv_obj_t *ui_Display;    // Display red
extern lv_obj_t *ui_Circuit;    // Circuit red
extern lv_obj_t *ui_Drag;       // Drag red
extern lv_obj_t *ui_Gps_red;    // GPS red

extern lv_obj_t *ui_Label4;  
extern lv_obj_t *ui_Screen1;

// GPS status images - shared with Radar screen
extern const lv_image_dsc_t ui_img_gps_status_png;    // No signal (Bad)
extern const lv_image_dsc_t ui_img_gps_status_1_png;  // Moderate/Fair
extern const lv_image_dsc_t ui_img_gps_status_2_png;  // Good
extern const lv_image_dsc_t ui_img_gps_status_3_png;  // Excellent

// Menu icons - Grey (default)
extern const lv_image_dsc_t ui_img_display_png;
extern const lv_image_dsc_t ui_img_artboard_4_png;
extern const lv_image_dsc_t ui_img_drag_png;
extern const lv_image_dsc_t ui_img_artboard_3_png;
extern const lv_image_dsc_t ui_img_artboard_5_png;
extern const lv_image_dsc_t ui_img_setting_png;
extern const lv_image_dsc_t ui_img_gps_png;           // ✅ เพิ่ม GPS grey
extern const lv_image_dsc_t ui_img_circuit_mode_png;

// Menu icons - Red (selected)
extern const lv_image_dsc_t ui_img_display_red_png;
// Note: Only display has red version, others will use tint
}

static lv_obj_t *menu_gps_icon = nullptr;
static bool menu_gps_icon_created = false;

// Store grey icon images for restoration
static const lv_image_dsc_t* grey_icons[7] = {
    &ui_img_display_png,       // index 0: Display
    &ui_img_artboard_4_png,    // index 1: Artboard 4
    &ui_img_drag_png,          // index 2: Drag
    &ui_img_artboard_3_png,    // index 3: Artboard 3
    &ui_img_artboard_5_png,    // index 4: Circuit (grey = artboard_5)
    &ui_img_setting_png,       // index 5: Setting
    &ui_img_gps_png            // index 6: GPS/Radar
};

// Store red icon images (only display has red version)
static const lv_image_dsc_t* red_icons[7] = {
    &ui_img_display_red_png,  // index 0 has red version
    nullptr,  // index 1: use tint
    nullptr,  // index 2: Drag has ui_Drag red version (handled separately)
    nullptr,  // index 3: use tint
    nullptr,  // index 4: Circuit - use tint (no red image)
    nullptr,  // index 5: Setting - use tint
    nullptr   // index 6: GPS has ui_Gps_red (handled separately)
};

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
        case 0: return menu_ui_Image5;  // Display
        case 1: return ui_Image6;        // Artboard 4
        case 2: return ui_Image7;        // Drag
        case 3: return ui_Image8;        // Artboard 3
        case 4: return ui_Image9;        // Artboard 5
        case 5: return ui_Image11;       // Setting
        case 6: return ui_Image10;       // GPS (Radar)
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

void MenuScreenHandler_HighlightIcon(uint8_t index)
{
    // First, hide all red icons and show all grey icons
    if (ui_Display) lv_obj_add_flag(ui_Display, LV_OBJ_FLAG_HIDDEN);
    if (ui_Circuit) lv_obj_add_flag(ui_Circuit, LV_OBJ_FLAG_HIDDEN);  // Circuit แดง
    if (ui_Drag) lv_obj_add_flag(ui_Drag, LV_OBJ_FLAG_HIDDEN);
    if (ui_Gps_red) lv_obj_add_flag(ui_Gps_red, LV_OBJ_FLAG_HIDDEN);
    
    // Reset all grey icons to dim opacity
    for (uint8_t i = 0; i < 7; i++) {
        lv_obj_t *icon = MenuScreenHandler_GetIcon(i);
        if (icon && grey_icons[i]) {
            lv_image_set_src(icon, grey_icons[i]);
            // Remove red tint and set to dim opacity
            lv_obj_set_style_image_recolor_opa(icon, LV_OPA_TRANSP, 0);
            lv_obj_set_style_opa(icon, LV_OPA_50, 0);  // Dim unselected icons
            lv_obj_remove_flag(icon, LV_OBJ_FLAG_HIDDEN);  // Show grey icons
        }
    }
    
    // Highlight selected icon
    if (index < 7) {
        lv_obj_t *grey_icon = MenuScreenHandler_GetIcon(index);
        
        if (grey_icon) {
            // Set grey icon to full brightness
            lv_obj_set_style_opa(grey_icon, LV_OPA_COVER, 0);
            
            // Check if there's a dedicated red icon
            lv_obj_t *red_icon = nullptr;
            switch (index) {
                case 0: red_icon = ui_Display; break;   // Display has red version
                case 2: red_icon = ui_Drag; break;      // Drag has red version
                case 4: red_icon = ui_Circuit; break;   // Circuit has red version (circuit_mode_png)
                case 6: red_icon = ui_Gps_red; break;   // GPS has red version
                default: break;  // Others use tint
            }
            
            if (red_icon) {
                // Hide grey, show red icon
                lv_obj_add_flag(grey_icon, LV_OBJ_FLAG_HIDDEN);
                lv_obj_remove_flag(red_icon, LV_OBJ_FLAG_HIDDEN);
            } else {
                // Use red tint for icons without red version
                lv_obj_set_style_image_recolor_opa(grey_icon, LV_OPA_COVER, 0);
                lv_obj_set_style_image_recolor(grey_icon, lv_color_hex(0xFF0000), 0);
            }
            
            // Force refresh
            lv_obj_invalidate(grey_icon);
            if (red_icon) lv_obj_invalidate(red_icon);
        }
    }
}
