#pragma once

#include "lib/defs.h"

#define I2C_SPEED_STANDARD 100000
#define I2C_SPEED_FAST     400000

class i2c_dev {
private:
    uint8_t  dev;
    i2c_bus* bus;

protected:
    i2c_dev(uint8_t dev, i2c_bus* bus = &Wire);

    bool check(void);

    void write_byte(uint8_t reg, uint8_t dat);
    void write_word(uint8_t reg, uint16_t dat);
    void write_dword(uint8_t reg, uint16_t dat);

    uint8_t  read_byte(uint8_t reg);
    uint16_t read_word(uint8_t reg);
    uint32_t read_dword(uint8_t reg);

    uint8_t  read_8bit(uint8_t reg);
    uint16_t read_12bit(uint8_t reg);
    uint16_t read_13bit(uint8_t reg);
    uint16_t read_16bit(uint8_t reg);
    uint32_t read_24bit(uint8_t reg);
    uint32_t read_32bit(uint8_t reg);
    void     read_block(uint8_t reg, uint8_t* buff, uint16_t len);

    bool read_byte_bit(uint8_t reg, uint8_t idx);
    bool read_word_bit(uint8_t reg, uint8_t idx);
    bool read_dword_bit(uint8_t reg, uint8_t idx);

    void write_byte_bit(uint8_t reg, uint8_t idx, bool val);
    void write_word_bit(uint8_t reg, uint8_t idx, bool val);
    void write_dword_bit(uint8_t reg, uint8_t idx, bool val);
};

uint8_t i2c_scan(i2c_bus* bus = &Wire);
