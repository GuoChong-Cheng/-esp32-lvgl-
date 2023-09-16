

#include "axp173.hpp"

axp173::axp173(i2c_bus* bus) : i2c_dev(AXP173_ADDR, bus)
{
}

bool axp173::init()
{
    return check();
}

/** @defgroup 输入电源状态寄存器 (addr = 0x00) */

bool axp173::is_acin_exist()
{
    return read_byte_bit(0x00, 7);
}

bool axp173::is_acin_avl()
{
    return read_byte_bit(0x00, 6);
}

bool axp173::is_vbus_exist()
{
    return read_byte_bit(0x00, 5);
}

bool axp173::is_vbus_avl()
{
    return read_byte_bit(0x00, 4);
}

bool axp173::get_bat_current_dir()
{
    return read_byte_bit(0x00, 2);
}

/** @defgroup 电源工作模式以及充电状态指示寄存器 (addr = 0x01) */

bool axp173::is_over_temp()
{
    return read_byte_bit(0x01, 7);
}

bool axp173::is_charging()
{
    return read_byte_bit(0x01, 6);
}

bool axp173::is_bat_exist()
{
    return read_byte_bit(0x01, 5);
}

bool axp173::is_charge_csmaller()
{
    return read_byte_bit(0x01, 2);
}

/** @defgroup 电源输出控制寄存 (addr = 0x12) */

void axp173::set_output_enable(axp173_output_channel_e channel, bool state)
{
    write_byte_bit(0x12, (uint8_t)(channel), state);
}

/* 电源输出电压配置寄存器
 * DC-DC2     0x23  25mV/step   7-6bit(stable)   5-0bit(usage)
 * DC-DC1     0x26  25mV/step   7bit(stable)     6-0bit(usage)
 * LDO4       0x27  25mV/step   7bit(stable)     6-0bit(usage)
 * LDO2&LDO3  0x28  100mV/step  None(stable)     7-4bit&3-0bit(usage)
 * 控制某一通道电源电压输出大小（具体见，h文件）
 *
 * (buff & 0B10000000)：将buff转为八位二进制,重置电压设置位0-6bit,"维持保留位7bit" !!!
 * (voltage & 0B01111111)：将voltage转为八位二进制,维持待写入电压位0-6bit，"重置保留位7bit" !!!
 * (buff & 0B10000000) | (voltage & 0B01111111)：最后的或运算结束后，只有0-6bit改变了，7bit还仍
 * 然维持读取时的状态，起到了不更改保留位但却更改了电压位的作用！！！
 */
void axp173::set_output_voltage(axp173_output_channel_e channel, uint16_t voltage)
{
    uint8_t buff = 0;
    switch (channel) {
        case AXP173_OUTPUT_CHANNEL_DCDC1:
            voltage = (clamp(voltage, 700, 3500) - 700) / 25;  // 0 - 112(step)
            buff    = read_8bit(0x26);
            buff    = (buff & 0B10000000) | (voltage & 0B01111111);
            write_byte(0x26, buff);
            break;
        case AXP173_OUTPUT_CHANNEL_DCDC2:
            voltage = (clamp(voltage, 700, 2275) - 700) / 25;
            buff    = read_8bit(0x23);
            buff    = (buff & 0B11000000) | (voltage & 0B00111111);
            write_byte(0x23, buff);
            break;
        case AXP173_OUTPUT_CHANNEL_LDO2:
            voltage = (clamp(voltage, 1800, 3300) - 1800) / 100;
            buff    = read_8bit(0x28);
            buff    = (buff & 0B00001111) | (voltage << 4);
            write_byte(0x28, buff);
            break;
        case AXP173_OUTPUT_CHANNEL_LDO3:
            voltage = (clamp(voltage, 1800, 3300) - 1800) / 100;
            buff    = read_8bit(0x28);
            buff    = (buff & 0B11110000) | (voltage);
            write_byte(0x28, buff);
            break;
        case AXP173_OUTPUT_CHANNEL_LDO4:
            voltage = (clamp(voltage, 700, 3500) - 700) / 25;
            buff    = read_8bit(0x27);
            buff    = (buff & 0B10000000) | (voltage & 0B01111111);
            write_byte(0x27, buff);
            break;
        default:
            break;
    }
}

// 开关芯片控制寄存器（地址：0x32）
void axp173::power_off(void)
{
    // 关闭芯片所有输出
    write_byte_bit(0x32, 7, true);
}

// 已关机则返回 false
bool axp173::power_state(void)
{
    return !read_byte_bit(0x32, 7);
}

/* 长按按键芯片开关机时间设置寄存器（地址：0x36） */
void axp173::set_power_on_time(axp173_power_on_time_e onTime)
{
    write_byte(0x36, ((read_8bit(0x36) & 0B00111111) | onTime));
}

