#include "st7789.hpp"

st7789::st7789(uint16_t bufsize, int16_t offsetx, int16_t offsety)
{
    m_offsetx = offsetx;
    m_offsety = offsety;

    m_bufsize = bufsize * 2;
    m_buffer  = new uint8_t[m_bufsize];
}

void st7789::init(spi_bus* bus, int8_t pin_dc, int8_t pin_rst, int8_t pin_bl)
{
    m_bus    = bus;
    m_pin_dc = pin_dc;
    m_pin_bl = pin_bl;

    bus->setDataMode(SPI_MODE2);  // SPI_MODE2 or SPI_MODE3
    bus->setBitOrder(SPI_MSBFIRST);

    pinMode(pin_dc, OUTPUT);

    if (pin_bl >= 0) {
#if CONFIG_LCD_BACKLIGHT
        uint8_t channel = 0;
        ledcAttachPin(pin_bl, channel);
        ledcSetup(channel, 1e3, 8);
        ledcWrite(channel, 1);
#else
        pinMode(pin_bl, OUTPUT);
        digitalWrite(pin_bl, 0);
#endif
    }

    if (pin_rst >= 0) {
        pinMode(pin_rst, OUTPUT);
        // reset
        digitalWrite(pin_rst, 1);
        delay_ms(50);
        digitalWrite(pin_rst, 0);
        delay_ms(50);
        digitalWrite(pin_rst, 1);
        delay_ms(50);
    }

    write_cmd(0x01);  // Software Reset
    delay_ms(150);

    write_cmd(0x11);  // Sleep Out
    delay_ms(255);

    write_cmd(0x3A);  // Interface Pixel Format
    write_data(0x55);
    delay_ms(10);

    write_cmd(0x36);  // Memory Data Access Control
#if CONFIG_LCD_ROTATION == 0
    write_data(0xC0);
#elif CONFIG_LCD_ROTATION == 90
    write_data(0xA0);
#elif CONFIG_LCD_ROTATION == 180
    write_data(0x00);
#elif CONFIG_LCD_ROTATION == 270
    write_data(0x60);
#endif

#if CONFIG_LCD_ROTATION == 90 || CONFIG_LCD_ROTATION == 270
    int16_t offset;
    offset    = m_offsetx;
    m_offsetx = m_offsety;
    m_offsety = offset;
#endif

    write_cmd(0x2A);  // Column Address Set
    write_data(0x00);
    write_data(0x00);
    write_data(0x00);
    write_data(0xF0);

    write_cmd(0x2B);  // Row Address Set
    write_data(0x00);
    write_data(0x00);
    write_data(0x00);
    write_data(0xF0);

    write_cmd(0x21);  // Display Inversion On
    delay_ms(10);

    write_cmd(0x13);  // Normal Display Mode On
    delay_ms(10);

    write_cmd(0x29);  // Display ON
    delay_ms(255);

#if CONFIG_LCD_BACKLIGHT
    backlight(10);
#else
    backlight(true);
#endif
}

void st7789::set_window(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    uint8_t buff[4];

    x1 += m_offsetx;
    x2 += m_offsetx;
    y1 += m_offsety;
    y2 += m_offsety;

    write_cmd(0x2A);  // set column(x) address

    buff[0] = x1 >> 8;
    buff[1] = x1 & 0xFF;
    buff[2] = x2 >> 8;
    buff[3] = x2 & 0xFF;
    write_block_data(buff, 4);

    write_cmd(0x2B);  // set Page(y) address

    buff[0] = y1 >> 8;
    buff[1] = y1 & 0xFF;
    buff[2] = y2 >> 8;
    buff[3] = y2 & 0xFF;
    write_block_data(buff, 4);

    write_cmd(0x2C);  // Memory Write
}

void st7789::write_color(uint16_t color, uint32_t len)
{
    uint8_t* src = (uint8_t*)&color;  // src
    uint8_t* dst = m_buffer;          // dst
    uint32_t cnt = min(len *= 2, m_bufsize);
    while (cnt) {
        *dst++ = src[1];
        *dst++ = src[0];
        cnt -= 2;
    }
    while (len > m_bufsize) {
        write_block_data(m_buffer, m_bufsize);
        len -= m_bufsize;
    }
    write_block_data(m_buffer, len);
}

void st7789::write_colors(uint16_t* colors, uint32_t len)
{
    uint8_t* src = (uint8_t*)colors;
    uint8_t* dst;

    uint32_t i, cnt;

    len *= 2;
    while (cnt = min(len, m_bufsize)) {
        dst = m_buffer;
        for (i = 0; i < cnt; i += 2) {
            *dst++ = src[1];
            *dst++ = src[0];
            src += 2;
        }
        write_block_data(m_buffer, cnt);
        len -= cnt;
    }
}
