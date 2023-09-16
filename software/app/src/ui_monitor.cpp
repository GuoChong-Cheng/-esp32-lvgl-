#include "ui_monitor.hpp"

static void floating_toolbar_create(lv_obj_t* parent);
static void floating_toolbar_callback(lv_event_t* e);
static void color_event_cb(lv_event_t* e);

static void event_sampler_state_invert(lv_event_t* e);
static void heartrate_label_create(lv_obj_t* parent);

lv_obj_t*          obj_chart  = nullptr;
lv_chart_series_t* series_ecg = nullptr;
lv_chart_series_t* series_ppg = nullptr;

lv_obj_t* lbl_ppm[3];

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ui_monitor(lv_obj_t* parent)
{
    if (nullptr == parent) parent = lv_scr_act();

    {
        obj_chart = lv_chart_create(parent);
        lv_obj_set_size(obj_chart, LV_HOR_RES, LV_VER_RES);
        lv_obj_center(obj_chart);

        // style
        lv_chart_set_type(obj_chart, LV_CHART_TYPE_LINE);

        /**
         * 移除小圆点:
         * @file lv_chart.c @fn draw_series_line()
         *   lv_coord_t point_w = lv_obj_get_style_width(obj, LV_PART_INDICATOR) / 2;  => point_w = 0;
         *   lv_coord_t point_h = lv_obj_get_style_height(obj, LV_PART_INDICATOR) / 2; => point_h = 0;
         */

        // grid
        lv_chart_set_div_line_count(obj_chart, 5, 7);
        // points
        lv_chart_set_point_count(obj_chart, n_displayed_points);

        // range
        lv_chart_set_range(obj_chart, LV_CHART_AXIS_PRIMARY_Y, -100, 4195);
        // lv_chart_set_range(obj_chart, LV_CHART_AXIS_SECONDARY_Y, -32768, 32767);

        // ticks
        lv_chart_set_axis_tick(obj_chart, LV_CHART_AXIS_PRIMARY_Y, 10, 5, 6, 2, true, 50);

#if CONFIG_OSC_MODE
        lv_chart_set_update_mode(obj_chart, LV_CHART_UPDATE_MODE_CIRCULAR);
#else
        lv_chart_set_update_mode(obj_chart, LV_CHART_UPDATE_MODE_SHIFT);
#endif

        // series
        series_ecg = lv_chart_add_series(obj_chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);
        series_ppg = lv_chart_add_series(obj_chart, lv_palette_main(LV_PALETTE_BLUE), LV_CHART_AXIS_PRIMARY_Y);
    }

    {
        floating_toolbar_create(parent);
        heartrate_label_create(parent);
    }
}

void add_val_to_monitor(lv_chart_series_t* ser, lv_coord_t data)
{
#if CONFIG_OSC_MODE

    lv_chart_set_next_value(obj_chart, ser, data);

    uint16_t    p = lv_chart_get_point_count(obj_chart);
    uint16_t    s = lv_chart_get_x_start_point(obj_chart, ser);
    lv_coord_t* a = lv_chart_get_y_array(obj_chart, ser);

    a[(s + 1) % p] = LV_CHART_POINT_NONE;
    a[(s + 2) % p] = LV_CHART_POINT_NONE;
    a[(s + 2) % p] = LV_CHART_POINT_NONE;

#else
    lv_chart_set_next_value(obj_chart, ser, data);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct {
    lv_palette_t  color;
    const char*   image;
    lv_event_cb_t event;
    void*         data;  // for ecb
} toolbtns[] = {
    {LV_PALETTE_PURPLE, LV_SYMBOL_IMAGE, nullptr,                    nullptr},
    {LV_PALETTE_ORANGE, LV_SYMBOL_STOP,  event_sampler_state_invert, nullptr},
};

#define CONFIG_MAIN_BUTTON_SIZE  LV_DPX(52)
#define CONFIG_SMALL_BUTTON_SIZE LV_DPX(32)

#define FLOATING_TOOLBAR_WIDTH   (CONFIG_MAIN_BUTTON_SIZE + ARRAY_SIZE(toolbtns) * CONFIG_SMALL_BUTTON_SIZE * 2)  // 展开长度

static void heartrate_label_create(lv_obj_t* parent)
{
    lv_obj_t* cont = lv_obj_create(parent);
    lv_obj_remove_style_all(cont);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_FLOATING);
    lv_obj_set_style_bg_opa(cont, LV_OPA_10, 0);

    lv_obj_set_height(cont, 60);
    lv_obj_align(cont, LV_ALIGN_BOTTOM_LEFT, LV_DPX(15), -LV_DPX(15));

    uint8_t i;
    for (i = 0; i < 2; ++i) {
        lbl_ppm[i] = lv_label_create(cont);
        lv_label_set_text(lbl_ppm[i], "- ppm");
    }
}