void axp173::set_power_off_time(axp173_power_off_time_e offTime)
{
    write_byte(0x36, ((read_8bit(0x36) & 0B11111100) | offTime));
}

/* 充电控制寄存器1（地址：0x33）
 * 设置充电电流以及是否使能充电功能，充电目标电压(5 and 6 bit)默认为4.2V(1 and 0)
 */
void axp173::set_charge_enable(bool state)
{  // 充电功能使能控制位7bit
    if (state)
        write_byte(0x33, ((read_8bit(0x33) | 0B10000000)));
    else
        write_byte(0x33, ((read_8bit(0x33) & 0B01111111)));
}

void axp173::set_charge_current(axp173_charge_current_e current)
{  // 写入电流见CHARGE_CURRENT枚举体
    write_byte(0x33, ((read_8bit(0x33) & 0B11110000) | current));
}

/* ADC使能寄存器1（地址：0x82）
 */
void axp173::set_adc_enable(axp173_adc_channel_e channel, bool state)
{
    uint8_t buff = read_8bit(0x82);
    buff         = state ? (buff | (1U << channel)) : (buff & ~(1U << channel));
    write_byte(0x82, buff);
}

/* ADC使能寄存器2（地址：0x83）
 * 设置芯片温度检测ADC使能（默认开启）
 */
void axp173::set_chip_temp_enable(bool state)
{
    uint8_t buff = read_8bit(0x83);
    buff         = state ? (buff | 0B10000000) : (buff & 0B01111111);
    write_byte(0x83, buff);
}

/* 库仑计控制寄存器（地址：0xB8）
 * 设置库仑计（开关ENABLE 7bit，暂停PAUSE 6bit，清零RESET 5bit。5 and 6 bit 操作结束后会自动置零，0-4bit为保留位）
 */
void axp173::set_coulometer(axp173_coulometer_ctrl_e option, bool state)
{
    uint8_t buff = read_8bit(0xB8);
    buff         = state ? (buff | (1U << option)) : (buff & ~(1U << option));
    write_byte(0xB8, buff);
}

/* 以上为设置寄存器数据 */
/* 以下为读取寄存器数据 */

/* IRQ引脚中断寄存器（地址：0xB8）
 * 设置库仑计（开关ENABLE 7bit，暂停PAUSE 6bit，清零RESET 5bit。5 and 6 bit 操作结束后会自动置零，0-4bit为保留位）
 */

/* 库仑计数据读取寄存器（地址：0xB0、0xB4）(待理解)
 * 读取库仑计数值并且计算电池电量
 *
 * 函数写法功能解析：(2^16) * 0.5 * (int32_t)(CCR - DCR) / 3600.0 / 25.0;
 *
 * SOC = RM / FCC：荷电状态% = 剩余电容量 / 完全充电容量
 * RM = CCR - DCR：剩余电容量 = 充电计数器寄存器 - 放电计数器寄存器
 *
 * (2^16)：65536
 * 0.5：current_LSB（数据手册说库仑计精度为0.5%）
 * (int32_t)：将结果强制转换为正数
 * (CCR - DCR)：剩余电容量mAh
 * 3600.0：1mAh = 0.001A * 3600s = 3.6库仑，将剩余电容量转化为电流
 * 25.0：ADC rate (0x84寄存器 "6 and 7 bit" 默认为 "00",使得ADC速率为"25*(2^0)"Hz)
 */
inline uint32_t axp173::get_coulometer_charge_data(void)
{
    // 电池充电库仑计数据寄存器3（积分后数据）
    return read_32bit(0xB0);
}

inline uint32_t axp173::get_coulometer_discharge_data(void)
{
    // 电池放电库仑计数据寄存器3（积分后数据）
    return read_32bit(0xB4);
}

float axp173::get_bat_coulomb_input(void)
{
    return read_32bit(0xB0) * 65536 * 0.5 / 3600 / 25.0;
}

float axp173::get_bat_coulomb_output(void)
{
    return read_32bit(0xB4) * 65536 * 0.5 / 3600 / 25.0;
}

float axp173::get_coulometer_data(void)
{
    // 返回库仑计计算数据
    uint32_t coin  = get_coulometer_charge_data();
    uint32_t coout = get_coulometer_discharge_data();
    // data = 65536 * current_LSB（电池电流ADC精度为0.5mA） * (coin - coout) / 3600（单位换算） / ADC rate (0x84寄存器 "6 and 7 bit" 默认为 "00",使得ADC速率为"25*(2^0)"Hz)
    float    CCC   = 65536 * 0.5 * (int32_t)(coin - coout) / 3600.0 / 25.0;
    return CCC;
}

// 电池电压 精度：1.1mV
float axp173::get_bat_voltage()
{
    return read_12bit(0x78) * (1.1f / 1000);
}

