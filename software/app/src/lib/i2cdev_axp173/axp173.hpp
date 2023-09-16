#pragma once

#ifndef __AXP173_H__
#define __AXP173_H__

#include "lib/i2c_dev/i2c_dev.hpp"

#define AXP173_ADDR      0x34  // 设备地址

#define AXP173_IRQ_STATE 0  // digitalRead(PIN_AXP173_IRQ)

// 可调电压输出通道（LDO1为RTC电源，电压不可调）
typedef enum {
    AXP173_OUTPUT_CHANNEL_DCDC1 = 0,
    AXP173_OUTPUT_CHANNEL_LDO4,
    AXP173_OUTPUT_CHANNEL_LDO2,
    AXP173_OUTPUT_CHANNEL_LDO3,
    AXP173_OUTPUT_CHANNEL_DCDC2,
} axp173_output_channel_e;

// ADC采集电压与电流参数
typedef enum {
    AXP173_ADC_CHANNEL_TS = 0,  // 温敏电阻管脚ADC
    AXP173_ADC_CHANNEL_APS_V,   // APS电压
    AXP173_ADC_CHANNEL_VBUS_C,  // 直流输入电流
    AXP173_ADC_CHANNEL_VBUS_V,  // 直流输入电压
    AXP173_ADC_CHANNEL_ACIN_C,  // 交流输入电流
    AXP173_ADC_CHANNEL_ACIN_V,  // 交流输入电压
    AXP173_ADC_CHANNEL_BAT_C,   // 电池输入电流
    AXP173_ADC_CHANNEL_BAT_V,   // 电池输入电压
} axp173_adc_channel_e;

// 电池充电电流设置
typedef enum {
    AXP173_CHARGE_CURRENT_100mA = 0,
    AXP173_CHARGE_CURRENT_190mA,
    AXP173_CHARGE_CURRENT_280mA,
    AXP173_CHARGE_CURRENT_360mA,
    AXP173_CHARGE_CURRENT_450mA,
    AXP173_CHARGE_CURRENT_550mA,
    AXP173_CHARGE_CURRENT_630mA,
    AXP173_CHARGE_CURRENT_700mA,
    AXP173_CHARGE_CURRENT_780mA,
    AXP173_CHARGE_CURRENT_880mA,
    AXP173_CHARGE_CURRENT_960mA,
    AXP173_CHARGE_CURRENT_1000mA,
    AXP173_CHARGE_CURRENT_1080mA,
    AXP173_CHARGE_CURRENT_1160mA,
    AXP173_CHARGE_CURRENT_1240mA,
    AXP173_CHARGE_CURRENT_1320mA,
} axp173_charge_current_e;

// 库仑计控制
typedef enum {
    AXP173_COULOMETER_RESET = 5,
    AXP173_COULOMETER_PAUSE,
    AXP173_COULOMETER_ENABLE,
} axp173_coulometer_ctrl_e;

// 关机时长设置
typedef enum {
    AXP173_POWER_OFF_TIME_4S = 0,
    AXP173_POWER_OFF_TIME_6S,
    AXP173_POWER_OFF_TIME_8S,
    AXP173_POWER_OFF_TIME_10S,
} axp173_power_off_time_e;

// 开机时长设置
typedef enum {
    AXP173_POWER_ON_128MS = 0,
    AXP173_POWER_ON_512MS,
    AXP173_POWER_ON_1S,
    AXP173_POWER_ON_2S,
} axp173_power_on_time_e;

// 长按键PEK多长时间触发开关机事件
typedef enum {
    AXP173_LPRESS_TIME_1S = 0,
    AXP173_LPRESS_TIME_1_5S,
    AXP173_LPRESS_TIME_2S,
    AXP173_LPRESS_TIME_2_5S,
} axp173_long_press_time_e;

class axp173 : public i2c_dev {
private:
    uint16_t clamp(uint16_t _val, uint16_t _min, uint16_t _max) { return max(min(_val, _max), _min); }

public:
    axp173(i2c_bus* bus = &Wire);

    bool init(void);

    /* Power input state（输入电源状态检测） */

    // 地址：0x00
    bool is_acin_exist();        // ACIN 存在指示
    bool is_acin_avl();          // ACIN 是否可用
    bool is_vbus_exist();        // VBUS 存在指示
    bool is_vbus_avl();          // VBUS 是否可用
    bool get_bat_current_dir();  // 获取电池电流方向（0：在放电；1：在充电）

