#pragma once

#include "lib/defs.h"


#define RED                 0xf800
#define GREEN               0x07e0
#define BLUE                0x001f
#define BLACK               0x0000
#define WHITE               0xffff
#define GRAY                0x8c51
#define YELLOW              0xFFE0
#define CYAN                0x07FF
#define PURPLE              0xF81F

class st7789 {
private:
    int8_t m_pin_dc, m_pin_bl;

    int16_t m_offsetx, m_offsety;

    spi_bus* m_bus;

    uint8_t* m_buffer;
    uint32_t m_bufsize;

    void write_cmd(uint8_t cmd)
    {
        digitalWrite(m_pin_dc, 0);
        m_bus->transfer(cmd);
        digitalWrite(m_pin_dc, 1);
    }

    void write_data(uint8_t dat) { m_bus->transfer(dat); }

    void write_block_data(uint8_t* dat, uint16_t len) { m_bus->transfer(dat, len); }

public:
    st7789(uint16_t bufsize, int16_t offsetx = 0, int16_t offsety = 0);

    ~st7789() { delete[] m_buffer; }

    void set_dir(uint8_t dir)
    {
        write_cmd(0x36);  // Memory Data Access Control
        write_data(dir);
    }

    void init(spi_bus* bus, int8_t pin_dc, int8_t pin_rst = -1, int8_t pin_bl = -1);

    void write_color(uint16_t color, uint32_t len);

    void write_colors(uint16_t* colors, uint32_t len);

    void set_window(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

    void display(bool enable) { write_cmd(enable ? 0x29 : 0x28); }

#if CONFIG_LCD_BACKLIGHT
    void backlight(uint8_t duty)
    {
        uint8_t ch = 0;
        ledcWrite(ch, duty);
    }
#else
    void backlight(bool enable)
    {
        if (m_pin_bl >= 0) {
            digitalWrite(m_pin_bl, enable);
        }
    }
#endif

    void invert(bool enable)
    {
        write_cmd(enable ? 0x21 : 0x20);
    }
};
