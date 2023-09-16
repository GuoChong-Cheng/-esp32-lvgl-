#pragma once

#include "bsp\board_conf.h"

#include "lib/i2c_dev/i2c_dev.hpp"

#include "math.h"

/**
 * @brief High Pass Filter
 */
class HighPassFilter {
    const float kX;
    const float kA0;
    const float kA1;
    const float kB1;
    float       last_filter_value_;
    float       last_raw_value_;

public:
    /**
     * @brief Initialize the High Pass Filter
     * @param samples Number of samples until decay to 36.8 %
     * @remark Sample number is an RC time-constant equivalent
     */
    HighPassFilter(float samples) : kX(exp(-1 / samples)),
                                    kA0((1 + kX) / 2),
                                    kA1(-kA0),
                                    kB1(kX),
                                    last_filter_value_(NAN),
                                    last_raw_value_(NAN) {}

    /**
     * @brief Initialize the High Pass Filter
     * @param cutoff Cutoff frequency
     * @pram sampling_frequency Sampling frequency
     */
    HighPassFilter(float cutoff, float sampling_frequency) : HighPassFilter(sampling_frequency / (cutoff * 2 * M_PI)) {}

    /**
     * @brief Applies the high pass filter
     */
    float process(float value)
    {
        if (isnan(last_filter_value_) || isnan(last_raw_value_)) {
            last_filter_value_ = 0.0;
        } else {
            last_filter_value_ =
                kA0 * value + kA1 * last_raw_value_ + kB1 * last_filter_value_;
        }

        last_raw_value_ = value;
        return last_filter_value_;
    }

    /**
     * @brief Resets the stored values
     */
    void reset()
    {
        last_raw_value_    = NAN;
        last_filter_value_ = NAN;
    }
};

/**
 * @brief Low Pass Filter
 */
class LowPassFilter {
    const float kX;
    const float kA0;
    const float kB1;
    float       last_value_;

public:
    /**
     * @brief Initialize the Low Pass Filter
     * @param samples Number of samples until decay to 36.8 %
     * @remark Sample number is an RC time-constant equivalent
     */
    LowPassFilter(float samples) : kX(exp(-1 / samples)),
                                   kA0(1 - kX),
                                   kB1(kX),
                                   last_value_(NAN) {}

    /**
     * @brief Initialize the Low Pass Filter
     * @param cutoff Cutoff frequency
     * @pram sampling_frequency Sampling frequency
     */
    LowPassFilter(float cutoff, float sampling_frequency) : LowPassFilter(sampling_frequency / (cutoff * 2 * M_PI)) {}

    /**
     * @brief Applies the low pass filter
     */
    float process(float value)
    {
        if (isnan(last_value_)) {
            last_value_ = value;
        } else {
            last_value_ = kA0 * value + kB1 * last_value_;
        }
        return last_value_;
    }

    /**
     * @brief Resets the stored values
     */
    void reset()
    {
        last_value_ = NAN;
    }
};

/**
 * @brief Differentiator
 */
class Differentiator {
    const float kSamplingFrequency;
    float       last_value_;

public:
    /**
     * @brief Initializes the differentiator
     */
    Differentiator(float sampling_frequency) : kSamplingFrequency(sampling_frequency),
                                               last_value_(NAN) {}

    /**
     * @brief Applies the differentiator
     */
    float process(float value)
    {
        float diff  = (value - last_value_) * kSamplingFrequency;
        last_value_ = value;
        return diff;
    }

    /**
     * @brief Resets the stored values
     */
    void reset()
    {
        last_value_ = NAN;
    }
};

/**
 * @brief MovingAverageFilter
 * @tparam buffer_size Number of samples to average over
 */
template <int kBufferSize>
class MovingAverageFilter {
    int   index_;
    int   count_;
    float values_[kBufferSize];

public:
    /**
     * @brief Initalize moving average filter
     */
    MovingAverageFilter() : index_(0),
                            count_(0) {}

    /**
     * @brief Applies the moving average filter
     */
    float process(float value)
    {
        // Add value
        values_[index_] = value;

        // Increase index and count
        index_ = (index_ + 1) % kBufferSize;
        if (count_ < kBufferSize) {
            count_++;
        }

        // Calculate sum
        float sum = 0.0;
        for (int i = 0; i < count_; i++) {
            sum += values_[i];
        }

        // Calculate average
        return sum / count_;
    }

    /**
     * @brief Resets the stored values
     */
    void reset()
    {
        index_ = 0;
        count_ = 0;
    }

    /**
     * @brief Get number of samples
     * @return Number of stored samples
     */
    int count() const
    {
        return count_;
    }
};

