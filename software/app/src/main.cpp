#include "bsp/board.h"

#include "ui_monitor.hpp"
#include "ui_settings.hpp"
#include "ecg_sampler.hpp"

void onTmrSampler();

ecg_sampler inst_ecg;
hw_timer_t* g_sampler = nullptr;

void Task1code(void* pvParameters);
void Task2code(void* pvParameters);
void onTmrSampler();

void setup()
{
    board_init();

    /* ui */

    lv_obj_t *obj_view, *obj_tile;

    obj_view = lv_tileview_create(lv_scr_act());

    obj_tile = lv_tileview_add_tile(obj_view, 0, 0, LV_DIR_RIGHT);
    // obj_tile = lv_tileview_add_tile(obj_view, 0, 0, LV_DIR_RIGHT | LV_DIR_BOTTOM);
    ui_monitor(obj_tile);

    obj_tile = lv_tileview_add_tile(obj_view, 1, 0, LV_DIR_LEFT);
    ui_settings(obj_tile);

    // obj_tile = lv_tileview_add_tile(obj_view, 0, 1, LV_DIR_TOP);

    // task
    xTaskCreatePinnedToCore(Task1code, "LVGL", 10000, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(Task2code, "SIG", 10000, NULL, 1, NULL, 1);
}

void loop() {}

//////////

void Task1code(void* pvParameters)
{
    while (1) {
        board_loop();
        delay(1);
    }
}

//////////

#define DIVLINE  2047
#define BASELINE 1023

static bool do_sampler = false;

void Task2code(void* pvParameters)
{
    /* sampler */
    g_sampler = timerBegin(0, 240, true);    // 分频 240Mz / 240 = 1MHz
    timerAttachInterrupt(g_sampler, onTmrSampler, false);
    timerAlarmWrite(g_sampler, 20e3, true);  // 重装载(单位us) 1e6Hz/20e3=50Hz
    // timerAlarmWrite(g_sampler, 5e3, true);  // 重装载(单位us) 1e6Hz/5e3=200Hz
    timerAlarmEnable(g_sampler);

    while (1) {
        if (do_sampler) {
            do_sampler = !do_sampler;
            if (inst_ppg.read() && inst_ecg.read()) {
                lv_label_set_text_fmt(lbl_ppm[0], "%d ppm", inst_ppg._average_bpm);
                lv_label_set_text_fmt(lbl_ppm[1], "%d ppm", inst_ppg._bpm);
            }
            if (inst_ppg.finger_detected && inst_ecg.finger_detected) {
                int16_t val;
                // ppg
                val = map(inst_ppg._ret, -17000, 9000, 0, DIVLINE);
                add_val_to_monitor(series_ppg, val);
                // ecg
                val = map(inst_ecg._ret, 2048, 4095, DIVLINE, 4095);
                add_val_to_monitor(series_ecg, val);
            } else {
                add_val_to_monitor(series_ppg, BASELINE);
                add_val_to_monitor(series_ecg, BASELINE + DIVLINE);
            }
        } else {
            delay(1);
        }
    }
}

void onTmrSampler()
{
    do_sampler = true;
}
