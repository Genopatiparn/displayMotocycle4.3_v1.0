#ifndef KLINE_H
#define KLINE_H

#include <Arduino.h>

#define KLINE_MODE 0

struct KLineData {
    uint16_t rpm;          // รอบเครื่อง
    int16_t  engine_temp;  // อุณหภูมิเครื่องยนต์ (C)
    int16_t  map;          // MAP 
    int16_t  tps;          // TPS (คูณ 10)
    int16_t  ignition;     // องศาไฟจุดระเบิด (คูณ 10)
    int16_t  afr;          // AFR (เช่น 147 = 14.7)
    int16_t  tinj;         // ระยะเวลาฉีด (คูณ 10)
    int16_t  speed;        // ความเร็ว km/h
    uint16_t inj_duty;     // ภาระหัวฉีด (0-100%) 
};

void KLine_Init(void);
void KLine_Poll(void);
KLineData KLine_GetData(void);

#endif