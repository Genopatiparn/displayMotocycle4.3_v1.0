#include "drag_screen_handler.h"
#include "KLine.h"
#include "gps_sr1723.h"
#include <stdio.h>
#include <math.h>
#include <string.h>

extern "C" {
extern lv_obj_t *drag_ui_dragScreen;
extern void drag_ui_dragScreen_screen_init(void);
extern void drag_ui_dragScreen_screen_destroy(void);

// Left side - Engine data
extern lv_obj_t *drag_ui_Label7;   // RPM (88888)
extern lv_obj_t *drag_ui_Label2;   // Speed GPS (888)
extern lv_obj_t *drag_ui_Label4;   // Temp (888)
extern lv_obj_t *drag_ui_Label6;   // AFR (12.8)

// Right side - Drag timing
extern lv_obj_t *drag_ui_Label10;  // Main time display (00.00)
extern lv_obj_t *drag_ui_TypeDrag; // State text (R/T)
extern lv_obj_t *drag_ui_Container2;
extern lv_obj_t *drag_ui_timeRT;   // R/T time 
extern lv_obj_t *drag_ui_time60ft;
extern lv_obj_t *drag_ui_time330ft;
extern lv_obj_t *drag_ui_time18mi;
extern lv_obj_t *drag_ui_time1000ft;
extern lv_obj_t *drag_ui_time14;
}

#define EARTH_RADIUS_M 6371000.0f
#define DRAG_60FT_M     18.288f
#define DRAG_330FT_M    100.584f
#define DRAG_660FT_M    201.168f
#define DRAG_1000FT_M   304.8f
#define DRAG_1320FT_M   402.336f

#define SPEED_START_THRESHOLD 2.0f
static constexpr uint32_t MAX_GPS_FIX_AGE_MS = 250;

typedef enum {
    DRAG_IDLE,
    DRAG_STAGED,
    DRAG_RUNNING,
    DRAG_FINISHED
} DragState;

static struct {
    DragState state;
    uint32_t time_start;
    uint32_t time_staged;
    uint32_t time_rt;
    uint32_t time_60ft;
    uint32_t time_330ft;
    uint32_t time_660ft;
    uint32_t time_1000ft;
    uint32_t time_1320ft;
    double start_lat;
    double start_lon;
    double distance_m;           // Total accumulated distance
    double previous_distance_m;
    double last_lat;             // Last GPS position for incremental distance
    double last_lon;
    uint32_t previous_fix_time_ms;
    bool previous_fix_valid;
    bool passed_60ft;
    bool passed_330ft;
    bool passed_660ft;
    bool passed_1000ft;
    bool passed_1320ft;
} drag_data;

static char buf[32];
static uint32_t last_update = 0;

static double deg2rad(double deg) {
    return deg * (M_PI / 180.0);
}

static double calculate_distance(double lat1, double lon1, double lat2, double lon2) {
    const double dLat = deg2rad(lat2 - lat1);
    const double dLon = deg2rad(lon2 - lon1);
    
    double a = sin(dLat / 2.0) * sin(dLat / 2.0) +
               cos(deg2rad(lat1)) * cos(deg2rad(lat2)) *
               sin(dLon / 2.0) * sin(dLon / 2.0);
    if (a < 0.0) a = 0.0;
    if (a > 1.0) a = 1.0;
    
    const double c = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));
    
    return EARTH_RADIUS_M * c;
}

static uint32_t interpolate_crossing_time_ms(double threshold_m,
                                             double previous_distance_m,
                                             double current_distance_m,
                                             uint32_t previous_elapsed_ms,
                                             uint32_t current_elapsed_ms) {
    if (current_distance_m <= previous_distance_m || current_elapsed_ms <= previous_elapsed_ms) {
        return current_elapsed_ms;
    }

    double fraction = (threshold_m - previous_distance_m) /
                      (current_distance_m - previous_distance_m);
    if (fraction < 0.0) fraction = 0.0;
    if (fraction > 1.0) fraction = 1.0;
    return previous_elapsed_ms + static_cast<uint32_t>(
        fraction * (current_elapsed_ms - previous_elapsed_ms));
}

