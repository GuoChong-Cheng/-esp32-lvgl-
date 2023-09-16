#ifndef __BOARD_H__
#define __BOARD_H__

#include "bsp/board_conf.h"

#include <SimpleSerialShell.h>

/* lvgl */
#include "lvgl.h"
#include "bsp/lv_port/lv_port_disp.h"
#include "bsp/lv_port/lv_port_indev.h"

/* bsp */

#include "bsp/sdcard/sd.hpp"

#include "lib/buzzer/buzzer.hpp"
#include "lib/encoder/encoder.hpp"
#include "lib/button/button.hpp"

#include "lib/spidev_ks1082/ks1082.hpp"
#include "lib/i2cdev_mpu6050/mpu6050.hpp"
#include "lib/i2cdev_max30102/max30102.hpp"
#include "lib/i2cdev_axp173/axp173.hpp"

#include "lib/spidev_st7789/st7789.hpp"
#include "lib/i2cdev_ctp/p169h002.hpp"

void board_init(void );
void board_loop(void);

// bsp

extern ctp      inst_ctp;
extern st7789   inst_lcd;
extern axp173   inst_pmu;
extern mpu6050  inst_imu;
extern encoder  inst_ec;
extern button   inst_btn;
extern max30102 inst_ppg;

extern buzzer inst_beep;


#endif