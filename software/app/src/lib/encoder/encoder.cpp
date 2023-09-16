

#include "encoder.hpp"

void encoder::process(void)
{
    bool now_a = digitalRead(m_pin_a);
    bool now_b = digitalRead(m_pin_b);

    if (m_first) {
        // A 下降沿
        if (!now_a) {
            m_flag  = now_b;
            m_first = false;
        }
    } else {
        // A 上升沿
        if (now_a) {
            if (!now_b && m_flag) {
                --m_count;
            } else if (now_b && !m_flag) {
                ++m_count;
            }
            m_first = true;
#if 0
            Serial.printf("%d,%d", now_b, m_flag);
            Serial.println("");
#endif
        }
    }
}

encoder::dir_e encoder::get(void)
{
    m_diff = m_count - m_abs;
    m_abs  = m_count;

    if (m_diff < 0) {
        m_dir = DECREMENT;
    } else if (m_diff > 0) {
        m_dir = INCREMENT;
    } else {
        m_dir = UNCHANGE;
    }

#if 0
        if (m_dir != UNCHANGE) {
            Serial.printf("%d,%d", m_diff, m_abs);
            Serial.println("");
        }
#endif

    return m_dir;
}