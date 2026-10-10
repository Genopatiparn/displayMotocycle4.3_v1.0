/**
 * @file hardware_config.h
 * @brief Hardware pin configuration for display_43inch project
 *
 * ESP32-S3 GPIO assignments for motorcycle dashboard display
 *
 * Hardware Specifications:
 * - MCU: ESP32-S3 @ 240MHz
 * - Flash: 32MB (Octal SPI)
 * - PSRAM: 16MB (Octal SPI @ 80MHz)
 * - LCD: 4.3" 480x272 RGB 565 parallel (no SPI)
 */

#ifndef HARDWARE_CONFIG_H
#define HARDWARE_CONFIG_H

#include "driver/gpio.h"

// ==================== Communication Interfaces ====================

// Race Panel UART1 (binary 0x85-prefixed protocol)
// #define RACE_PANEL_TX_PIN    (GPIO_NUM_43)
// #define RACE_PANEL_RX_PIN    (GPIO_NUM_44)
// #define RACE_PANEL_UART_NUM  UART_NUM_1
// #define RACE_PANEL_BAUD_RATE 115200
// #define RACE_PANEL_BUF_SIZE  128

#define GPS_UART_NUM       UART_NUM_1
#define GPS_TX_PIN         (GPIO_NUM_16)  // ESP32 TX → GPS RXD
#define GPS_RX_PIN         (GPIO_NUM_15)  // ESP32 RX ← GPS TXD
#define GPS_BAUD_RATE      115200          // High-speed baud rate for UBX protocol (saved to flash)
#define GPS_RX_BUF_SIZE    2048

// K-Line (ISO 9141-2 / KWP2000) — UART2
#define KLINE_TX_PIN (GPIO_NUM_1)   // K-Line TX
#define KLINE_RX_PIN (GPIO_NUM_4)  // K-Line RX

// USB Debug (Native USB)
#define USB_D_MINUS_PIN (GPIO_NUM_19)
#define USB_D_PLUS_PIN (GPIO_NUM_20)

// ==================== Analog Inputs (ADC) ====================

// // Internal ESP32 ADC (not used - using ADS1115 instead)
// #define ADC_SW_PIN (GPIO_NUM_2)  // Switch input
// #define ADC_LDR_PIN (GPIO_NUM_6) // Light sensor
// #define ADC_12V_PIN (GPIO_NUM_7) // 12V monitor

// ==================== I2C for ADS1115 ====================

#define I2C_SDA_PIN (GPIO_NUM_48)  
#define I2C_SCL_PIN (GPIO_NUM_47)  
#define I2C_FREQ_HZ 100000         

// ADS1115 external ADC via I2C
#define ADS1115_I2C_ADDR 0x49      // ADDR pin tied to VDD
#define ADS1115_CH_SW1 0           // Channel 0: Switch group 1
#define ADS1115_CH_SW2 1           // Channel 1: Switch group 2
#define ADS1115_CH_V12 2           // Channel 2: 12V 
#define ADS1115_CH_LDR 3           // Channel 3: LDR

// ==================== PWM Output ====================

#define LCD_BACKLIGHT_PIN (GPIO_NUM_0) // Backlight PWM
#define LCD_BL_PWM_CHANNEL 1
#define LCD_BL_PWM_FREQ 1000     // 1 kHz
#define LCD_BL_PWM_RESOLUTION 10 // 10-bit (0-1023)

// ==================== LCD Control Pins (Direct GPIO) ====================

#define LCD_RESET_PIN (GPIO_NUM_42)   // LCD Reset
// #define LCD_SPI_CS_PIN (GPIO_NUM_42) // LCD SPI Chip Select - Dedicated pin

// ==================== LCD Module (RGB 565 Parallel Interface) ====================
// Hybrid mode: SPI (IO15/IO16) used only for register init; pixel data via 16-bit RGB parallel bus.
// COLMOD = 0x55 — 16-bit RGB565 DPI mode.
// 16 data pins driven: R[4:0] on panel D[15:11], G[5:0] on D[10:5], B[4:0] on D[4:0].
// ESP32 RGB panel: data_width=16, framebuffer=RGB565.
// IO36 and IO42 are NOT available for data pins (RESET and CS respectively).

