#pragma once

#include "lib/defs.h"

class button {
public:
    button(uint8_t pin, uint16_t debounce_ms = 100)
        : m_pin(pin), m_interval(debounce_ms), m_state(HIGH), m_time(0), m_changed(false)
    {
        pinMode(pin, INPUT_PULLUP);
        read();
    }

    bool read(void);

    // has the button been toggled from on -> off, or vice versa
    bool toggled(void) { return read(), has_changed(); }

    // has the button gone from off -> on
    bool pressed(void) { return (read() == PRESSED && has_changed()); }

    // has the button gone from on -> off
    bool released(void) { return (read() == RELEASED && has_changed()); }

    // mostly internal, tells you if a button has changed after calling the read() function
    bool has_changed(void)
    {
        if (m_changed) {
            m_changed = false;
            return true;
        }
        return false;
    }

    const static bool PRESSED  = LOW;
    const static bool RELEASED = HIGH;

private:
    uint8_t m_pin;

    bool m_state;
    bool m_changed;

    uint16_t m_interval;
    uint32_t m_time;  // ms
};

#if 0

#define PIN_BTN_1 40
#define PIN_BTN_2 38
#define PIN_BTN_3 36

button button1(PIN_BTN_1);
button button2(PIN_BTN_2);
button button3(PIN_BTN_3);

void loop()
{
    if (button1.pressed()) {
        Serial.println("button 1 pressed");
    }
    if (button2.released()) {
        Serial.println("button 2 released");
    }
    if (button3.toggled()) {
        if (button3.read() == button::PRESSED) {
            Serial.println("button 3 has been pressed");
        } else {
            Serial.println("button 3 has been released");
        }
    }
}

#endif