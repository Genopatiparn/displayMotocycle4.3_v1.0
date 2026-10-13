#include "drag_screen_handler.h"
#include "KLine.h"
#include "gps_sr1723.h"
#include <stdio.h>
#include <math.h>

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

#define SPEED_START_THRESHOLD 5.0f
#define SPEED_STAGED_THRESHOLD 2.0f

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
    float start_lat;
    float start_lon;
    float distance_m;
    bool passed_60ft;
    bool passed_330ft;
    bool passed_660ft;
    bool passed_1000ft;
    bool passed_1320ft;
} drag_data;

static char buf[32];
static uint32_t last_update = 0;

static float deg2rad(float deg) {
    return deg * (M_PI / 180.0f);
}

static float calculate_distance(float lat1, float lon1, float lat2, float lon2) {
    float dLat = deg2rad(lat2 - lat1);
    float dLon = deg2rad(lon2 - lon1);
    
    float a = sinf(dLat / 2.0f) * sinf(dLat / 2.0f) +
              cosf(deg2rad(lat1)) * cosf(deg2rad(lat2)) *
              sinf(dLon / 2.0f) * sinf(dLon / 2.0f);
    
    float c = 2.0f * atan2f(sqrtf(a), sqrtf(1.0f - a));
    
    return EARTH_RADIUS_M * c;
}

void DragScreenHandler_Init(void)
{
    if (!drag_ui_dragScreen) {
        drag_ui_dragScreen_screen_init();
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
    drag_data.passed_60ft = false;
    drag_data.passed_330ft = false;
    drag_data.passed_660ft = false;
    drag_data.passed_1000ft = false;
    drag_data.passed_1320ft = false;
    last_update = 0;
    
    if (drag_ui_TypeDrag) lv_label_set_text(drag_ui_TypeDrag, "READY");
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
    
    // GPS speed
    float gps_speed = GPS_GetSpeed();
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
            if (gps_speed >= SPEED_START_THRESHOLD) {
                drag_data.state = DRAG_RUNNING;
                drag_data.time_start = now;
                drag_data.time_rt = now - drag_data.time_staged;
                drag_data.start_lat = GPS_GetLatitude();
                drag_data.start_lon = GPS_GetLongitude();
                
                if (drag_ui_TypeDrag) lv_label_set_text(drag_ui_TypeDrag, "RUN");
                
                snprintf(buf, sizeof(buf), "%.2f", drag_data.time_rt / 1000.0f);
                if (drag_ui_timeRT) lv_label_set_text(drag_ui_timeRT, buf);
            }
            break;
            
        case DRAG_RUNNING:
            {
                float current_lat = GPS_GetLatitude();
                float current_lon = GPS_GetLongitude();
                drag_data.distance_m = calculate_distance(
                    drag_data.start_lat,
                    drag_data.start_lon,
                    current_lat,
                    current_lon
                );
                
                uint32_t elapsed = now - drag_data.time_start;
                
                if (drag_ui_Label10) {
                    snprintf(buf, sizeof(buf), "%.2f", elapsed / 1000.0f);
                    lv_label_set_text(drag_ui_Label10, buf);
                }
                
                if (!drag_data.passed_60ft && drag_data.distance_m >= DRAG_60FT_M) {
                    drag_data.passed_60ft = true;
                    drag_data.time_60ft = elapsed;
                    if (drag_ui_time60ft) {
                        snprintf(buf, sizeof(buf), "%.2f", elapsed / 1000.0f);
                        lv_label_set_text(drag_ui_time60ft, buf);
                    }
                }
                
                if (!drag_data.passed_330ft && drag_data.distance_m >= DRAG_330FT_M) {
                    drag_data.passed_330ft = true;
                    drag_data.time_330ft = elapsed;
                    if (drag_ui_time330ft) {
                        snprintf(buf, sizeof(buf), "%.2f", elapsed / 1000.0f);
                        lv_label_set_text(drag_ui_time330ft, buf);
                    }
                }
                
                if (!drag_data.passed_660ft && drag_data.distance_m >= DRAG_660FT_M) {
                    drag_data.passed_660ft = true;
                    drag_data.time_660ft = elapsed;
                    if (drag_ui_time18mi) {
                        snprintf(buf, sizeof(buf), "%.2f", elapsed / 1000.0f);
                        lv_label_set_text(drag_ui_time18mi, buf);
                    }
                }
                
                if (!drag_data.passed_1000ft && drag_data.distance_m >= DRAG_1000FT_M) {
                    drag_data.passed_1000ft = true;
                    drag_data.time_1000ft = elapsed;
                    if (drag_ui_time1000ft) {
                        snprintf(buf, sizeof(buf), "%.2f", elapsed / 1000.0f);
                        lv_label_set_text(drag_ui_time1000ft, buf);
                    }
                }
                
                if (!drag_data.passed_1320ft && drag_data.distance_m >= DRAG_1320FT_M) {
                    drag_data.passed_1320ft = true;
                    drag_data.time_1320ft = elapsed;
                    drag_data.state = DRAG_FINISHED;
                    if (drag_ui_time14) {
                        snprintf(buf, sizeof(buf), "%.2f", elapsed / 1000.0f);
                        lv_label_set_text(drag_ui_time14, buf);
                    }
                    if (drag_ui_TypeDrag) lv_label_set_text(drag_ui_TypeDrag, "FINISH");
                }
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
    drag_data.passed_60ft = false;
    drag_data.passed_330ft = false;
    drag_data.passed_660ft = false;
    drag_data.passed_1000ft = false;
    drag_data.passed_1320ft = false;
    
    if (drag_ui_TypeDrag) lv_label_set_text(drag_ui_TypeDrag, "STAGED");
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
