#include <Arduino.h>

#include "Display_RGB.h"
#include "LVGL_Driver.h"
#include "hardware_config.h"
#include "adc_button.h"
#include "screen_manager.h"
#include "ui_home/ui.h"
#include "ws2811_control_2.h"
#include "KLine.h"
#include "gps_sr1723.h"
#include "application/handlers/radar_screen_handler.h"
#include "application/handlers/menu_screen_handler.h"

uint16_t rpm = 0;
uint16_t speed = 0;
uint16_t afr = 147;
uint16_t tps = 0;
uint16_t map_val = 0;
uint16_t inj_duty = 0;
int16_t ignition = 0;
int16_t engine_temp = 0;
uint16_t tinj = 0;

static int32_t last_rpm = -1000;
static int32_t last_speed = -1000;
static int32_t last_tps = -1000;
static int32_t last_map = -1000;
static int32_t last_afr = -1000;
static int16_t last_ignition = -1000;
static int32_t last_tinj = -1000;
static int32_t last_angle = -1000;
static int32_t last_inj_duty = -1000;
static int32_t last_engine_temp = -1000;

unsigned long last_ui_update = 0;
const uint8_t UI_UPDATE_INTERVAL = 50;

static button_status_t last_btn12 = BTN_NONE;
static button_status_t last_btn34 = BTN_NONE;
static unsigned long last_btn_debug_time = 0;
static bool adc_initialized = false;
bool led_initialized = false;

// GPS variables
static bool gps_initialized = false;
static unsigned long last_gps_print = 0;

TaskHandle_t KLineTask;

void KLineTaskCode(void * pvParameters) {
    for(;;) {
        KLine_Poll(); 
        vTaskDelay(10 / portTICK_PERIOD_MS);  // 10ms delay to prevent watchdog
    }
}

// GPS Task - runs on Core 0 to handle 230400 bps without LVGL blocking
void GPSTaskCode(void * pvParameters) {
    for(;;) {
        if (gps_initialized) {
            GPS_ProcessIncoming();
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);  // 100 Hz polling
    }
}

void setup()
{
    Serial.begin(115200);
    delay(2000);
    Serial.println("\n\n=== ESP32-S3 Motorcycle Display ===");

    adc_initialized = ADC_Button_Init();
    
    LCD_RGB_Init();
    Lvgl_Init();
    LCD_RGB_BacklightInit();

    KLine_Init();
    ScreenManager_Init();
    
    led_initialized = WS2811_Init();
    if (led_initialized) {
        WS2811_Test();
    }

    Serial.println("Initializing GPS SR1723...");
    GPS_Init();  // Already calls GPS_ConfigurePureUBX() inside
    gps_initialized = true;
    Serial.println("GPS initialized - waiting for satellite lock...");

    // Create K-Line Task on Core 0 with Priority 2 (higher)
    xTaskCreatePinnedToCore(
        KLineTaskCode,   
        "KLineTask",     
        4096,            
        NULL,            
        2,               // Priority 2 (higher than GPS)
        &KLineTask,      
        0);              
    
    // Create GPS Task on Core 0 with Priority 1 (lower than K-Line)
    xTaskCreatePinnedToCore(
        GPSTaskCode,
        "GPSTask",
        4096,
        NULL,
        1,               // Priority 1 (lower than K-Line)
        NULL,
        0);              
}