static void floating_toolbar_create(lv_obj_t* parent)
{
    // container

    lv_obj_t* color_cont = lv_obj_create(parent);
    lv_obj_remove_style_all(color_cont);
    lv_obj_set_flex_flow(color_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(color_cont, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_add_flag(color_cont, LV_OBJ_FLAG_FLOATING);

    lv_obj_set_style_bg_color(color_cont, lv_color_white(), 0);
    lv_obj_set_style_pad_right(color_cont, LV_DPX(47), 0);
    lv_obj_set_style_bg_opa(color_cont, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(color_cont, LV_RADIUS_CIRCLE, 0);

#if 0
    lv_obj_set_style_border_width(color_cont, LV_DPX(1), LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(color_cont, lv_palette_main(LV_PALETTE_GREY), LV_STATE_DEFAULT);
#endif

    lv_obj_set_size(color_cont, CONFIG_MAIN_BUTTON_SIZE, CONFIG_MAIN_BUTTON_SIZE);

    lv_obj_align(color_cont, LV_ALIGN_BOTTOM_RIGHT, -LV_DPX(10), -LV_DPX(10));

    // small buttons

    uint8_t i;
    for (i = 0; i < ARRAY_SIZE(toolbtns); ++i) {
        lv_obj_t* obj = lv_btn_create(color_cont);
        lv_obj_set_style_bg_color(obj, lv_palette_main(toolbtns[i].color), 0);
        lv_obj_set_style_radius(obj, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_opa(obj, LV_OPA_TRANSP, 0);
        lv_obj_set_size(obj, CONFIG_SMALL_BUTTON_SIZE, CONFIG_SMALL_BUTTON_SIZE);
        if (toolbtns[i].event) lv_obj_add_event_cb(obj, toolbtns[i].event, LV_EVENT_CLICKED, toolbtns[i].data);
        if (toolbtns[i].image) lv_obj_set_style_bg_img_src(obj, toolbtns[i].image, 0);
        lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    }

    // big button

    lv_obj_t* btn = lv_btn_create(parent);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_FLOATING | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_color(btn, lv_color_white(), LV_STATE_CHECKED);
    lv_obj_set_style_pad_all(btn, 10, 0);
    lv_obj_set_style_radius(btn, LV_RADIUS_CIRCLE, 0);
    lv_obj_add_event_cb(btn, floating_toolbar_callback, LV_EVENT_ALL, color_cont);
    lv_obj_set_style_shadow_width(btn, 0, 0);
    lv_obj_set_style_bg_img_src(btn, LV_SYMBOL_EYE_CLOSE, 0);

    lv_obj_set_size(btn, LV_DPX(42), LV_DPX(42));
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -LV_DPX(15), -LV_DPX(15));
}

static void floating_toolbar_animate(void* var, int32_t v)
{
    lv_obj_t*  obj   = (lv_obj_t*)var;
    lv_coord_t max_w = FLOATING_TOOLBAR_WIDTH;  // lv_obj_get_width(lv_obj_get_parent(obj)) - LV_DPX(20);  // 宽度
    lv_coord_t w;

    w = lv_map(v, 0, 256, CONFIG_MAIN_BUTTON_SIZE, FLOATING_TOOLBAR_WIDTH);
    lv_obj_set_width(obj, w);
    lv_obj_align(obj, LV_ALIGN_BOTTOM_RIGHT, -LV_DPX(10), -LV_DPX(10));

    if (v > LV_OPA_COVER) v = LV_OPA_COVER;

    uint32_t i;
    for (i = 0; i < lv_obj_get_child_cnt(obj); i++) {
        lv_obj_set_style_opa(lv_obj_get_child(obj, i), v, 0);
    }
}

static void floating_toolbar_callback(lv_event_t* e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        lv_obj_t* btn        = lv_event_get_target(e);
        lv_obj_t* color_cont = (lv_obj_t*)lv_event_get_user_data(e);

        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, color_cont);
        lv_anim_set_exec_cb(&a, floating_toolbar_animate);
        if (lv_obj_get_width(color_cont) < FLOATING_TOOLBAR_WIDTH) {
            lv_obj_set_style_bg_img_src(btn, LV_SYMBOL_EYE_OPEN, 0);
            lv_anim_set_values(&a, 0, 256);
        } else {
            lv_obj_set_style_bg_img_src(btn, LV_SYMBOL_EYE_CLOSE, 0);
            lv_anim_set_values(&a, 256, 0);
        }
        lv_anim_set_time(&a, 200);
        lv_anim_start(&a);
    }
}

static void event_sampler_state_invert(lv_event_t* e)
{
    lv_obj_t* obj = lv_event_get_target(e);

    extern hw_timer_t* g_sampler;

    if (timerAlarmEnabled(g_sampler)) {
        timerAlarmDisable(g_sampler);
        lv_obj_set_style_bg_img_src(obj, LV_SYMBOL_PLAY, 0);
    } else {
        timerAlarmEnable(g_sampler);
        lv_obj_set_style_bg_img_src(obj, LV_SYMBOL_STOP, 0);
    }
}