    // 地址：0x01
    bool is_over_temp();        // 芯片过温
    bool is_charging();         // 充电指示（0：未充电或已充电完成；1：正在充电）
    bool is_bat_exist();        // 电池存在状态指示
    bool is_charge_csmaller();  // 指示充电电流是否小于期望电流（0：实际充电电流等于期望电流；1：实际充电电流小于期望电流）

    /* Power output control （电源输出控制）*/
    void set_output_enable(axp173_output_channel_e channel, bool state);         // channel：设置电源输出通道（OUTPUT_CHANNEL）；state：设置是否输出
    void set_output_voltage(axp173_output_channel_e channel, uint16_t voltage);  // channel：设置电源输出通道（OUTPUT_CHANNEL）；voltage：设置输出电压
                                                                                 // DCDC1 & LDO4: 700~3500(mV), DCDC2: 700~2275(mV), LDO2 & LDO3: 1800~3300(mV)

    /* Basic control, 开关芯片控制 */
    void power_off(void);                                      // 关机
    bool power_state(void);                                    // 电源状态
    void set_power_off_time(axp173_power_off_time_e offTime);  // 关机时间
    void set_power_on_time(axp173_power_on_time_e onTime);     // 开机时间

    /* Charge control, 电池充电设置 */
    void set_charge_enable(bool state);                        // 充电功能使能控制位7bit（上电默认开启）
    void set_charge_current(axp173_charge_current_e current);  // 充电电流设置0-3bit

    /* ADC control (ADC设置) */
    void set_adc_enable(axp173_adc_channel_e channel, bool state);  // ADC使能1 channel：设置ADC使能通道 参数见ADC_CHANNEL枚举体 state:设置是否输出
    void set_chip_temp_enable(bool state);                          // ADC使能2 设置芯片温度检测ADC使能 state:设置是否输出 默认输出

    /* Coulometer control (库仑计模式设置) */
    void set_coulometer(axp173_coulometer_ctrl_e option, bool state);  // 设置库仑计状态（开关ENABLE，暂停PAUSE，清零RESET）

    /* Coulometer data (库仑计数据) */
    uint32_t get_coulometer_charge_data(void);     // 电池充电库仑计数据寄存器3
    uint32_t get_coulometer_discharge_data(void);  // 电池放电库仑计数据寄存器3
    float    get_bat_coulomb_input(void);          // 库仑计输入计数
    float    get_bat_coulomb_output(void);         // 库仑计输出计数
    float    get_coulometer_data(void);            // 计算后返回的值 get coulomb val affter calculation

    /* BAT data (电池状态数据) */
    float    get_bat_voltage();  // 电池电压
    float    get_bat_current();  // 电池电流
    float    get_bat_level();    // 电池电量等级（%）
    float    get_bat_power();    // 电池瞬时功率
    uint32_t get_charge_time();  // ms

    /* VBUS data (外部输入电压状态数据) */
    float get_vbus_voltage();  // USB 输入电压
    float get_vbus_current();  // USB 输入电流

    /* Temperature data (温度监控数据) */
    float get_chip_temp();  // 内置温度传感器温度
    float get_ts_temp();    // TS脚热敏电阻检测到的电池温度

    /* Read IRQ enable and state REG get PEK Long and Short Press state(读取与操作IRQ使能与状态寄存器获取长按键与短按键状态) */

    void ao_to_power_off_enable(void);                             // 按键时长大于关机时长自动关机使能
    void set_short_press_enabale(void);                            // 短按键使能REG31H[3] 调用后立刻导致短按键中断发生
    bool get_short_press_irq_state(void);                          // 读取短按键IRQ中断状态
    void init_irq_state(void);                                     // 所有IRQ中断使能置零 REG40H 41H 42H 43H 4AH
    void set_long_press_time(axp173_long_press_time_e pressTime);  // 设置长按键触发时间
    bool get_long_rress_irq_state(void);                           // 读取长按键IRQ中断状态
    void init_key_press_irq(axp173_long_press_time_e pressTime);   // 初始化IRQ中断使能，设置长按键触发时间
    void set_short_press_irq_disabale();                           // 对应位写1结束中断
    void set_long_press_irq_disabale();                            // 对应位写1结束中断

