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

extern bool kline_use_handshake;
extern bool kline_handshake_done;

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

TaskHandle_t KLineTask;
static bool kline_task_init_done = false;

void KLineTaskCode(void * pvParameters) {
    vTaskDelay(1500 / portTICK_PERIOD_MS);

    KLine_Init();
    kline_task_init_done = true;

    for (;;) {
        if (kline_handshake_done) {
            KLine_Poll();
        } else {
            vTaskDelay(2000 / portTICK_PERIOD_MS);
            KLine_Init();
        }
        vTaskDelay(5 / portTICK_PERIOD_MS);
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

    // ========== CRITICAL: จอติดก่อน! ==========
    LCD_RGB_Init();
    Lvgl_Init();
    LCD_RGB_BacklightInit();
    ScreenManager_Init();

    adc_initialized = ADC_Button_Init();
    
    led_initialized = WS2811_Init();

    kline_use_handshake = true;

    GPS_Init();
    gps_initialized = true;

    xTaskCreatePinnedToCore(
        KLineTaskCode,   
        "KLineTask",     
        8192,            //
        NULL,            
        3,               
        &KLineTask,      
        0);              
    
    xTaskCreatePinnedToCore(
        GPSTaskCode,
        "GPSTask",
        4096,
        NULL,
        1,
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
        
    }

    lv_timer_handler();
    delay(5);
}