class bpm_det {
    // protected:
public:
    // Sensor (adjust to your sensor type)
    float kSamplingFrequency = 50.0;

    // Finger Detection Threshold and Cooldown
    unsigned long kFingerThreshold  = 10000;
    unsigned int  kFingerCooldownMs = 500;

    // Edge Detection Threshold (decrease for MAX30100)
    float kEdgeThreshold = -2000.0;

    // Filters
    float kLowPassCutoff  = 5.0;
    float kHighPassCutoff = 0.5;

    // Averaging
    bool             kEnableAveraging  = true;
    static const int kAveragingSamples = 50;
    int              kSampleThreshold  = 5;

    // Filter Instances
    HighPassFilter high_pass_filter;
    LowPassFilter  low_pass_filter;
    Differentiator differentiator;

    MovingAverageFilter<kAveragingSamples> averager;

    // Timestamp of the last heartbeat
    long last_heartbeat = 0;

    // Timestamp for finger detection
    long finger_timestamp = 0;
    bool finger_detected  = false;

    // Last diff to detect zero crossing
    float last_diff    = NAN;
    bool  crossed      = false;
    long  crossed_time = 0;

public:
    bpm_det()
        : high_pass_filter(kHighPassCutoff, kSamplingFrequency),
          low_pass_filter(kLowPassCutoff, kSamplingFrequency),
          differentiator(kSamplingFrequency)
    {
    }

    float current_diff;

    bool process(float new_value)
    {
        float current_value = new_value;

        // Detect Finger using raw sensor value
        if (current_value > kFingerThreshold) {
            if (millis() - finger_timestamp > kFingerCooldownMs) {
                finger_detected = true;
            }
        } else {
            // Reset values if the finger is removed
            differentiator.reset();
            averager.reset();
            low_pass_filter.reset();
            high_pass_filter.reset();

            finger_detected  = false;
            finger_timestamp = millis();
        }

        if (finger_detected) {
            current_value = low_pass_filter.process(current_value);
            current_value = high_pass_filter.process(current_value);
            current_diff  = differentiator.process(current_value);

            _ret = current_diff;

            // Valid values?
            if (!isnan(current_diff) && !isnan(last_diff)) {
                // Detect Heartbeat - Zero-Crossing
                if (last_diff > 0 && current_diff < 0) {
                    crossed      = true;
                    crossed_time = millis();
                }

                if (current_diff > 0) {
                    crossed = false;
                }

                // Detect Heartbeat - Falling Edge Threshold
                if (crossed && current_diff < kEdgeThreshold) {
                    if (last_heartbeat != 0 && crossed_time - last_heartbeat > 300) {
                        // Show Results
                        int bpm = 60000 / (crossed_time - last_heartbeat);
                        if (bpm > 50 && bpm < 250) {
                            // Average?

                            _average_bpm = averager.process(bpm);
                            _bpm         = bpm;

                            return true;

                            // if (kEnableAveraging) {
                            //     int average_bpm = averager.process(bpm);

                            //     // Show if enough samples have been collected
                            //     if (averager.count() > kSampleThreshold) {
                            //         Serial.print("Heart Rate (avg, bpm): ");
                            //         Serial.println(average_bpm);
                            //     }
                            // } else {
                            //     Serial.print("Heart Rate (current, bpm): ");
                            //     Serial.println(bpm);
                            // }
                        }
                    }

                    crossed        = false;
                    last_heartbeat = crossed_time;
                }
            }

            last_diff = current_diff;
        }

        return false;
    }

    int _average_bpm, _bpm, _ret;
};

class max30102 : protected i2c_dev, public bpm_det {
private:
    enum {
        // Status
        REG_INTR_STATUS_1 = 0x00,  // interrupt status 1
        REG_INTR_STATUS_2 = 0x01,  // interrupt status 2
        REG_INTR_ENABLE_1 = 0x02,  // interrupt enable 1
        REG_INTR_ENABLE_2 = 0x03,  // interrupt enable 2

        // FIFO
        REG_FIFO_WR_PTR = 0x04,  // fifo write pointer
        REG_OVF_COUNTER = 0x05,  // overflow counter
        REG_FIFO_RD_PTR = 0x06,  // fifo read pointer
        REG_FIFO_DATA   = 0x07,  // fifo data

        // Configuration
        REG_FIFO_CONFIG = 0x08,  // fifo config
        REG_MODE_CONFIG = 0x09,  // mode config
        REG_SPO2_CONFIG = 0x0A,  // spo2 config