    void prepare_to_sleep(void);
    void light_sleep(uint64_t time_in_us);
    void deep_sleep(uint64_t time_in_us);
    void restore_from_light_sleep(void);
};

#endif

#if 0

#include "bsp/board.h"
#include <SimpleSerialShell.h>
#include "lib/i2cdev_axp173/axp173.hpp"

axp173 pmu;

uint8_t pmu_output = 3;  // bit0: bus; bit1: bat

void pmu_key_ecb()
{
    pmu.init_key_press_irq(AXP173_LPRESS_TIME_2_5S);  // 开启

    if (pmu.power_state()) {
        if (pmu.get_short_press_irq_state()) {
            Serial.println("PEK Short Press");
            pmu.set_short_press_irq_disabale();  // 清除
#if 0
            static bool LOCK_STATE = 1;
            if (LOCK_STATE) {
                pmu.prepare_to_sleep();
                Serial.println("Screen Lock");
            } else {  // = 0 唤醒
                pmu.restore_from_light_sleep();
                Serial.println("Screen UnLock");
            }
#endif
        } else if (pmu.get_long_rress_irq_state()) {
            Serial.println("PEK Long Press");
            pmu.set_long_press_irq_disabale();  // 清除
#if 0
            Serial.println("Shut Down");
            delay_ms(1000);
            pmu.power_off();  // or deepsleep
#endif
        }
    }
}

int pmu_ctrl(int argc, char** argv)
{
    if (argc == 2) {
        int32_t ctl = atoi(argv[1]);
        pmu_output  = ctl;
    }
    return EXIT_SUCCESS;
}

void pmu_init()
{
    pmu.attach(&Wire);

    // 开关机
    pmu.set_power_off_time(AXP173_POWER_OFF_TIME_8S);
    pmu.set_power_on_time(AXP173_POWER_ON_2S);

    // VBUS
    pmu.set_adc_enable(AXP173_ADC_CHANNEL_VBUS_V, true);
    pmu.set_adc_enable(AXP173_ADC_CHANNEL_VBUS_C, true);

    // 电池
    pmu.set_adc_enable(AXP173_ADC_CHANNEL_BAT_V, true);
    pmu.set_adc_enable(AXP173_ADC_CHANNEL_BAT_C, true);

    // 库仑计
    pmu.set_coulometer(AXP173_COULOMETER_ENABLE, true);

    // 充电
    pmu.set_charge_enable(true);
    pmu.set_charge_current(AXP173_CHARGE_CURRENT_550mA);  // 电流
}

void pmu_loop(uint32_t dt)
{
    static uint32_t last_time = 0;

    if ((millis() - last_time) > dt) {
        if (pmu_output & 0x02) {
            if (pmu.is_bat_exist()) {
                Serial.printf("bat [ charge = %d ( %.2f s), dir = %d ] : v: %.2f V, i: %.2f mA, p: %.2f mW, lvl: %.2f%%",
                              pmu.is_charging(),
                              pmu.get_charge_time() / 1000.f,
                              pmu.get_bat_current_dir(),
                              pmu.get_bat_voltage(),
                              pmu.get_bat_current(),
                              pmu.get_bat_power(),
                              pmu.get_bat_level());
                Serial.println("");

                Serial.printf("coulometer: in %.2fC, out %.2fC, %.2fC",
                              pmu.get_bat_coulomb_input(),
                              pmu.get_bat_coulomb_output(),
                              pmu.get_coulometer_data());
                Serial.println("");
            } else {
                Serial.printf("no battery");
                Serial.println("");
            }
        }

        if (pmu_output & 0x01) {
            // first_order_lowpass_filter
            static float last = 0, coeff = 0.15, val;
            val  = pmu.get_vbus_voltage() * pmu.get_vbus_current();
            last = coeff * val + (1 - coeff) * last;

            Serial.printf("bus : v: %.2f V, i: %.2f mA, p: %.2f mW",
                          pmu.get_vbus_voltage(),
                          pmu.get_vbus_current(), last);
            Serial.println("");
        }

        last_time = millis();
    }
}

void setup()
{
    Serial.begin(115200);
    while (!Serial) {}

    Wire.begin(PIN_PMU_SDA, PIN_PMU_SCL);

    shell.attach(Serial);
    shell.addCommand(F("pmu"), pmu_ctrl);
    pmu_init();
}

void loop()
{
    shell.executeIfInput();

    pmu_loop(200);
    pmu_key_ecb();
}

#endif