#pragma once

// Capacitive Touch Screen

#ifndef __CTP_DRIVER_H__
#define __CTP_DRIVER_H__

#include "lib/i2c_dev/i2c_dev.hpp"

#define CTP_DEV_ADDR 0x15

// #define CTP_INT_PIN  3

class ctp : public i2c_dev {
private:
    int16_t m_pos_x, m_pos_y;

public:
    ctp(i2c_bus* bus = &Wire) : i2c_dev(CTP_DEV_ADDR, bus) {}

    void init(void)
    {
        /* hold until init done */
        while (check()) {
            Serial.println("[CTP] please touch screen to activate it");
            delay(500);
        }
        Serial.println("[CTP] init successful");

        /* reset coordinate */
        m_pos_x = m_pos_y = -1;

#if 1
        /* shut down auto sleep */
        write_byte(0xFE, 0xFF);
        Serial.println("[CTP] auto sleep shut down");
#endif

#ifdef CTP_INT_PIN
        /* setup INT pin */
        pinMode(CTP_INT_PIN, INPUT);
        Serial.println("[CTP] set int-pin input");
        write_byte(0xFA, 0B01000001);
        write_byte(0xED, 0xC8);
#endif
    }

    void update(void)
    {
        // coor_x = read_12bit(0x03);
        // coor_y = read_12bit(0x05);

        uint8_t buff[4];

        read_block(0x03, buff, 4);

        m_pos_x = ((buff[0] & 0x0F) << 8) | buff[1];
        m_pos_y = ((buff[2] & 0x0F) << 8) | buff[3];

        // 校准
        // m_pos_x = map(m_pos_x, 7, 231, 0, 240);
        // m_pos_y = map(m_pos_y, 13, 273, 0, 280);

#if CONFIG_LCD_ROTATION == 0
        m_pos_x = 240 - m_pos_x;
        m_pos_y = 280 - m_pos_y;
#elif CONFIG_LCD_ROTATION == 90
        uint16_t pos;
        pos     = m_pos_x;
        m_pos_x = 280 - m_pos_y;
        m_pos_y = pos;
#elif CONFIG_LCD_ROTATION == 180
        // m_pos_x = m_pos_x;
        // m_pos_y = m_pos_y;
#elif CONFIG_LCD_ROTATION == 270
        uint16_t pos;
        pos     = 240 - m_pos_x;
        m_pos_x = m_pos_y;
        m_pos_y = pos;
#endif

#if 0  // DEBUG
        Serial.printf("%d,%d\r\n", m_pos_x, m_pos_y);
#endif
    }

    bool is_touched(void)
    {
#ifdef CTP_INT_PIN
        return !digitalRead(CTP_INT_PIN);
#else
        return read_8bit(0x02) ? true : false;
#endif
    }

    int16_t x(void) const { return m_pos_x; }

    int16_t y(void) const { return m_pos_y; }
};

#endif