        // PA = Pulse Amplitude
        REG_LED1_PA         = 0x0C,  // red led pulse amplitude
        REG_LED2_PA         = 0x0D,  // iR led pulse amplitude
        REG_LED3_PA         = 0x0E,  // green led pulse amplitude (max30101)
        REG_PILOT_PA        = 0x10,  // proximity led pulse amplitude
        REG_MULTI_LED_CTRL1 = 0x11,
        REG_MULTI_LED_CTRL2 = 0x12,

        // Die Temperature 芯片温度
        REG_TEMP_INTR   = 0x1F,  // die temperature integer
        REG_TEMP_FRAC   = 0x20,  // die temperature fraction
        REG_TEMP_CONFIG = 0x21,

        // Proximity 接近传感器中断阈值
        REG_PROX_INT_THRESH = 0x30,

        // Part ID
        REG_REV_ID  = 0xFE,  // revision id
        REG_PART_ID = 0xFF,  // part id = 0x15
    };

    typedef enum {
        // REG_INTR_STATUS_1:
        INTERRUPT_STATUS_A_FULL       = 7,  // fifo almost full
        INTERRUPT_STATUS_PPG_RDY      = 6,  // new fifo data ready
        INTERRUPT_STATUS_ALC_OVF      = 5,  // ambient light cancellation overflow
        INTERRUPT_STATUS_PROX_INT     = 4,  // proximity threshold triggered 接近传感器
        INTERRUPT_STATUS_PWR_RDY      = 0,  // power ready
        // REG_INTR_STATUS_2:
        INTERRUPT_STATUS_DIE_TEMP_RDY = 1,  // internal temperature ready
    } interrupt_status_t;

    ///////////////// filter

public:
    uint32_t red, ir;

    max30102(uint8_t dev = 0x57, i2c_bus* bus = &Wire)
        : i2c_dev(dev, bus), bpm_det()
    {
    }

    void init(void)
    {
        write_byte(REG_MODE_CONFIG, 0x40);    // Reset
        write_byte(REG_INTR_ENABLE_1, 0xc0);  // INTR1 setting
        write_byte(REG_INTR_ENABLE_2, 0x00);  // INTR2 setting
        write_byte(REG_FIFO_WR_PTR, 0x00);    // FIFO_WR_PTR[4:0]
        write_byte(REG_OVF_COUNTER, 0x00);    // OVF_COUNTER[4:0]
        write_byte(REG_FIFO_RD_PTR, 0x00);    // FIFO_RD_PTR[4:0]
        write_byte(REG_FIFO_CONFIG, 0x0f);    // sample avg = 1, fifo rollover=false, fifo almost full = 17
        write_byte(REG_MODE_CONFIG, 0x03);    // 0x02 for Red only, 0x03 for SpO2 mode 0x07 multimode LED
        write_byte(REG_SPO2_CONFIG, 0x27);    // SPO2_ADC range = 4096nA, SPO2 sample rate (100 Hz), LED pulseWidth (400uS)
        write_byte(REG_LED1_PA, 0x24);        // Choose value for ~ 7mA for LED1
        write_byte(REG_LED2_PA, 0x24);        // Choose value for ~ 7mA for LED2
        write_byte(REG_PILOT_PA, 0x7f);       // Choose value for ~ 25mA for Pilot LED
    }

    bool read(void)
    {
#if 0
        // check interrupt flag
        if (!read_byte_bit(REG_INTR_STATUS_1, (uint8_t)INTERRUPT_STATUS_A_FULL)) return false;
#endif
        uint8_t data_fifo[6];
        read_block(REG_FIFO_DATA, data_fifo, 6);
        red = ((data_fifo[0] & 0x03) << 16) | (data_fifo[1] << 8) | (data_fifo[2]);  // Mask MSB [23:18], 0x03FFFF
        ir  = ((data_fifo[3] & 0x03) << 16) | (data_fifo[4] << 8) | (data_fifo[5]);
#if 0
        shell.printf("%i,%i", red, ir);
        shell.println();
#endif

        bpm_det::process(red);

        return true;
    }
};

// class ecg_sampler {
//     HighPassFilter high_pass_filter;

//     float kSamplingFrequency = 50.0;
//     float kHighPassCutoff    = 0.5;

// public:
//     int  _average_bpm, _bpm, _ret;
//     bool finger_detected;

//     ecg_sampler()
//         : high_pass_filter(kHighPassCutoff, kSamplingFrequency)
//     {
//         _average_bpm = _bpm = _ret = 0;
//         finger_detected            = true;
//     }

//     bool read(void)
//     {
//         _ret = analogRead(PIN_ECG_ADC1);
//         return true;
//     }
// };