void loop()
{
    const KLineData data = KLine_GetData();
    rpm = data.rpm;
    engine_temp = data.engine_temp;
    map_val = data.map;
    tps = data.tps;
    ignition = data.ignition;
    tinj = data.tinj;
    afr = data.afr;
    speed = data.speed;
    
    // ดึงค่า inj_duty ที่คำนวณอย่างแม่นยำมาจาก K-Line โดยตรง!
    inj_duty = data.inj_duty;

    if (millis() - last_ui_update >= UI_UPDATE_INTERVAL) {
        last_ui_update = millis();

        if (led_initialized) {
            WS2811_ShowRPM(rpm, 12000);
        }

        if (abs((int)rpm - (int)last_rpm) > 15 || rpm == 0) {
            lv_label_set_text_fmt(ui_Label1, "%d", rpm);
            
            int16_t angle = (rpm * 2700) / 12000;
            if (abs(angle - last_angle) >= 2) {
                int32_t delta = angle - last_angle;
                if (delta < 0) delta = -delta;
                
                lv_arc_set_value(ui_Arc1, angle); 
                if (delta >= 5) { 
                    lv_image_set_rotation(ui_Image3, angle);
                }
                last_angle = angle;
            }
            last_rpm = rpm;
        }

        if (speed != last_speed) {
            lv_label_set_text_fmt(ui_Label7, "%d", speed);
            last_speed = speed;
        }

        if (tps != last_tps) {
            lv_label_set_text_fmt(ui_Label17, "%d.%d", tps / 10, tps % 10);
            last_tps = tps;
        }

        if (tinj != last_tinj) {
            lv_label_set_text_fmt(ui_Label2, "%d.%d", tinj / 10, tinj % 10);
            lv_bar_set_value(ui_Bar1, (tinj * 100) / 150, LV_ANIM_ON);
            last_tinj = tinj;
        }

        if (ignition != last_ignition) {
            lv_label_set_text_fmt(ui_Label10, "%d.%d", ignition / 10, abs(ignition % 10));
            lv_bar_set_value(ui_Bar2, (ignition * 100) / 600, LV_ANIM_ON);
            last_ignition = ignition;
        }

        if (afr != last_afr) {
            lv_label_set_text_fmt(ui_Label15, "%d.%d", afr / 10, afr % 10);
            last_afr = afr;
        }

        if (map_val != last_map) {
            lv_label_set_text_fmt(ui_Label20, "%d", map_val); 
            last_map = map_val;
        }

        if (inj_duty != last_inj_duty) {
            lv_label_set_text_fmt(ui_Label5, "%d", inj_duty);
            lv_bar_set_value(ui_Bar3, inj_duty, LV_ANIM_ON);
            last_inj_duty = inj_duty;
        }

        if (engine_temp != last_engine_temp) {
            lv_label_set_text_fmt(ui_Label22, "%d", engine_temp);
            lv_bar_set_value(ui_Bar4, (engine_temp * 100) / 120, LV_ANIM_ON);
            last_engine_temp = engine_temp;
        }
    }

    static unsigned long last_adc_update = 0;
    if (adc_initialized && millis() - last_adc_update >= 50) {
        last_adc_update = millis();
        ADC_Update();
    }

    button_status_t btn12 = adc_initialized ? ADC_ReadButton_SW12() : BTN_NONE;
    button_status_t btn34 = adc_initialized ? ADC_ReadButton_SW34() : BTN_NONE;
    ScreenManager_HandleButton(btn12, btn34);

    static button_status_t last_logged_btn12 = BTN_NONE;
    static button_status_t last_logged_btn34 = BTN_NONE;
    if (btn12 != last_logged_btn12 || btn34 != last_logged_btn34) {
        last_logged_btn12 = btn12;
        last_logged_btn34 = btn34;
        Serial.printf("[BUTTON] SW12=%d (%.1fmV), SW34=%d (%.1fmV)\n",
                      btn12, ADC_ReadVoltage_mV(ADS1115_CH_SW1),
                      btn34, ADC_ReadVoltage_mV(ADS1115_CH_SW2));
    }

    // GPS UI updates (GPS_ProcessIncoming now runs in separate task)
    if (gps_initialized) {
        static unsigned long last_gps_ui_update = 0;
        if (millis() - last_gps_ui_update >= 1000) {  // Update UI every 1000ms (1 Hz) - save CPU
            last_gps_ui_update = millis();
            
            // Update GPS UI based on current screen
            screen_type_t current_screen = ScreenManager_GetCurrentScreen();
            if (current_screen == SCREEN_RADAR) {
                RadarScreenHandler_UpdateGPS();
            } else if (current_screen == SCREEN_MENU) {
                MenuScreenHandler_UpdateGPS();
            }
        }
        
        // Print GPS status every 5 seconds
        if (millis() - last_gps_print >= 5000) {
            last_gps_print = millis();
            
            if (GPS_HasFix()) {
                gps_debug_info_t gps_info;
                GPS_GetDebugInfo(&gps_info);
                uint8_t sats = GPS_GetSatellites();
                uint8_t visible = GPS_GetVisibleSatellites();
                Serial.printf("[GPS] FIX   | Pos: %s,%s | Speed: %s km/h | Time: %s | Sats: %d/%d | %s\n",
                              gps_info.lat, gps_info.lon, gps_info.speed_kmh, 
                              gps_info.utc_time, sats, visible, GPS_GetQualityStatus());
            } else {
                uint8_t visible = GPS_GetVisibleSatellites();
                Serial.printf("[GPS] NO FIX | Visible satellites: %d | Status: %s\n", 
                              visible, GPS_GetQualityStatus());
            }
        }
    }

    lv_timer_handler();
    delay(5);
}