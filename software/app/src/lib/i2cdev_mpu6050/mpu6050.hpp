#pragma once

#include "lib/i2c_dev/i2c_dev.hpp"

class mpu6050 : protected i2c_dev {
public:
    float ax, ay, az;
    float gx, gy, gz;
    float t;

    float pitch /* -90~90 */, roll /* -180~180 */, yaw;

    mpu6050(uint8_t dev = 0x68,i2c_bus* bus = &Wire) :i2c_dev(dev,bus){}

    void init(void);

    void update(void);
};