// 电池充电电流，精度：0.5mA
float axp173::get_bat_current()
{
    uint16_t CurrentIn  = read_13bit(0x7A);
    uint16_t CurrentOut = read_13bit(0x7C);
    return (CurrentIn - CurrentOut) * 0.5f;
}

// 返回电池电量等级（%）
float axp173::get_bat_level()
{
    const float batVoltage    = get_bat_voltage();
    const float batPercentage = (batVoltage < 3.248088) ? (0) : (batVoltage - 3.120712) * 100;
    return (batPercentage <= 100) ? batPercentage : 100;
}

// 电池瞬时功率
float axp173::get_bat_power()
{
    return 1.1f * 0.5f * read_24bit(0x70) / 1000.f;
}

uint32_t axp173::get_charge_time()
{
    static uint32_t chargeTime = 0, startTime = 0;

    uint32_t nowTime = millis();

    if (is_charging()) {
        chargeTime = nowTime - startTime;
    } else {
        startTime = nowTime;
    }

    return chargeTime;
}

// USB 输入电压，精度：1.7mV
float axp173::get_vbus_voltage()
{
    return read_12bit(0x5A) * (1.7f / 1000);
}

// USB 输入电流，精度：0.375mA
float axp173::get_vbus_current()
{
    return read_12bit(0x5C) * 0.375f;
}

// 片内温度传感器温度，精度：0.1℃，最小值-144.7℃
float axp173::get_chip_temp()
{
    return -144.7f + read_12bit(0x5E) * 0.1f;
}

// TS脚热敏电阻检测到的电池温度，精度：0.1℃，最小值-144.7℃
float axp173::get_ts_temp()
{
    return -144.7f + read_12bit(0x62) * 0.1f;
}

/* 按键状态检测 */
void axp173::ao_to_power_off_enable(void)
{
    // 按键时长大于关机时长自动关机
    return write_byte_bit(0x36, 3, true);
}

void axp173::set_short_press_enabale(void)
{
    // 短按键使能REG31H[3] 调用后立刻导致短按键中断发生
    return write_byte_bit(0x31, 3, true);
}

bool axp173::get_short_press_irq_state(void)
{
    // 读取短按键IRQ中断状态
    return read_byte_bit(0x46, 1);
}

void axp173::init_irq_state(void)
{
    // 所有IRQ中断使能置零REG40H 41H 42H 43H 4AH
    write_byte(0x40, ((read_8bit(0x40) & 0B00000001)));
    write_byte(0x41, ((read_8bit(0x41) & 0B00000000)));
    write_byte(0x42, ((read_8bit(0x42) & 0B00000100)));
    write_byte(0x43, ((read_8bit(0x43) & 0B11000010)));
    write_byte(0x4A, ((read_8bit(0x4A) & 0B01111111)));
}

void axp173::set_long_press_time(axp173_long_press_time_e pressTime)
{  // 设置长按键触发时间 5 and 4 bit
    write_byte(0x36, ((read_8bit(0x36) & 0B11001111) | pressTime));
}

bool axp173::get_long_rress_irq_state(void)
{
    // 读取长按键IRQ中断状态
    return read_byte_bit(0x46, 0);
}

void axp173::init_key_press_irq(axp173_long_press_time_e pressTime)
{
    // 初始化IRQ中断使能，设置长按键触发时间
    init_irq_state();
    ao_to_power_off_enable();
    // setShortPressEnabale();
    set_long_press_time(pressTime);
}

void axp173::set_short_press_irq_disabale(void)
{
    write_byte_bit(0x46, 1, true);
}

void axp173::set_long_press_irq_disabale(void)
{
    write_byte_bit(0x46, 0, true);
}

/* 按键与睡眠 */
void axp173::prepare_to_sleep(void)
{
    // ldo断电
    // set_output_enable(AXP173_OUTPUT_CHANNEL_LDO3, false);     //LDO3关闭输出
}

void axp173::light_sleep(uint64_t time_in_us)
{
    // 类似于锁屏，需要打开REG31[3]
    // prepare_to_sleep();
}

void axp173::deep_sleep(uint64_t time_in_us)
{
    // ldo断电加MCU低功耗模式
    // prepareToSleep();

    // /* nnn */
    // set_sleep_mode (WiFiSleepType_t type, int listenInterval=0);  //WiFiSleepType_t type, int listenInterval=0

    // restore_from_light_sleep();
}

void axp173::restore_from_light_sleep(void)
{
    // ldo重启输出
    // set_output_voltage(AXP173_OUTPUT_CHANNEL_LDO3, 3300);  // LDO3电压设置为3.300V
    // set_output_enable(AXP173_OUTPUT_CHANNEL_LDO3, true);   // LDO3设置为输出
}