// LCD Control Signals (RGB Parallel - Reference Only)
// #define LCD_PIN_RE              (GPIO_NUM_36)  // NOT AVAILABLE - IO36 dedicated to LCD_RESET_PIN
#define LCD_PIN_HSYNC (GPIO_NUM_40) // Horizontal sync
#define LCD_PIN_VSYNC (GPIO_NUM_41) // Vertical sync
#define LCD_PIN_DE    (GPIO_NUM_42) // Data enable
#define LCD_PIN_PCLK  (GPIO_NUM_38) // Pixel clock
#define LCD_PIN_DSIP  (GPIO_NUM_39)  
// #define LCD_PIN_CS              (GPIO_NUM_44)  // NOT AVAILABLE - IO44 dedicated to LCD_SPI_CS_PIN

// LCD Data Pins — Red channel (MSB first in RGB565)
#define LCD_PIN_R0              (GPIO_NUM_45)  // R0=LSB → DATA[0]
#define LCD_PIN_R1              (GPIO_NUM_21)   // R1
#define LCD_PIN_R2              (GPIO_NUM_14)   // R2
#define LCD_PIN_R3              (GPIO_NUM_13)  // R3
#define LCD_PIN_R4              (GPIO_NUM_12)  // R4=MSB → DATA[4]

// LCD Data Pins — Green channel
#define LCD_PIN_G0              (GPIO_NUM_11)  // G0=LSB → DATA[5]
#define LCD_PIN_G1              (GPIO_NUM_10)  // G1
#define LCD_PIN_G2              (GPIO_NUM_9)  // G2
#define LCD_PIN_G3              (GPIO_NUM_46)  // G3
#define LCD_PIN_G4              (GPIO_NUM_3)  // G4
#define LCD_PIN_G5              (GPIO_NUM_8)   // G5=MSB → DATA[10]

// LCD Data Pins — Blue channel
#define LCD_PIN_B0              (GPIO_NUM_18)   // B0=LSB → DATA[10]
#define LCD_PIN_B1              (GPIO_NUM_17)  // B1 → DATA[11]
#define LCD_PIN_B2              (GPIO_NUM_7)  // B2 → DATA[12]
#define LCD_PIN_B3              (GPIO_NUM_6)  // B3 → DATA[13]
#define LCD_PIN_B4              (GPIO_NUM_5)  // B4=MSB → DATA[14]

// LCD Specifications (4.3 inch 480x272 display)
#undef LCD_WIDTH
#undef LCD_HEIGHT
#define LCD_WIDTH       480  // logical landscape width  (used by LVGL / application)
#define LCD_HEIGHT      272  // logical landscape height (used by LVGL / application)
#define LCD_COLOR_DEPTH 16   // RGB565 — also used as data_width for esp_lcd_rgb_panel

// ==================== LCD Timing (RGB 565 Parallel @ 20 MHz - MAXIMUM SPEED) ====================
#define LCD_PCLK_HZ         (20 * 1000 * 1000)  // เพิ่มเป็น 20 MHz สูงสุด!
#define LCD_HPW             2    // HSYNC pulse width (pixels)
#define LCD_HBP             46   // horizontal back porch
#define LCD_HFP             16   // horizontal front porch
#define LCD_VPW             2    // VSYNC pulse width (lines)
#define LCD_VBP             16   // vertical back porch
#define LCD_VFP             8    // vertical front porch
#define LCD_PCLK_ACTIVE_NEG 0    // 0 = rising edge

// ==================== Backlight PWM limits ====================
#define LCD_BL_MAX_DUTY     100  // 100% duty = full brightness (maps to 10-bit full scale)

#endif // HARDWARE_CONFIG_H
