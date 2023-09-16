#pragma once

#include "lvgl.h"
#include "lib/defs.h"

#define CONFIG_OSC_MODE    1

#define n_displayed_points 100  // number of points displayed

extern lv_chart_series_t* series_ecg;
extern lv_chart_series_t* series_ppg;

extern lv_obj_t* lbl_ppm[3];

void ui_monitor(lv_obj_t* parent = nullptr);
void add_val_to_monitor(lv_chart_series_t* ser, lv_coord_t data);
