#pragma once

#include <math.h>
#include <stdint.h>

#include "lvgl.h"
#include "lib/data_structure/linklist.hpp"

#if 1

#define Tr 30  // R波宽度(25~40ms)

class ecg_sampler {
    // original ecg (electrocardiogram) signal
    uint16_t* m_signal;

    // calculate heart rate when index value reaches overflow value
    uint16_t       m_idx;
    const uint16_t m_ovf;  // overflow

    int32_t* m_state;      // 状态缓冲

    uint8_t tophat(size_t wnd_size = Tr, uint16_t fs = 50, uint32_t threshold = 1e6)
    {
        if (m_ovf < wnd_size * 2) {
            assert(0);
            return 0;
        }

        /////////////////////////////////////// tophat 算法

        uint32_t count  = m_ovf - wnd_size * 2;
        int32_t* buffer = new int32_t[count];  // tophat 运算结果

        LinkList<uint16_t> q;                  // queue (本质上使用的是队列)

        for (int i = 0; i < m_ovf; ++i) {
            if (i >= wnd_size) {
                if (i == wnd_size) {
                    for (int j = 0; j <= wnd_size; ++j) {
                        m_state[j] = m_signal[i] - q.front()->data;
                    }
                } else {
                    m_state[i] = m_signal[i] - q.front()->data;
                }
                if (m_signal[i - wnd_size] == q.front()->data) {
                    q.pop_front();
                }
            }

            while (!q.is_empty() && q.back()->data > m_signal[i]) {
                q.pop_back();
            }

            q.push_back(m_signal[i]);
        }

        m_state[m_ovf - 1] = m_signal[m_ovf - 1] - q.front()->data;

        int max_val = -2147483648;  // -2^31, 用于顶部对齐, 方便对R波计数

        for (int i = wnd_size, j = 0; i < m_ovf - wnd_size; ++i, ++j) {
            buffer[j] = (m_state[i] - m_state[i - wnd_size]) * (m_state[i] - m_state[i + wnd_size]);

            // find max value -> use for align top peak
            if (max_val < buffer[j]) max_val = buffer[j];
        }

        /////////////////////////////////////// 心率计算

        uint16_t peak_count  = 0;
        bool     peak_search = true;

        for (int i = 0; i < count; ++i) {
            // y_i = x_i * e^( -x_i / max(x_i) )
            float v = buffer[i] * expf(-buffer[i] / (float)max_val);  // gamma 变换

            // 计R波数
            if (v >= threshold) {
                if (peak_search) {
                    peak_count += 1;
                    peak_search = false;
                }
            } else {
                peak_search = true;
            }
        }

        delete[] buffer;

        return peak_count / ((float)m_ovf / fs / 60);
    }

public:
    bool     finger_detected;
    uint8_t  _bpm;  // 心率
    uint16_t _ret;

    ecg_sampler(uint16_t ovf = 200)
        : m_idx(0), m_ovf(ovf), m_signal(new uint16_t[ovf]), m_state(new int32_t[ovf])
    {
        _bpm = _ret     = 0;
        finger_detected = true;
    }

    void reset(void) { m_idx = 0; }

    bool read(void)
    {
        if (m_ovf == m_idx) {
            m_idx = 0;
            _bpm  = tophat();
        }

        _ret = m_signal[m_idx++] = analogRead(PIN_ECG_ADC1);

        return true;
    }
};

#endif
