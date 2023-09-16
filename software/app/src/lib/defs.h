#ifndef __DEFS_H__
#define __DEFS_H__

#include <Arduino.h>

// helper

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

// delay

#define delay_ms(ms)    delay(ms)
#define delay_us(us)

// bus

#include "Wire.h"
#include "SPI.h"

typedef TwoWire  i2c_bus;
typedef SPIClass spi_bus;

// lcd

#define CONFIG_LCD_ROTATION  270
#define CONFIG_LCD_BACKLIGHT 1  // mode: 0 GPIO, 1 PWM

#if CONFIG_LCD_ROTATION % 90
#warning "unsupported rotation angle"
#endif

#endif