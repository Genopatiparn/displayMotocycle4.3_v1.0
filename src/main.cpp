#include <Arduino.h>

#include "Display_RGB.h"
#include "LVGL_Driver.h"
#include "application/ui_home/ui.h"

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

void setup()
{
    Serial.begin(115200);
    delay(2000);

    LCD_RGB_Init();
    Lvgl_Init();
    ui_init();
    LCD_RGB_BacklightInit();
}

void loop()
{
    // ========== 1. จำลองข้อมูลให้ทำงานไปเรื่อยๆ (เร็วๆ) ==========
    rpm += 33;
    rpm %= 12300;

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

    // ========== 2. อัปเดต UI ทุกๆ 50ms (20 FPS) - ไม่ใช้ Animation แบบตัวอย่าง ==========
    if (millis() - last_ui_update >= UI_UPDATE_INTERVAL) {
        last_ui_update = millis();

        // RPM Label - อัปเดตตรงๆ ไม่ animation
        if (rpm != last_rpm) {
            lv_label_set_text_fmt(ui_Label1, "%d", rpm);
            last_rpm = rpm;
        }

        // Arc + Arrow - คำนวณมุมและตั้งค่าตรงๆ แบบตัวอย่าง
        int16_t angle = (rpm * 2700) / 12000;
        if (angle != last_angle) {
            // ตรวจสอบว่า delta มากพอจะอัปเดต (แบบตัวอย่าง)
            int32_t delta = angle - last_angle;
            if (delta < 0) delta = -delta;
            
            lv_arc_set_value(ui_Arc1, angle);  // ไม่ animation
            
            if (delta >= 5) {  // อัปเดต arrow ถ้าเปลี่ยนแปลงมากพอ
                lv_image_set_rotation(ui_Image3, angle);  // ไม่ animation
            }
            last_angle = angle;
        }

        // Speed
        if (speed != last_speed) {
            lv_label_set_text_fmt(ui_Label7, "%d", speed);
            last_speed = speed;
        }

        // TPS - มีทศนิยม 1 ตำแหน่ง
        if (tps != last_tps) {
            lv_label_set_text_fmt(ui_Label17, "%d.%d", tps / 10, tps % 10);
            last_tps = tps;
        }

        // Tinj - มีทศนิยม 1 ตำแหน่ง
        if (tinj != last_tinj) {
            lv_label_set_text_fmt(ui_Label2, "%d.%d", tinj / 10, tinj % 10);
            last_tinj = tinj;
        }

        // Ignition - มีทศนิยม 1 ตำแหน่ง
        if (ignition != last_ignition) {
            lv_label_set_text_fmt(ui_Label10, "%d.%d", ignition / 10, ignition % 10);
            last_ignition = ignition;
        }

        // AFR - มีทศนิยม 1 ตำแหน่ง
        if (afr != last_afr) {
            lv_label_set_text_fmt(ui_Label15, "%d.%d", afr / 10, afr % 10);
            last_afr = afr;
        }

        // MAP - มีทศนิยม 1 ตำแหน่ง
        if (map_val != last_map) {
            lv_label_set_text_fmt(ui_Label20, "%d.%d", map_val / 10, map_val % 10);
            last_map = map_val;
        }

        // Inj Duty - integer
        if (inj_duty != last_inj_duty) {
            lv_label_set_text_fmt(ui_Label5, "%d", inj_duty);
            last_inj_duty = inj_duty;
        }

        // Engine Temp - integer
        if (engine_temp != last_engine_temp) {
            lv_label_set_text_fmt(ui_Label22, "%d", engine_temp);
            last_engine_temp = engine_temp;
        }

        // อัปเดต Bar - ใช้ LV_ANIM_OFF เพื่อไม่ให้ LVGL ทำ animation
        lv_bar_set_value(ui_Bar1, (tinj * 100) / 150, LV_ANIM_OFF);
        lv_bar_set_value(ui_Bar2, (ignition * 100) / 600, LV_ANIM_OFF);
        lv_bar_set_value(ui_Bar3, inj_duty, LV_ANIM_OFF);
        lv_bar_set_value(ui_Bar4, (engine_temp * 100) / 120, LV_ANIM_OFF);
    }

    // ========== 3. ให้ LVGL จัดการวาดหน้าจอ (ต้องเรียกบ่อยๆ) ==========
    lv_timer_handler();
    delay(5);
}