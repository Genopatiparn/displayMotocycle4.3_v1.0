# วิธีแก้ปัญหาลูกศรไม่แสดง

## สรุป
โปรเจคอ้างอิง `display_43inch` **ไม่ได้ใช้ลูกศร** (comment ออกไว้)  
แต่โปรเจคของคุณ **ต้องการใช้ลูกศร** (`ui_Image3` ใช้ `ui_img_arrow_1_png`)

## ปัญหา
รูป `Arrow_1.png` ถูก export เป็น `LV_COLOR_FORMAT_I8` (indexed color)  
ซึ่ง LVGL 9.5 มีปัญหาในการแสดงผล

## วิธีแก้ (แนะนำ)

### ใน SquareLine Studio:

1. เปิด **SquareLine Studio**
2. เปิดโปรเจค **SquareLine_Project_RacePanel42**
3. ไปที่แท็บ **Assets** (ด้านขวา)
4. หารูป `Arrow_1.png`
5. คลิกขวา → **Image Settings** หรือ **Properties**
6. เปลี่ยนการตั้งค่า:
   ```
   Color format: I8 → RGB565A8
   หรือ
   Color format: I8 → True color with alpha
   ```
7. คลิก **Apply**
8. **Export UI files**:
   - File → Export → Export UI Files
   - หรือกด Ctrl+E
   - เลือกโฟลเดอร์ export: `src/application/ui_home/`
   - คลิก Export

9. กลับมา **PlatformIO**:
   - Build โปรเจค
   - Upload
   - ทดสอบ

## ตรวจสอบผลลัพธ์

รูปลูกศรควรแสดงที่ตำแหน่งกลาง speedometer และหมุนตามค่า Arc

## หมายเหตุ

- **โปรเจคอ้างอิง (display_43inch)**: ไม่ใช้ลูกศร ใช้แค่ Arc indicator
- **โปรเจคของคุณ**: ใช้ทั้ง Arc + ลูกศร (ดีกว่า!)
- หากไม่อยากใช้ลูกศร ให้ comment โค้ด `ui_Image3` ออกเหมือนโปรเจคอ้างอิง

## ทางเลือกอื่น (ไม่แนะนำ)

หากไม่มี SquareLine Studio:
1. ใช้ online converter แปลง PNG → C Array (RGB565A8)
2. แทนที่ไฟล์ `ui_img_arrow_1_png.c`
3. เปลี่ยน color format ใน descriptor

แต่วิธีนี้ยุ่งยากกว่า ควรใช้ SquareLine Studio
