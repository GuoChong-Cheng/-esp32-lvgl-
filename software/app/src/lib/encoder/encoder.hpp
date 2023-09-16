#pragma once

#include "lib/defs.h"

class encoder {
public:
    typedef enum {
        INCREMENT,  // increment
        DECREMENT,  // decrement
        UNCHANGE    // unchange
    } dir_e;

public:
    int32_t m_abs;
    int32_t m_diff;
    dir_e   m_dir;

private:
    int8_t m_pin_a;
    int8_t m_pin_b;

    bool    m_last_a;
    int32_t m_count;

    // 去编码器抖动的
    bool m_first = 0;
    bool m_flag  = 0;

public:
    encoder(int8_t pin_a, int8_t pin_b)
        : m_diff(0), m_abs(0), m_count(0), m_last_a(HIGH), m_pin_a(pin_a), m_pin_b(pin_b)
    {
        pinMode(m_pin_a, INPUT_PULLUP);
        pinMode(m_pin_b, INPUT_PULLUP);
    }

    void  process(void);
    dir_e get(void);
};

#if 0

encoder ec(PIN_ENCODER_A, PIN_ENCODER_B);

void setup()
{
    Serial.begin(115200);
    while (!Serial) {}
}

void loop()
{
    ec.process();
    ec.get();
}

#endif