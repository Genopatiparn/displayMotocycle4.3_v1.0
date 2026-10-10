/**
 * @file ui_radarmode_gps.cpp
 * @brief GPS Status Display for Radar Mode Screen Implementation
 */

#include "ui_radarmode_gps.h"
#include "gps_sr1723.h"
#include "application/ui_radarmode/ui.h"
#include "application/ui_radarmode/ui_Screen1.h"  // Use header that exports externs
#include <Arduino.h>

// NOTE: ui_Bar1-5 and ui_Image3 are used in radar_screen_handler.cpp now
// to avoid linker issues with ui_Screen1.c symbols

void UIRadarMode_GPS_Init(void) {
    // Nothing to initialize yet
}

void UIRadarMode_GPS_Update(void) {
    // Check if UI objects exist
    if (!ui_Label5 || !ui_Label6) {
        static bool warned = false;
        if (!warned) {
            warned = true;
        }
        return;
    }

    // Get GPS data
    uint8_t sats_in_use = GPS_GetSatellites();
    uint8_t sats_visible = GPS_GetVisibleSatellites();
    const char* quality = GPS_GetQualityStatus();
    
    // Update satellite count (show visible count)
    lv_label_set_text_fmt(ui_Label5, "%d", sats_visible);
    
    // Update quality status
    lv_label_set_text(ui_Label6, quality);
    
    // NOTE: GPS icon (ui_Image3) is updated in radar_screen_handler.cpp
    
    // Change color based on quality
    if (strcmp(quality, "Excellent") == 0) {
        lv_obj_set_style_text_color(ui_Label6, lv_color_hex(0x00FF00), LV_PART_MAIN); // Green
    } else if (strcmp(quality, "Good") == 0) {
        lv_obj_set_style_text_color(ui_Label6, lv_color_hex(0x00FF00), LV_PART_MAIN); // Green
    } else if (strcmp(quality, "Moderate") == 0) {
        lv_obj_set_style_text_color(ui_Label6, lv_color_hex(0xFFFF00), LV_PART_MAIN); // Yellow
    } else if (strcmp(quality, "Fair") == 0) {
        lv_obj_set_style_text_color(ui_Label6, lv_color_hex(0xFFA500), LV_PART_MAIN); // Orange
    } else { // Bad
        lv_obj_set_style_text_color(ui_Label6, lv_color_hex(0xFF0000), LV_PART_MAIN); // Red
    }
    
    // NOTE: Satellite bars are updated in radar_screen_handler.cpp
    
    // Update time from GPS (convert UTC to UTC+7 for Thailand)
    gps_debug_info_t info;
    GPS_GetDebugInfo(&info);
    
    if (ui_Label4 && info.utc_time[0] != '\0') {
        // Parse UTC time (format: "HH:MM:SS")
        int hour = 0, minute = 0, second = 0;
        if (sscanf(info.utc_time, "%d:%d:%d", &hour, &minute, &second) == 3) {
            // Convert UTC to UTC+7 (Thailand time)
            hour += 7;
            
            // Handle day overflow
            if (hour >= 24) {
                hour -= 24;
            }
            
            // Format and display local time
            char local_time[16];
            snprintf(local_time, sizeof(local_time), "%02d:%02d:%02d", hour, minute, second);
            lv_label_set_text(ui_Label4, local_time);
        } else {
            // Failed to parse, show UTC as-is
            lv_label_set_text(ui_Label4, info.utc_time);
        }
    } else if (ui_Label4) {
        // No time data available
        lv_label_set_text(ui_Label4, "--:--");
    }
}
