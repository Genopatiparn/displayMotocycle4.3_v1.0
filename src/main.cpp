#include <Arduino.h>

#include "Display_RGB.h"
#include "LVGL_Driver.h"
#include "hardware_config.h"
#include "adc_button.h"
#include "screen_manager.h"
#include "ui_home/ui.h"
#include "ws2811_control.h"

// ตัวแปรสำหรับจำลองข้อมูล
uint16_t rpm = 0;
uint16_t speed = 0;
uint16_t afr = 100;
uint16_t tps = 0;
uint16_t map_val = 0;
uint16_t inj_duty = 0;
uint16_t ignition = 200;
int16_t engine_temp = 0;
uint16_t tinj = 50;

// ตัวแปรเก็บค่าเก่าเพื่อตรวจสอบการเปลี่ยนแปลง
static int32_t last_rpm = -1;
static int32_t last_speed = -1;
static int32_t last_tps = -1;
static int32_t last_map = -1;
static int32_t last_afr = -1;
static int32_t last_ignition = -1;
static int32_t last_tinj = -1;
static int32_t last_angle = -1;
static int32_t last_inj_duty = -1;
static int32_t last_engine_temp = -1;

// ตัวแปรสำหรับจัดการเวลาอัปเดต UI (50ms = 20 FPS)
unsigned long last_ui_update = 0;
const uint8_t UI_UPDATE_INTERVAL = 50;

// ตัวแปรสำหรับ button debounce และ debug
static button_status_t last_btn12 = BTN_NONE;
static button_status_t last_btn34 = BTN_NONE;
static unsigned long last_btn_debug_time = 0;
static bool adc_initialized = false;

// ตัวแปรสำหรับ LED control
bool led_initialized = false;

void setup()
{
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("\n\n=== ESP32-S3 Motorcycle Display ===");

    // Initialize ADS1115 for button input
    Serial.println("Initializing ADS1115...");
    adc_initialized = ADC_Button_Init();
    if (!adc_initialized) {
        Serial.println("WARNING: ADS1115 initialization failed!");
        Serial.printf("Check I2C wiring (SDA=GPIO%d, SCL=GPIO%d, address=0x%02X)\n",
                      I2C_SDA_PIN, I2C_SCL_PIN, ADS1115_I2C_ADDR);
    }

    LCD_RGB_Init();
    Lvgl_Init();
    
    LCD_RGB_BacklightInit();
    
    // Initialize screen manager (will set Home as default)
    ScreenManager_Init();
    
    // Initialize WS2811 LEDs
    Serial.println("Initializing WS2811 LEDs...");
    led_initialized = WS2811_Init();
    
    if (led_initialized) {
        Serial.println("WS2811 initialized successfully");
        WS2811_Test();
    } else {
        Serial.println("WARNING: WS2811 initialization failed!");
    }
    
    Serial.println("Setup complete - System ready!");
    Serial.println("Press buttons to test...\n");
}

void loop()
{
    static unsigned long last_rpm_update = 0;
    if (millis() - last_rpm_update >= 50) {
        last_rpm_update = millis();
        rpm += 33;
        rpm %= 12000;
    }

    speed += 3;
    speed %= 200;
    
    afr += 2;
    afr %= 2000; 
    
    tps += 5;
    tps %= 1000;  
    
    map_val += 3;
    map_val %= 1000;  
    
    inj_duty += 2;
    inj_duty %= 100;
    
    ignition += 1;
    ignition %= 600; 
    
    engine_temp += 1;
    if (engine_temp > 120)
        engine_temp = 0;  
        
    tinj += 1;
    tinj %= 150;  

    if (millis() - last_ui_update >= UI_UPDATE_INTERVAL) {
        last_ui_update = millis();

        // Update WS2811 LEDs - แสดง RPM เท่านั้น
        if (led_initialized) {
            WS2811_ShowRPM(rpm, 12000);
        }

        if (rpm != last_rpm) {
            lv_label_set_text_fmt(ui_Label1, "%d", rpm);
            last_rpm = rpm;
        }

        int16_t angle = (rpm * 2700) / 12000;
        if (angle != last_angle) {

            int32_t delta = angle - last_angle;
            if (delta < 0) delta = -delta;
            
            lv_arc_set_value(ui_Arc1, angle); 
            
            if (delta >= 5) { 
                lv_image_set_rotation(ui_Image3, angle);
            }
            last_angle = angle;
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
            last_tinj = tinj;
        }

        if (ignition != last_ignition) {
            lv_label_set_text_fmt(ui_Label10, "%d.%d", ignition / 10, ignition % 10);  // ไม่ใส่องศาเพราะ font ไม่รองรับ
            last_ignition = ignition;
        }

        if (afr != last_afr) {
            lv_label_set_text_fmt(ui_Label15, "%d.%d", afr / 10, afr % 10);
            last_afr = afr;
        }

        if (map_val != last_map) {
            lv_label_set_text_fmt(ui_Label20, "%d.%d", map_val / 10, map_val % 10);
            last_map = map_val;
        }

        if (inj_duty != last_inj_duty) {
            lv_label_set_text_fmt(ui_Label5, "%d", inj_duty);
            last_inj_duty = inj_duty;
        }

        if (engine_temp != last_engine_temp) {
            lv_label_set_text_fmt(ui_Label22, "%d", engine_temp);
            last_engine_temp = engine_temp;
        }

        lv_bar_set_value(ui_Bar1, (tinj * 100) / 150, LV_ANIM_OFF);
        lv_bar_set_value(ui_Bar2, (ignition * 100) / 600, LV_ANIM_OFF);
        lv_bar_set_value(ui_Bar3, inj_duty, LV_ANIM_OFF);
        lv_bar_set_value(ui_Bar4, (engine_temp * 100) / 120, LV_ANIM_OFF);
    }

    static unsigned long last_adc_update = 0;
    if (adc_initialized && millis() - last_adc_update >= 50) {
        last_adc_update = millis();
        ADC_Update();
    }

    button_status_t btn12 = adc_initialized ? ADC_ReadButton_SW12() : BTN_NONE;
    button_status_t btn34 = adc_initialized ? ADC_ReadButton_SW34() : BTN_NONE;
    
    ScreenManager_HandleButton(btn12, btn34);
    
    if (btn12 != last_btn12) {
        last_btn12 = btn12;
        if (btn12 != BTN_NONE) {
            Serial.printf("Button SW1/2: %d\n", btn12);
        }
    }
    
    if (btn34 != last_btn34) {
        last_btn34 = btn34;
        if (btn34 != BTN_NONE) {
            Serial.printf("Button SW3/4: %d\n", btn34);
        }
    }
    
    if (adc_initialized && millis() - last_btn_debug_time > 500) {
        last_btn_debug_time = millis();
        ADC_PrintButtonStatus();
    }

    lv_timer_handler();
    delay(5);
}