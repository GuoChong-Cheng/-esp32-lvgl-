#include "ui_settings.hpp"

#define USE_CHART_TICKS      0

#define CONFIG_BATTERY_ALARM 40  // 低电压预警值

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ecb_page_changed(lv_event_t* e);
static void ecb_chart_battery(lv_event_t* e);

//

typedef struct {
    struct {
        lv_obj_t* section;
        lv_obj_t *x, *y, *z;  // bar
    } accel, gyro, angle;
} motion_sampler_t;

static void ecb_page_motion_sampler(bool state, void* user_data);

typedef struct {
    struct {
        lv_obj_t *v, *i;  // lbl
    } bus;

    struct {
        lv_obj_t* state;      // img
        lv_obj_t *v, *i, *p;  // lbl
    } bat;
} power_sampler_t;

static void ecb_page_power_sampler(bool state, void* user_data);

//

enum {
    ECB_PAGE_MOTION = 0,
    ECB_PAGE_POWER  = 1,
    ECB_PAGE_COUNT,
};

struct {
    lv_obj_t* page;
    void*     data;
} ecb_pages[ECB_PAGE_COUNT];

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ui_settings(lv_obj_t* parent)
{
    lv_settings root(parent, ecb_page_changed);

    root.add_separator();

    {
        root.start_section();

        {
            root.add_page(LV_SYMBOL_USB, "Shell");

            {
                root.start_section();
                root.add_label(nullptr, "Baudrate"), root.add_sameline(), root.add_dropdown("9600\n115200\n921600");
                root.stop_section();
            }

            root.end_page();

            root.add_page(LV_SYMBOL_DRIVE, "Network");

            {
                root.start_section();
                root.add_label(LV_SYMBOL_UPLOAD, "Server"), root.add_sameline(), root.add_switch(false);
                root.add_label(nullptr, "- IP"), root.add_sameline(), root.add_lineedit("192.168.3.20");
                root.add_label(nullptr, "- Port"), root.add_sameline(), root.add_lineedit("5000");
                root.stop_section();
            }

            root.add_separator();

            {
                root.start_section();
                root.add_label(LV_SYMBOL_WIFI, "WiFi"), root.add_sameline(), root.add_switch(false);
                root.stop_section();
            }

            root.end_page();

            if (g_sdcrad_ok) {
                root.add_page(LV_SYMBOL_SD_CARD, "SDCard");
                root.end_page();
            }
        }

        root.stop_section();
    }

    root.add_separator();

    {
        root.start_section();

        {
            root.add_page(LV_SYMBOL_SHUFFLE, "Sampler");

            {
                root.start_section();
                root.add_label(nullptr, "Chart");
                root.add_label(nullptr, "- Mode"), root.add_sameline(), root.add_dropdown("Shift\nCircular");
                root.add_label(nullptr, "- Points"), root.add_sameline(), root.add_silder(200, 100, 400);
                root.stop_section();
            }

            root.add_separator();

            {
                root.start_section();
                root.add_label(nullptr, "Filter"), root.add_sameline(), root.add_switch(true);
                root.add_label(nullptr, "- Type"), root.add_sameline(), root.add_dropdown("Median Average\nFirst Order Lowpass\n20Hz FIR LPF");
                root.add_label(nullptr, "- Order"), root.add_sameline(), root.add_silder(3, 1, 20);  // 阶数
                root.add_label(nullptr, "- Coeffi"), root.add_sameline(), root.add_silder(10, 1, 100);
                root.stop_section();
            }

            root.end_page();
        }

        {
            static motion_sampler_t motion;

            ecb_pages[ECB_PAGE_MOTION].page = root.add_page(LV_SYMBOL_GPS, "Motion");
            ecb_pages[ECB_PAGE_MOTION].data = &motion;

            {
                motion.accel.section = root.start_section();
                root.add_label(nullptr, "ax"), root.add_sameline(), motion.accel.x = root.add_offset_bar(0, -100, 100);
                root.add_label(nullptr, "ay"), root.add_sameline(), motion.accel.y = root.add_offset_bar(0, -100, 100);
                root.add_label(nullptr, "az"), root.add_sameline(), motion.accel.z = root.add_offset_bar(0, -100, 100);
                root.stop_section();

                root.add_separator();

                motion.gyro.section = root.start_section();
                root.add_label(nullptr, "gx"), root.add_sameline(), motion.gyro.x = root.add_offset_bar(0, -100, 100);
                root.add_label(nullptr, "gy"), root.add_sameline(), motion.gyro.y = root.add_offset_bar(0, -100, 100);
                root.add_label(nullptr, "gz"), root.add_sameline(), motion.gyro.z = root.add_offset_bar(0, -100, 100);
                root.stop_section();

                root.add_separator();

                motion.angle.section = root.start_section();
                root.add_label(nullptr, "roll"), root.add_sameline(), motion.angle.x  = root.add_offset_bar(0, -180, 180);
                root.add_label(nullptr, "pitch"), root.add_sameline(), motion.angle.y = root.add_offset_bar(0, -180, 180);
                root.add_label(nullptr, "yaw"), root.add_sameline(), motion.angle.z   = root.add_offset_bar(0, -180, 180);
                root.stop_section();
            }

            root.add_separator();
        }

        {
            static power_sampler_t pwr;

            ecb_pages[ECB_PAGE_POWER].page = root.add_page(LV_SYMBOL_BATTERY_FULL, "Battery");
            ecb_pages[ECB_PAGE_POWER].data = &pwr;

            root.start_section();
            {
                lv_obj_t* chart = root.add_chart();

                lv_obj_set_height(chart, 160);

                lv_chart_set_point_count(chart, 12);
                lv_chart_set_div_line_count(chart, 0, 12);

                lv_obj_set_style_border_side(chart, LV_BORDER_SIDE_FULL, 0);  // 边框

#if USE_CHART_TICKS
                lv_chart_set_axis_tick(chart, LV_CHART_AXIS_PRIMARY_X, 0, 0, 12, 1, true, 100);  // 刻度
#endif

                extern void ecb_chart_battery(lv_event_t * e);
                lv_obj_add_event_cb(chart, ecb_chart_battery, LV_EVENT_ALL, NULL);

                lv_chart_series_t* ser = lv_chart_add_series(chart, lv_theme_get_color_primary(chart), LV_CHART_AXIS_PRIMARY_Y);  // 剩余电量

                for (uint8_t i = 0; i < 12; ++i) {
                    lv_chart_set_next_value(chart, ser, lv_rand(0, 70));
                }
            }
            root.stop_section();

            root.add_separator();

            root.start_section();
            {
                root.add_label(nullptr, "BUS");
                root.add_label(nullptr, "Voltage"), root.add_sameline(), pwr.bus.v = root.add_label(nullptr, "", 1);
                root.add_label(nullptr, "Current"), root.add_sameline(), pwr.bus.i = root.add_label(nullptr, "", 1);
            }
            root.stop_section();

            root.add_separator();

            root.start_section();
            {
                pwr.bat.state = root.add_label(LV_SYMBOL_DUMMY, "BAT", 0);
                lv_obj_add_flag(pwr.bat.state, LV_OBJ_FLAG_HIDDEN);
                root.add_label(nullptr, "Voltage"), root.add_sameline(), pwr.bat.v = root.add_label(nullptr, "", 1);
                root.add_label(nullptr, "Current"), root.add_sameline(), pwr.bat.i = root.add_label(nullptr, "", 1);
                root.add_label(nullptr, "Power"), root.add_sameline(), pwr.bat.p   = root.add_label(nullptr, "", 1);
            }
            root.stop_section();

            root.end_page();
        }

        root.stop_section();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ecb_page_changed(lv_event_t* e)
{
    lv_obj_t* obj_menu = lv_event_get_target(e);
    lv_obj_t* obj_page = lv_menu_get_cur_main_page(obj_menu);

    ecb_page_motion_sampler(ecb_pages[ECB_PAGE_MOTION].page == obj_page, ecb_pages[ECB_PAGE_MOTION].data);
    ecb_page_power_sampler(ecb_pages[ECB_PAGE_POWER].page == obj_page, ecb_pages[ECB_PAGE_POWER].data);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ecb_chart_battery(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t*       obj  = lv_event_get_target(e);

    if (code == LV_EVENT_PRESSED || code == LV_EVENT_RELEASED) {
        lv_obj_invalidate(obj); /* To make the value boxes visible */
    } else if (code == LV_EVENT_DRAW_PART_BEGIN) {
        lv_obj_draw_part_dsc_t* dsc = (lv_obj_draw_part_dsc_t*)lv_event_get_param(e);
        /* set the markers' text */
        if (dsc->part == LV_PART_TICKS && dsc->id == LV_CHART_AXIS_PRIMARY_X) {
#if USE_CHART_TICKS
            const char* month[] = {"I", "II", "III", "IV", "V", "VI", "VII", "VIII", "IX", "X", "XI", "XII"};
            lv_snprintf(dsc->text, dsc->text_length, "%s", month[dsc->value]);
#endif
        }
        /* add the faded area before the lines are drawn */
        else if (dsc->part == LV_PART_ITEMS) {
#if 1  // LV_USE_DRAW_MASKS
            /* add a line mask that keeps the area below the line */
            if (dsc->p1 && dsc->p2) {
                lv_draw_mask_line_param_t line_mask_param;
                lv_draw_mask_line_points_init(&line_mask_param, dsc->p1->x, dsc->p1->y, dsc->p2->x, dsc->p2->y, LV_DRAW_MASK_LINE_SIDE_BOTTOM);
                int16_t line_mask_id = lv_draw_mask_add(&line_mask_param, NULL);

                /* add a fade effect: transparent bottom covering top */
                lv_coord_t                h = lv_obj_get_height(obj);
                lv_draw_mask_fade_param_t fade_mask_param;
                lv_draw_mask_fade_init(&fade_mask_param, &obj->coords, LV_OPA_COVER, obj->coords.y1 + h / 8, LV_OPA_TRANSP, obj->coords.y2);
                int16_t fade_mask_id = lv_draw_mask_add(&fade_mask_param, NULL);

                lv_area_t obj_clip_area;  // 剪辑区
                _lv_area_intersect(&obj_clip_area, dsc->draw_ctx->clip_area, &obj->coords);
                const lv_area_t* clip_area_ori = dsc->draw_ctx->clip_area;
                dsc->draw_ctx->clip_area       = &obj_clip_area;

                lv_area_t a;  // 填充区域
                a.x1 = dsc->p1->x;
                a.x2 = dsc->p2->x - 1;
                a.y1 = LV_MIN(dsc->p1->y, dsc->p2->y);
                a.y2 = obj->coords.y2;

                /* draw a rectangle that will be affected by the mask */
                lv_draw_rect_dsc_t draw_rect_dsc;
                lv_draw_rect_dsc_init(&draw_rect_dsc);
                draw_rect_dsc.bg_opa = LV_OPA_50;

                lv_coord_t y_alarm = (float)a.y1 / dsc->value * CONFIG_BATTERY_ALARM;
                lv_coord_t x_alarm = (float)(y_alarm - a.y1) * (a.x2 - a.x1) / (a.y2 - a.y1) + a.x1;

                // 低于阈值和高于阈值分别使用不同颜色绘制

                if (a.y1 < y_alarm && a.y2 < y_alarm) {
                    draw_rect_dsc.bg_color = lv_palette_main(LV_PALETTE_RED);  // 阴影色
                    lv_draw_rect(dsc->draw_ctx, &draw_rect_dsc, &a);
                } else if (a.y1 < y_alarm) {
                    lv_coord_t t;

                    // lv_log("1. %d,%d\n", a.x1, a.x2);

                    t    = a.x1;
                    a.x1 = x_alarm;

                    draw_rect_dsc.bg_color = dsc->line_dsc->color;
                    lv_draw_rect(dsc->draw_ctx, &draw_rect_dsc, &a);

                    a.x2 = a.x1;
                    a.x1 = t;

                    draw_rect_dsc.bg_color = lv_palette_main(LV_PALETTE_RED);
                    lv_draw_rect(dsc->draw_ctx, &draw_rect_dsc, &a);

                } else if (a.y2 < y_alarm) {
                    lv_coord_t t;

                    t    = a.x2;
                    a.x2 = x_alarm;

                    // lv_log("2. %d,%d\n", a.x1, a.x2);

                    draw_rect_dsc.bg_color = dsc->line_dsc->color;
                    lv_draw_rect(dsc->draw_ctx, &draw_rect_dsc, &a);

                    a.x1 = a.x2;
                    a.x2 = t;

                    draw_rect_dsc.bg_color = lv_palette_main(LV_PALETTE_RED);
                    lv_draw_rect(dsc->draw_ctx, &draw_rect_dsc, &a);

                } else {
                    draw_rect_dsc.bg_color = dsc->line_dsc->color;
                    lv_draw_rect(dsc->draw_ctx, &draw_rect_dsc, &a);
                }

                dsc->draw_ctx->clip_area = clip_area_ori;

                /* remove the masks */
                lv_draw_mask_remove_id(line_mask_id);
                lv_draw_mask_remove_id(fade_mask_id);
            }
#endif

            const lv_chart_series_t* ser = (lv_chart_series_t*)dsc->sub_part_ptr;

            if (lv_chart_get_pressed_point(obj) == dsc->id) {
                if (lv_chart_get_type(obj) == LV_CHART_TYPE_LINE) {
                    dsc->rect_dsc->outline_color = lv_color_white();
                    dsc->rect_dsc->outline_width = 2;
                } else {
                    dsc->rect_dsc->shadow_color  = ser->color;
                    dsc->rect_dsc->shadow_width  = 15;
                    dsc->rect_dsc->shadow_spread = 0;
                }

                char buf[8];
                lv_snprintf(buf, sizeof(buf), "%d", dsc->value);

                lv_point_t text_size;
                lv_txt_get_size(&text_size, buf, LV_FONT_DEFAULT, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);

                lv_area_t txt_area;
                if (lv_chart_get_type(obj) == LV_CHART_TYPE_BAR) {
                    txt_area.y2 = dsc->draw_area->y1 - LV_DPX(15);
                    txt_area.y1 = txt_area.y2 - text_size.y;
                    if (ser == lv_chart_get_series_next(obj, NULL)) {
                        txt_area.x1 = dsc->draw_area->x1 + lv_area_get_width(dsc->draw_area) / 2;
                        txt_area.x2 = txt_area.x1 + text_size.x;
                    } else {
                        txt_area.x2 = dsc->draw_area->x1 + lv_area_get_width(dsc->draw_area) / 2;
                        txt_area.x1 = txt_area.x2 - text_size.x;
                    }
                } else {
                    txt_area.x1 = dsc->draw_area->x1 + lv_area_get_width(dsc->draw_area) / 2 - text_size.x / 2;
                    txt_area.x2 = txt_area.x1 + text_size.x;
                    txt_area.y2 = dsc->draw_area->y1 - LV_DPX(15);
                    txt_area.y1 = txt_area.y2 - text_size.y;
                }

                // draw value box

                lv_area_t bg_area;
                bg_area.x1 = txt_area.x1 - LV_DPX(8);
                bg_area.x2 = txt_area.x2 + LV_DPX(8);
                bg_area.y1 = txt_area.y1 - LV_DPX(8);
                bg_area.y2 = txt_area.y2 + LV_DPX(8);

                lv_draw_rect_dsc_t rect_dsc;
                lv_draw_rect_dsc_init(&rect_dsc);
                rect_dsc.bg_color = ser->color;
                rect_dsc.radius   = LV_DPX(5);
                lv_draw_rect(dsc->draw_ctx, &rect_dsc, &bg_area);  // background

                lv_draw_label_dsc_t label_dsc;
                lv_draw_label_dsc_init(&label_dsc);
                label_dsc.color = lv_color_white();
                label_dsc.font  = LV_FONT_DEFAULT;
                lv_draw_label(dsc->draw_ctx, &label_dsc, &txt_area, buf, NULL);  // value
            } else {
                dsc->rect_dsc->outline_width = 0;
                dsc->rect_dsc->shadow_width  = 0;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ecb_timer_motion_sampler(lv_timer_t* timer)
{
    motion_sampler_t* motion = (motion_sampler_t*)(timer->user_data);

    inst_imu.update();

    lv_offset_bar_set_value(motion->accel.x, inst_imu.ax * 100);
    lv_offset_bar_set_value(motion->accel.y, inst_imu.ay * 100);
    lv_offset_bar_set_value(motion->accel.z, inst_imu.az * 100);
    lv_offset_bar_set_value(motion->gyro.x, inst_imu.gx * 10);
    lv_offset_bar_set_value(motion->gyro.y, inst_imu.gy * 10);
    lv_offset_bar_set_value(motion->gyro.z, inst_imu.gz * 10);
    lv_offset_bar_set_value(motion->angle.x, inst_imu.roll);
    lv_offset_bar_set_value(motion->angle.y, inst_imu.pitch);
    lv_offset_bar_set_value(motion->angle.z, inst_imu.yaw);
}

static void ecb_page_motion_sampler(bool state, void* user_data)
{
    static lv_timer_t* tmr = nullptr;

    if (state) {
        tmr = lv_timer_create(ecb_timer_motion_sampler, 100, user_data);
    } else {
        if (tmr) {
            lv_timer_del(tmr);
            tmr = nullptr;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ecb_timer_power_sampler(lv_timer_t* timer)
{
    power_sampler_t* pwr = (power_sampler_t*)(timer->user_data);

    float vbat;

    // 需使能 LV_SPRINTF_CUSTOM
    lv_label_set_text_fmt(pwr->bus.v, "%.2f V", inst_pmu.get_vbus_voltage());
    lv_label_set_text_fmt(pwr->bus.i, "%.2f mA", inst_pmu.get_vbus_current());
    lv_label_set_text_fmt(pwr->bat.v, "%.2f V", vbat = inst_pmu.get_bat_voltage());
    lv_label_set_text_fmt(pwr->bat.i, "%.2f mA", inst_pmu.get_bat_current());
    lv_label_set_text_fmt(pwr->bat.p, "%.2f mW", inst_pmu.get_bat_power());

#define VBAT_FULL 4.2f

    char* img;

    if (inst_pmu.is_bat_exist()) {
        lv_obj_clear_flag(pwr->bat.state, LV_OBJ_FLAG_HIDDEN);
        if (inst_pmu.is_charging()) {
            img = (char*)LV_SYMBOL_CHARGE;
        } else {
            if (vbat <= (VBAT_FULL * 0.1f)) {
                img = (char*)LV_SYMBOL_BATTERY_EMPTY;
            } else if (vbat <= (VBAT_FULL * 0.25f)) {
                img = (char*)LV_SYMBOL_BATTERY_1;
            } else if (vbat <= (VBAT_FULL * 0.5f)) {
                img = (char*)LV_SYMBOL_BATTERY_2;
            } else if (vbat <= (VBAT_FULL * 0.75f)) {
                img = (char*)LV_SYMBOL_BATTERY_3;
            } else {
                img = (char*)LV_SYMBOL_BATTERY_FULL;
            }
        }
    } else {
        lv_obj_add_flag(pwr->bat.state, LV_OBJ_FLAG_HIDDEN);
        img = (char*)LV_SYMBOL_DUMMY;
    }

    lv_img_set_src(pwr->bat.state, img);
}

static void ecb_page_power_sampler(bool state, void* user_data)
{
    static lv_timer_t* tmr = nullptr;

    if (state) {
        tmr = lv_timer_create(ecb_timer_power_sampler, 1000, user_data);
    } else {
        if (tmr) {
            lv_timer_del(tmr);
            tmr = nullptr;
        }
    }
}