void DragScreenHandler_Init(void)
{
    if (!drag_ui_dragScreen) {
        drag_ui_dragScreen_screen_init();
    }

    lv_obj_t *value_labels[] = {
        drag_ui_Label7,
        drag_ui_Label2,
        drag_ui_Label4,
        drag_ui_Label6
    };
    for (lv_obj_t *label : value_labels) {
        if (!label) continue;
        lv_obj_set_width(label, 110);
        lv_obj_set_x(label, -123);
        lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    
    drag_data.state = DRAG_IDLE;
    drag_data.time_start = 0;
    drag_data.time_staged = 0;
    drag_data.time_rt = 0;
    drag_data.time_60ft = 0;
    drag_data.time_330ft = 0;
    drag_data.time_660ft = 0;
    drag_data.time_1000ft = 0;
    drag_data.time_1320ft = 0;
    drag_data.start_lat = 0;
    drag_data.start_lon = 0;
    drag_data.distance_m = 0;
    drag_data.previous_distance_m = 0;
    drag_data.last_lat = 0;
    drag_data.last_lon = 0;
    drag_data.previous_fix_time_ms = 0;
    drag_data.previous_fix_valid = false;
    drag_data.passed_60ft = false;
    drag_data.passed_330ft = false;
    drag_data.passed_660ft = false;
    drag_data.passed_1000ft = false;
    drag_data.passed_1320ft = false;
    last_update = 0;

    if (drag_ui_Container2) {
        lv_obj_set_style_bg_color(drag_ui_Container2, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(drag_ui_Container2, 50, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    
    if (drag_ui_TypeDrag) lv_label_set_text(drag_ui_TypeDrag, "R/T");
    if (drag_ui_Label10) lv_label_set_text(drag_ui_Label10, "00.00");
    if (drag_ui_timeRT) lv_label_set_text(drag_ui_timeRT, "00.00");
    if (drag_ui_time60ft) lv_label_set_text(drag_ui_time60ft, "00.00");
    if (drag_ui_time330ft) lv_label_set_text(drag_ui_time330ft, "00.00");
    if (drag_ui_time18mi) lv_label_set_text(drag_ui_time18mi, "00.00");
    if (drag_ui_time1000ft) lv_label_set_text(drag_ui_time1000ft, "00.00");
    if (drag_ui_time14) lv_label_set_text(drag_ui_time14, "00.00");
}

void DragScreenHandler_Update(void)
{
    const uint32_t now = millis();
    
    if ((now - last_update) < 50) {
        return;
    }
    last_update = now;
    
    // Update left side - Engine data from K-Line
    KLineData kline = KLine_GetData();
    
    if (drag_ui_Label7) {
        snprintf(buf, sizeof(buf), "%u", kline.rpm);
        lv_label_set_text(drag_ui_Label7, buf);
    }
    
    if (drag_ui_Label4) {
        snprintf(buf, sizeof(buf), "%d", kline.engine_temp);
        lv_label_set_text(drag_ui_Label4, buf);
    }
    
    if (drag_ui_Label6) {
        snprintf(buf, sizeof(buf), "%.1f", kline.afr / 10.0f);
        lv_label_set_text(drag_ui_Label6, buf);
    }
    
    gps_fix_t gps_fix{};
    GPS_GetFix(&gps_fix);
    const bool gps_fix_fresh = gps_fix.valid &&
        (now - gps_fix.updated_at_ms) <= MAX_GPS_FIX_AGE_MS;
    const char *gps_quality = GPS_GetQualityStatus();
    const bool gps_quality_good = strcmp(gps_quality, "Good") == 0 ||
                                  strcmp(gps_quality, "Excellent") == 0;
    const bool gps_position_usable = gps_fix_fresh && gps_quality_good;
    const bool ready_and_gps_good = drag_data.state != DRAG_IDLE && gps_position_usable;
    const float gps_speed = gps_fix_fresh ? gps_fix.speed_kmh : 0.0f;
    if (drag_ui_Container2 && drag_data.state != DRAG_IDLE) {
        lv_obj_set_style_bg_color(drag_ui_Container2,
            lv_color_hex(ready_and_gps_good ? 0x00C853 : 0xFF0000),
            LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(drag_ui_Container2, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    if (drag_ui_Label2) {
        snprintf(buf, sizeof(buf), "%.0f", gps_speed);
        lv_label_set_text(drag_ui_Label2, buf);
    }
    
    // Drag mode state machine
    switch (drag_data.state) {
        case DRAG_IDLE:
            // Waiting for start
            break;
            
        case DRAG_STAGED:
            if (gps_position_usable && gps_speed >= SPEED_START_THRESHOLD) {
                drag_data.state = DRAG_RUNNING;
                drag_data.time_start = gps_fix.updated_at_ms;
                drag_data.time_rt = gps_fix.updated_at_ms - drag_data.time_staged;
                drag_data.start_lat = gps_fix.latitude_deg;
                drag_data.start_lon = gps_fix.longitude_deg;
                drag_data.last_lat = gps_fix.latitude_deg;
                drag_data.last_lon = gps_fix.longitude_deg;
                drag_data.distance_m = 0.0;
                drag_data.previous_distance_m = 0.0;
                drag_data.previous_fix_time_ms = gps_fix.updated_at_ms;
                drag_data.previous_fix_valid = true;
            }
            break;
            
        case DRAG_RUNNING:
            {
                const uint32_t elapsed = now - drag_data.time_start;
                if (drag_ui_Label10) {
                    snprintf(buf, sizeof(buf), "%.2f", elapsed / 1000.0f);
                    lv_label_set_text(drag_ui_Label10, buf);
                }

                if (!gps_fix_fresh ||
                    gps_fix.updated_at_ms == drag_data.previous_fix_time_ms) {
                    break;
                }

                // Calculate incremental distance from last position (accumulated distance)
                double increment_m = calculate_distance(
                    drag_data.last_lat,
                    drag_data.last_lon,
                    gps_fix.latitude_deg,
                    gps_fix.longitude_deg
                );
                
                // Filter out GPS noise: ignore very small movements or unrealistic jumps
                const double MIN_INCREMENT_M = 0.5;   // Ignore sub-meter noise
                const double MAX_INCREMENT_M = 50.0;  // Ignore 50m+ jumps (GPS glitch)
                
                if (increment_m >= MIN_INCREMENT_M && increment_m <= MAX_INCREMENT_M) {
                    drag_data.distance_m += increment_m;
                }
                
                // Update last position for next increment
                drag_data.last_lat = gps_fix.latitude_deg;
                drag_data.last_lon = gps_fix.longitude_deg;

                const uint32_t fix_elapsed_ms = gps_fix.updated_at_ms - drag_data.time_start;
                const uint32_t previous_elapsed_ms = drag_data.previous_fix_valid
                    ? drag_data.previous_fix_time_ms - drag_data.time_start
                    : fix_elapsed_ms;
                const double previous_distance_m = drag_data.previous_fix_valid
                    ? drag_data.previous_distance_m
                    : drag_data.distance_m;
                
                if (!drag_data.passed_60ft && drag_data.distance_m >= DRAG_60FT_M) {
                    drag_data.passed_60ft = true;
                    drag_data.time_60ft = interpolate_crossing_time_ms(
                        DRAG_60FT_M, previous_distance_m, drag_data.distance_m,
                        previous_elapsed_ms, fix_elapsed_ms);
                }
                
                if (!drag_data.passed_330ft && drag_data.distance_m >= DRAG_330FT_M) {
                    drag_data.passed_330ft = true;
                    drag_data.time_330ft = interpolate_crossing_time_ms(
                        DRAG_330FT_M, previous_distance_m, drag_data.distance_m,
                        previous_elapsed_ms, fix_elapsed_ms);
                }
                
                if (!drag_data.passed_660ft && drag_data.distance_m >= DRAG_660FT_M) {
                    drag_data.passed_660ft = true;
                    drag_data.time_660ft = interpolate_crossing_time_ms(
                        DRAG_660FT_M, previous_distance_m, drag_data.distance_m,
                        previous_elapsed_ms, fix_elapsed_ms);
                }
                
                if (!drag_data.passed_1000ft && drag_data.distance_m >= DRAG_1000FT_M) {
                    drag_data.passed_1000ft = true;
                    drag_data.time_1000ft = interpolate_crossing_time_ms(
                        DRAG_1000FT_M, previous_distance_m, drag_data.distance_m,
                        previous_elapsed_ms, fix_elapsed_ms);
                }
                
                if (!drag_data.passed_1320ft && drag_data.distance_m >= DRAG_1320FT_M) {
                    drag_data.passed_1320ft = true;
                    drag_data.time_1320ft = interpolate_crossing_time_ms(
                        DRAG_1320FT_M, previous_distance_m, drag_data.distance_m,
                        previous_elapsed_ms, fix_elapsed_ms);
                    drag_data.state = DRAG_FINISHED;
                    if (drag_ui_TypeDrag) lv_label_set_text(drag_ui_TypeDrag, "FINISH");
                }

                drag_data.previous_distance_m = drag_data.distance_m;
                drag_data.previous_fix_time_ms = gps_fix.updated_at_ms;
                drag_data.previous_fix_valid = true;
            }
            break;
            
        case DRAG_FINISHED:
            // Show final time
            if (drag_ui_Label10) {
                snprintf(buf, sizeof(buf), "%.2f", drag_data.time_1320ft / 1000.0f);
                lv_label_set_text(drag_ui_Label10, buf);
            }
            break;
    }
}

void DragScreenHandler_RefreshTimes(void)
{
    // Refresh all recorded times that have been passed
    // This ensures they stay visible even if LVGL recreates objects
    
    if (drag_data.passed_60ft && drag_ui_time60ft) {
        snprintf(buf, sizeof(buf), "%.2f", drag_data.time_60ft / 1000.0f);
        lv_label_set_text(drag_ui_time60ft, buf);
    }
    
    if (drag_data.passed_330ft && drag_ui_time330ft) {
        snprintf(buf, sizeof(buf), "%.2f", drag_data.time_330ft / 1000.0f);
        lv_label_set_text(drag_ui_time330ft, buf);
    }
    
    if (drag_data.passed_660ft && drag_ui_time18mi) {
        snprintf(buf, sizeof(buf), "%.2f", drag_data.time_660ft / 1000.0f);
        lv_label_set_text(drag_ui_time18mi, buf);
    }
    
    if (drag_data.passed_1000ft && drag_ui_time1000ft) {
        snprintf(buf, sizeof(buf), "%.2f", drag_data.time_1000ft / 1000.0f);
        lv_label_set_text(drag_ui_time1000ft, buf);
    }
    
    if (drag_data.passed_1320ft && drag_ui_time14) {
        snprintf(buf, sizeof(buf), "%.2f", drag_data.time_1320ft / 1000.0f);
        lv_label_set_text(drag_ui_time14, buf);
    }
    
    if (drag_data.time_rt > 0 && drag_ui_timeRT) {
        snprintf(buf, sizeof(buf), "%.2f", drag_data.time_rt / 1000.0f);
        lv_label_set_text(drag_ui_timeRT, buf);
    }
}

void DragScreenHandler_Start(void)
{
    drag_data.state = DRAG_STAGED;
    drag_data.time_staged = millis();
    drag_data.time_rt = 0;
    drag_data.time_60ft = 0;
    drag_data.time_330ft = 0;
    drag_data.time_660ft = 0;
    drag_data.time_1000ft = 0;
    drag_data.time_1320ft = 0;
    drag_data.distance_m = 0;
    drag_data.previous_distance_m = 0;
    drag_data.last_lat = 0;
    drag_data.last_lon = 0;
    drag_data.previous_fix_valid = false;
    drag_data.passed_60ft = false;
    drag_data.passed_330ft = false;
    drag_data.passed_660ft = false;
    drag_data.passed_1000ft = false;
    drag_data.passed_1320ft = false;

    if (drag_ui_Label10) lv_label_set_text(drag_ui_Label10, "00.00");
}

void DragScreenHandler_Reset(void)
{
    DragScreenHandler_Init();
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
