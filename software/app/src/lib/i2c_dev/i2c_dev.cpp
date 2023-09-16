#include "i2c_dev.hpp"

i2c_dev::i2c_dev(uint8_t dev, i2c_bus* bus)
{
    this->dev = dev;
    this->bus = bus;
}

bool i2c_dev::check()
{
    bus->beginTransmission(dev);
    return (bus->endTransmission() ? true : false);
}

void i2c_dev::write_byte(uint8_t reg, uint8_t dat)
{
    bus->beginTransmission(dev);
    bus->write(reg);
    bus->write(dat);
    bus->endTransmission();
}

uint8_t i2c_dev::read_byte(uint8_t reg) { return read_8bit(reg); }

uint16_t i2c_dev::read_word(uint8_t reg) { return read_16bit(reg); }

uint32_t i2c_dev::read_dword(uint8_t reg) { return read_32bit(reg); }

bool i2c_dev::read_byte_bit(uint8_t reg, uint8_t idx) { return (read_byte(reg) & (1 << idx)) ? true : false; }

bool i2c_dev::read_word_bit(uint8_t reg, uint8_t idx) { return (read_word(reg) & (1 << idx)) ? true : false; }

bool i2c_dev::read_dword_bit(uint8_t reg, uint8_t idx) { return (read_dword(reg) & (1 << idx)) ? true : false; }

void i2c_dev::write_byte_bit(uint8_t reg, uint8_t idx, bool val)
{
    uint8_t dat = read_byte(reg);
    if (val) {
        dat |= 1 << idx;
    } else {
        dat &= ~(1 << idx);
    }
    write_byte(reg, dat);
}

void i2c_dev::write_word_bit(uint8_t reg, uint8_t idx, bool val)
{
    uint8_t dat = read_word(reg);
    if (val) {
        dat |= 1 << idx;
    } else {
        dat &= ~(1 << idx);
    }
    write_word(reg, dat);
}

void i2c_dev::write_dword_bit(uint8_t reg, uint8_t idx, bool val)
{
    uint8_t dat = read_dword(reg);
    if (val) {
        dat |= 1 << idx;
    } else {
        dat &= ~(1 << idx);
    }
    write_dword(reg, dat);
}

uint8_t i2c_dev::read_8bit(uint8_t reg)
{
    bus->beginTransmission(dev);
    bus->write(reg);
    bus->endTransmission();
    bus->requestFrom(dev, 1);
    return bus->read();
}

uint16_t i2c_dev::read_12bit(uint8_t reg)
{
    uint8_t buff[2];
    read_block(reg, buff, 2);
    return (buff[0] << 4) + buff[1];
}

uint16_t i2c_dev::read_13bit(uint8_t reg)
{
    uint8_t buff[2];
    read_block(reg, buff, 2);
    return (buff[0] << 5) + buff[1];
}

uint16_t i2c_dev::read_16bit(uint8_t reg)
{
    uint8_t buff[2];
    read_block(reg, buff, 2);
    return (buff[0] << 8) + buff[1];
}

uint32_t i2c_dev::read_24bit(uint8_t reg)
{
    uint8_t buff[3];
    read_block(reg, buff, 3);
    return (buff[0] << 16) + (buff[1] << 8) + buff[2];
}

uint32_t i2c_dev::read_32bit(uint8_t reg)
{
    uint8_t buff[4];
    read_block(reg, buff, 4);
    return (buff[0] << 24) + (buff[1] << 16) + (buff[2] << 8) + buff[3];
}

void i2c_dev::read_block(uint8_t reg, uint8_t* buff, uint16_t len)
{
    bus->beginTransmission(dev);
    bus->write(reg);
    bus->endTransmission();
    bus->requestFrom(dev, (int)len);
    while (len--) {
        *buff++ = bus->read();
    }
}

void i2c_dev::write_word(uint8_t reg, uint16_t dat)
{
    bus->beginTransmission(dev);
    bus->write(reg);
    bus->write(dat >> 8);
    bus->write(dat & 0xFF);
    bus->endTransmission();
}

void i2c_dev::write_dword(uint8_t reg, uint16_t dat)
{
    bus->beginTransmission(dev);
    bus->write(reg);
    bus->write((dat >> 24) & 0xFF);
    bus->write((dat >> 16) & 0xFF);
    bus->write((dat >> 8) & 0xFF);
    bus->write(dat & 0xFF);
    bus->endTransmission();
}

/**
 * @brief I2C device scan
 *
 * @return int I2C device number
 */
uint8_t i2c_scan(i2c_bus* bus)
{
    uint8_t error, regess;
    uint8_t nDevices;

    Serial.println("[I2C_SCAN] device scanning...");

    nDevices = 0;
    for (regess = 1; regess < 0x7F; ++regess) {
        // The i2c_scanner uses the return value of
        // the Write.endTransmisstion to see if
        // a device did acknowledge to the regess.
        bus->beginTransmission(regess);
        error = bus->endTransmission();

        if (error == 0) {
            Serial.print("[I2C_SCAN]: device found at regess 0x");
            if (regess < 16)
                Serial.print("0");
            Serial.print(regess, HEX);
            Serial.println(" !");

            nDevices++;
        } else if (error == 4) {
            Serial.print("[I2C_SCAN]: unknow error at regess 0x");
            if (regess < 16)
                Serial.print("0");
            Serial.println(regess, HEX);
        }
    }

    Serial.print("[I2C_SCAN]:");
    Serial.printf(" %d devices was found\r\n", nDevices);
    return nDevices;
}
