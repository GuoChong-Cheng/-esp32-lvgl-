#pragma once

#include "bsp/board_conf.h"

void pwm_setup(uint8_t pin, uint8_t channel, uint16_t freq = 1e3, uint8_t resolution = 8){
    ledcAttachPin(pin, channel);
    ledcSetup(channel, freq, resolution);
    ledcWrite(channel, 0);
}

void pwm_set_duty(uint8_t channel, uint8_t duty){
    ledcWrite(channel, duty);
}

