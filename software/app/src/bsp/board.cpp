#include "board.h"

int cmd_echo(int argc, char** argv);
int cmd_info(int argc, char** argv);
int cmd_itoctl(int argc, char** argv);
int cmd_scan(int argc, char** argv);
int cmd_wifi(int argc, char** argv);  // connect wifi

ctp    inst_ctp;
st7789 inst_lcd(CONFIG_LCD_CACHE_SIZE, CONFIG_LCD_XOFFSET, CONFIG_LCD_YOFFSET);

axp173   inst_pmu;
mpu6050  inst_imu;
max30102 inst_ppg;

encoder inst_ec(PIN_ENCODER_A, PIN_ENCODER_B);
button  inst_btn(PIN_ENCODER_C);

buzzer inst_beep(PIN_BUZZER);

//
void board_init(void)
{
    setCpuFrequencyMhz(240);  // 默认80MHz

    inst_beep.play(start_up);

    /* Init serial port */
    Serial.begin(115200);
    shell.attach(Serial);
    shell.addCommand(F("echo"), cmd_echo);
    shell.addCommand(F("info"), cmd_info);
    shell.addCommand(F("scan"), cmd_scan);
    shell.addCommand(F("wifi"), cmd_wifi);

    /* Init bus */

    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    Wire.setClock(4e5);  // 400k

    SPI.begin(PIN_LCD_SCL, -1, PIN_LCD_SDA, -1);
    SPI.setFrequency(40e6);  // 40M

    /* Init bsp */

    // PMU
    {
        inst_pmu.init();

        // 开关机
        inst_pmu.set_power_on_time(AXP173_POWER_ON_2S);
        inst_pmu.set_power_off_time(AXP173_POWER_OFF_TIME_4S);

        // VBUS
        inst_pmu.set_adc_enable(AXP173_ADC_CHANNEL_VBUS_V, true);
        inst_pmu.set_adc_enable(AXP173_ADC_CHANNEL_VBUS_C, true);

        // 电池
        inst_pmu.set_adc_enable(AXP173_ADC_CHANNEL_BAT_V, true);
        inst_pmu.set_adc_enable(AXP173_ADC_CHANNEL_BAT_C, true);

        // 库仑计
        inst_pmu.set_coulometer(AXP173_COULOMETER_ENABLE, true);

        // 充电
        inst_pmu.set_charge_enable(true);
        inst_pmu.set_charge_current(AXP173_CHARGE_CURRENT_550mA);  // 电流
    }

    inst_imu.init();  // IMU
    // 底板 PIN_IMU_INT 与 BOOT 冲突了, 没加上拉电阻，导致无法启动，用刀子割开了

    inst_ctp.init();                                           // CTP
    inst_lcd.init(&SPI, PIN_LCD_DC, PIN_LCD_RST, PIN_LCD_BL);  // LCD

    // sd_init();  // SD

    inst_ppg.init();  // PPG

    // ECG
    {
        // PIN_ECG_ADC1 与 PIN_SD_MOSI 冲突了, 如要使用该 ADC，需取掉核心板上 PIN_SD_MOSI 的上拉电阻

        pinMode(PIN_ECG_ADC1, ANALOG);
        analogSetPinAttenuation(PIN_ECG_ADC1, ADC_2_5db);
        // 衰减倍数, 用低衰减采集低电压，高衰减意味大的采集范围
    }

    /* Init lvgl */
    shell.println("[Board] init lvgl...");
    lv_init();
    lv_port_disp_init();
    lv_port_indev_init();
}

void board_loop(void)
{
    shell.executeIfInput();
    lv_timer_handler();
}

////////////////////////////////////////////////////////////////////////

/** @defgroup shell command */

/**
 * @brief print board info
 * @example shell.execute("info")
 */
int cmd_info(int argc, char** argv)
{
    shell.printf("%s %s (" __DATE__ ")", PROJECT, VERISON);
    shell.println("");
    shell.printf("- Run time: %lu", millis());
    shell.println("");

    /* chip information */
    esp_chip_info_t chip_info;
    uint32_t        flash_size;
    esp_chip_info(&chip_info);
    if (ESP_OK != esp_flash_get_size(NULL, &flash_size)) {
        shell.printf("Get flash size failed");
        shell.println("");
        return EXIT_FAILURE;
    } else {
        shell.printf("- %luMB %s flash", flash_size / (1024 * 1024), (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");
        shell.println("");
        shell.printf("- Minimum free heap size: %ld bytes", esp_get_minimum_free_heap_size());
        shell.println("");
        return EXIT_SUCCESS;
    }
}

// example:  "echo Hello world"
// prints:  "Hello world\n" to attached stream
int cmd_echo(int argc, char** argv)
{
    auto lastArg = argc - 1;

    for (int i = 1; i < argc; i++) {
        shell.print(argv[i]);
        if (i < lastArg) {
            shell.print(F(" "));
        }
    }

    shell.println();
    return EXIT_SUCCESS;
}

/** @brief int to control */
int cmd_itoctl(int argc, char** argv)
{
    if (argc == 2) {
        int32_t ctl = atoi(argv[1]);
#if 1
        ctl;
#else
        switch (ctl) {
            case 0: break;
            case 1: break;
            case 2: break;
            case 3: break;
            default: break;
        }
#endif
    }
    return EXIT_SUCCESS;
}

int cmd_scan(int argc, char** argv)
{
    i2c_scan();
    return EXIT_SUCCESS;
}

int cmd_wifi(int argc, char** argv)
{
    return EXIT_SUCCESS;
}