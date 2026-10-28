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
#include "application/handlers/drag_screen_handler.h"
#include "application/handlers/customUIHandler.h"
#include "application/handlers/wifiSettingHandler.h"

extern bool kline_use_handshake;
extern bool kline_handshake_done;

uint16_t rpm = 0;
uint16_t speed = 0;
uint16_t afr = 0;  // 0.0 AFR (รอข้อมูลจริงจาก K-Line)
uint16_t tps = 0;
uint16_t map_val = 0;
uint16_t inj_duty = 0;
uint16_t battery = 0;
int16_t ignition = 0;
int16_t engine_temp = 0;
float stf = 0.0f;
float ltf = 0.0f;
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

    Serial.println("[K-Line Task] Starting K-Line initialization...");
    KLine_Init();
    kline_task_init_done = true;

    for (;;) {
        if (kline_handshake_done) {
            KLine_Poll();
        } else {
            Serial.println("[K-Line Task] Connection Lost! Retrying Handshake in 2 seconds...");
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
    Serial.println("\n\n=== ESP32-S3 Motorcycle Display - FAST BOOT ===");
    WifiSettingHandler_ConnectSavedNetwork();

    LCD_RGB_Init();
    Lvgl_Init();
    LCD_RGB_BacklightInit();
    ScreenManager_Init();
    Serial.println("[BOOT] Display ready!");

    adc_initialized = ADC_Button_Init();
    led_initialized = WS2811_Init();
    kline_use_handshake = true;

    GPS_Init();
    gps_initialized = true;

    xTaskCreatePinnedToCore(KLineTaskCode, "KLineTask", 8192, NULL, 3, &KLineTask, 0);
    xTaskCreatePinnedToCore( GPSTaskCode, "GPSTask", 4096, NULL, 1, NULL, 0);

    Serial.println("[BOOT] All systems ready - Display online!");
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
    battery = data.battery_voltage_x10;
    stf = data.stf_raw * 0.0078f;
    ltf = data.ltf_raw * 0.0078f;

    if (millis() - last_ui_update >= UI_UPDATE_INTERVAL) {
        last_ui_update = millis();

        if (led_initialized) {
            WS2811_ShowRPM(rpm, WifiSettingHandler_GetMaxRpm());
        }

        if (ui_Screen1 != nullptr && lv_screen_active() == ui_Screen1) {
            if (abs((int)rpm - (int)last_rpm) > 5 || rpm == 0) {
                const int32_t max_rpm = WifiSettingHandler_GetMaxRpm();
                const int32_t angle = max_rpm > 0
                    ? ((int32_t)rpm * 2700) / max_rpm
                    : 0;
                const int32_t clamped_angle = angle > 2700 ? 2700 : angle;

                if (abs(clamped_angle - last_angle) >= 5) {
                    lv_arc_set_value(ui_Arc1, clamped_angle);
                    lv_image_set_rotation(ui_Image3, clamped_angle);
                    last_angle = clamped_angle;
                }
                lv_label_set_text_fmt(ui_Label1, "%u", (unsigned)rpm);
                last_rpm = rpm;
            }
            CustomUIHandler_Update();
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
    WifiSettingHandler_Update();

    static button_status_t last_logged_btn12 = BTN_NONE;
    static button_status_t last_logged_btn34 = BTN_NONE;
    if (btn12 != last_logged_btn12 || btn34 != last_logged_btn34) {
        last_logged_btn12 = btn12;
        last_logged_btn34 = btn34;
        Serial.printf("[BUTTON] SW12=%d (%.1fmV), SW34=%d (%.1fmV)\n",
                      btn12, ADC_ReadVoltage_mV(ADS1115_CH_SW1),
                      btn34, ADC_ReadVoltage_mV(ADS1115_CH_SW2));
    }

    if (gps_initialized) {
        static unsigned long last_gps_ui_update = 0;
        screen_type_t current_screen = ScreenManager_GetCurrentScreen();
        
        if (current_screen == SCREEN_DRAG) {
            if (millis() - last_gps_ui_update >= 50) {
                last_gps_ui_update = millis();
                DragScreenHandler_Update();
            }
        } else {
            if (millis() - last_gps_ui_update >= 1000) {
                last_gps_ui_update = millis();
                
                if (current_screen == SCREEN_RADAR) {
                    RadarScreenHandler_UpdateGPS();
                } else if (current_screen == SCREEN_MENU) {
                    MenuScreenHandler_UpdateGPS();
                }
            }
        }
    }

    lv_timer_handler();
    delay(5);
    
    static unsigned long last_drag_time_refresh = 0;
    if (gps_initialized && ScreenManager_GetCurrentScreen() == SCREEN_DRAG) {
        if (millis() - last_drag_time_refresh >= 100) {
            last_drag_time_refresh = millis();
            DragScreenHandler_RefreshTimes();
        }
    }
}