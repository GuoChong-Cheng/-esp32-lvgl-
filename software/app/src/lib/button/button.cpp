#include "button.hpp"

bool button::read()
{
    // ignore pin changes until after this delay time
    if (m_time > millis()) {
        // ignore any changes during this period
    }
    // pin has changed
    else if (digitalRead(m_pin) != m_state) {
        m_time    = millis() + m_interval;
        m_state   = !m_state;
        m_changed = true;
    }

    return m_state;
}