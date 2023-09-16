

#pragma once

#include "lib/defs.h"

// info

#define PROJECT               "uYanki-Core"
#define VERISON               "v0.0.2"

// wifi

#define CONFIG_WIFI_SSID      ""
#define CONFIG_WIFI_PWD       ""

// lcd

#define CONFIG_LCD_WIDTH      240
#define CONFIG_LCD_HEIGHT     280
#define CONFIG_LCD_CACHE_SIZE (CONFIG_LCD_HEIGHT * 8)
#define CONFIG_LCD_XOFFSET    0
#define CONFIG_LCD_YOFFSET    20

// lvgl

#define CONFIG_LVGL_USE_PSRAM 1
#define CONFIG_LVGL_USE_DEMOS 0

/* pins */

#define PIN_I2C_SCL           12
#define PIN_I2C_SDA           11

// lcd
#define PIN_LCD_BL            48
#define PIN_LCD_DC            21
#define PIN_LCD_SCL           14
#define PIN_LCD_SDA           13
#define PIN_LCD_RST           10

// buzzer
#define PIN_BUZZER            46

// axp173
#define PIN_PMU_SDA           PIN_I2C_SDA
#define PIN_PMU_SCL           PIN_I2C_SCL

// ctp
#define PIN_CTP_SDA           PIN_I2C_SDA
#define PIN_CTP_SCL           PIN_I2C_SCL
#define PIN_CTP_RST           PIN_LCD_RST
#define PIN_CTP_INT           3

// sdcard
#define PIN_SD_MISO           16
#define PIN_SD_MOSI           6
#define PIN_SD_SCLK           15
#define PIN_SD_NSS            5
#define PIN_SD_DET            17

// encoder
#define PIN_ENCODER_A         4
#define PIN_ENCODER_B         5
#define PIN_ENCODER_C         15  // btn

// max30102
#define PIN_PPG_INT           1
#define PIN_PPG_SDA           PIN_I2C_SDA
#define PIN_PPG_SCL           PIN_I2C_SCL

// mpu6050
#define PIN_IMU_INT           0
#define PIN_IMU_SDA           PIN_I2C_SDA
#define PIN_IMU_SCL           PIN_I2C_SCL

// ks1082
#define PIN_ECG_ADC1          6
#define PIN_ECG_ADC2          7
#define PIN_ECG_MOSI          2
#define PIN_ECG_MISO          42
#define PIN_ECG_SCLk          41
#define PIN_ECG_NSS           39
