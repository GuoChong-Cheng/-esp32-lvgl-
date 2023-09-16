
#include "lv_settings.hpp"

static lv_obj_t* g_keyboard = nullptr;

// text area defalut callback
static void ecb_ta_def(lv_event_t* e)
{
    lv_event_code_t event = lv_event_get_code(e);

    lv_obj_t* ta = (lv_obj_t*)lv_event_get_target(e);
    lv_obj_t* kb = (lv_obj_t*)lv_event_get_user_data(e);  // or g_keyboard

    switch (event) {
        case LV_EVENT_FOCUSED: {
            lv_keyboard_set_textarea(kb, ta);
            lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
            break;
        }
        case LV_EVENT_CANCEL:  // 左下小键盘按钮
        case LV_EVENT_READY:   // 右下确认按钮
        case LV_EVENT_DEFOCUSED: {
            lv_keyboard_set_textarea(kb, NULL);
            lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
            break;
        }
    }
}

static void event_offset_bar_def(lv_event_t* e)
{
    lv_obj_draw_part_dsc_t* dsc = lv_event_get_draw_part_dsc(e);
    if (dsc->part != LV_PART_INDICATOR) return;

    lv_obj_t* obj = lv_event_get_target(e);

    lv_draw_label_dsc_t label_dsc;
    lv_draw_label_dsc_init(&label_dsc);
    label_dsc.font = LV_FONT_DEFAULT;

    char buf[8];
    lv_snprintf(buf, sizeof(buf), "%d", lv_bar_get_value(obj) > 0 ? lv_bar_get_value(obj) : -lv_bar_get_start_value(obj));

    lv_point_t txt_size;
    lv_txt_get_size(&txt_size, buf, label_dsc.font, label_dsc.letter_space, label_dsc.line_space, LV_COORD_MAX, label_dsc.flag);

    lv_area_t txt_area;
    /*If the indicator is long enough put the text inside on the right*/
    if (lv_area_get_width(dsc->draw_area) > txt_size.x + 20) {
        txt_area.x2     = dsc->draw_area->x2 - 5;
        txt_area.x1     = txt_area.x2 - txt_size.x + 1;
        label_dsc.color = lv_color_white();
    }
    /*If the indicator is still short put the text out of it on the right*/
    else {
        txt_area.x1     = dsc->draw_area->x2 + 5;
        txt_area.x2     = txt_area.x1 + txt_size.x - 1;
        label_dsc.color = lv_color_black();
    }

    txt_area.y1 = dsc->draw_area->y1 + (lv_area_get_height(dsc->draw_area) - txt_size.y) / 2;
    txt_area.y2 = txt_area.y1 + txt_size.y - 1;

    lv_draw_label(dsc->draw_ctx, &label_dsc, &txt_area, buf, NULL);
}

///////////////////////////////////////////////////////////////

lv_settings::lv_settings(lv_obj_t* parent, lv_event_cb_t cbk_page_changed, void* user_data) : m_levels(), m_sublevels()
{
    if (nullptr == g_keyboard) {
        g_keyboard = lv_keyboard_create(lv_scr_act());
        lv_obj_add_flag(g_keyboard, LV_OBJ_FLAG_HIDDEN);
    }

    m_sameline  = false;
    m_curr_sect = m_curr_cont = m_last_obj = nullptr;

    m_menu = lv_menu_create(parent ? parent : lv_scr_act());

    // set menu pos & size

    lv_obj_set_size(m_menu, LV_HOR_RES, LV_VER_RES);
    lv_obj_center(m_menu);

    m_offset = lv_obj_get_style_pad_left(lv_menu_get_main_header(m_menu), 0);

    // style

    lv_obj_set_style_bg_color(m_menu, lv_color_darken(lv_obj_get_style_bg_color(m_menu, 0), 12), 0);

    // root back button

#if 0
    if (cbk_btn_back) {
        lv_menu_set_mode_root_back_btn(m_menu, LV_MENU_ROOT_BACK_BTN_ENABLED);
        lv_obj_add_event_cb(m_menu, cbk_btn_back, LV_EVENT_CLICKED, nullptr);
    }
#else
    lv_menu_set_mode_root_back_btn(m_menu, LV_MENU_ROOT_BACK_BTN_ENABLED);

    // 更改：在 lv_menu.c 中的 lv_menu_constructor() 中
    // 将 lv_obj_set_flex_flow(main_header_back_btn, LV_FLEX_FLOW_ROW); 改为 lv_obj_set_size(main_header_back_btn, 24, 24);
    // 在 lv_img_set_src(main_header_back_icon, LV_SYMBOL_LEFT); 后增加 lv_obj_center(main_header_back_icon);

#endif

    if (cbk_page_changed) lv_obj_add_event_cb(m_menu, cbk_page_changed, LV_EVENT_VALUE_CHANGED, user_data);

    // new page
    m_curr_page = lv_menu_page_create(m_menu, "uYanki (1910950414)");
    lv_obj_set_style_pad_hor(m_curr_page, m_offset, 0);  // padding

    lv_menu_set_page(m_menu, m_curr_page);
    // lv_menu_set_sidebar_page(m_menu, page);
}

lv_obj_t* lv_settings::add_page(const char* icon, const char* text)
{
    // save cureent page
    m_levels.push_back(m_curr_page);

    // create new page
    lv_obj_t* page = lv_menu_page_create(m_menu, (char*)text);

    // set horizontal padding
    lv_obj_set_style_pad_hor(page, m_offset, 0);

    // bind label (click label and then auto jump to the page)
    lv_obj_t* cont = add_label(icon, text);
    lv_menu_set_load_page_event(m_menu, cont, page);

    return m_curr_page = page;
}

void lv_settings::end_page(void)
{
    // current page is top page ?
    if (m_levels.is_empty()) return;

    // recover previous page
    m_curr_page = m_levels.back()->data;

    m_levels.pop_back();
}

lv_obj_t* lv_settings::start_section(void)
{
    if (m_curr_sect) {
        m_sublevels.push_back(m_curr_sect);
    }

    return m_curr_sect = lv_menu_section_create(m_curr_page);
}

void lv_settings::stop_section(void)
{
    if (m_sublevels.is_empty()) {
        m_curr_sect = m_curr_cont = nullptr;
    } else {
        m_curr_sect = m_sublevels.front()->data;
        m_sublevels.pop_back();
    }
}

void lv_settings::add_sameline(void)
{
    m_sameline = true;
}

lv_obj_t* lv_settings::obj_parent(void)
{
    if (m_sameline) {
        m_sameline = false;
        if (m_curr_cont) {
            return m_curr_cont;
        }
    }

    m_curr_cont = lv_menu_cont_create(m_curr_sect ? m_curr_sect : m_curr_page);

    // lv_obj_set_size(m_curr_cont, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_flex_flow(m_curr_cont, LV_FLEX_FLOW_ROW_WRAP);

    return m_curr_cont;
}

lv_obj_t* lv_settings::add_separator(void)
{
    return m_last_obj = lv_menu_separator_create(m_curr_page);
}

lv_obj_t* lv_settings::add_label(const char* icon, const char* text, uint8_t ret)
{
    lv_obj_t* cont = obj_parent();

    lv_obj_t *img = nullptr, *lbl = nullptr;

    if (icon) {
        img = lv_img_create(cont);
        lv_img_set_src(img, icon);
        lv_obj_set_width(img, 18);  // right align text
        m_last_obj = img;
    }

    if (text) {
        lbl = lv_label_create(cont);
        lv_label_set_text(lbl, text);
        lv_label_set_long_mode(lbl, LV_LABEL_LONG_SCROLL_CIRCULAR);
        lv_obj_set_flex_grow(lbl, 1);
        m_last_obj = lbl;
    }

    switch (ret) {
        case 0: return img;
        case 1: return lbl;
        default: return cont;
    }
}

lv_obj_t* lv_settings::add_silder(int32_t val, int32_t min, int32_t max)
{
    lv_obj_t* cont = obj_parent();

    lv_obj_t* obj = lv_slider_create(cont);
    lv_obj_set_flex_grow(obj, 2);

    lv_obj_set_width(obj, LV_VER_RES);
    lv_slider_set_range(obj, min, max);
    lv_slider_set_value(obj, val, LV_ANIM_OFF);

    // lv_group_add_obj(ec_grp, obj);

    return m_last_obj = obj;
}

lv_obj_t* lv_settings::add_switch(bool chk)
{
    lv_obj_t* cont = obj_parent();

    lv_obj_t* obj = lv_switch_create(cont);

    lv_obj_set_height(obj, 24);

    lv_obj_add_state(obj, chk ? LV_STATE_CHECKED : 0);

    // lv_group_add_obj(ec_grp, obj);

    return m_last_obj = obj;
}

lv_obj_t* lv_settings::add_lineedit(const char* text)
{
    lv_obj_t* cont = obj_parent();

    lv_obj_t* obj = lv_textarea_create(cont);

    lv_obj_set_height(obj, 24);
    lv_obj_set_flex_grow(obj, 2);

    lv_textarea_set_text(obj, text);
    lv_textarea_set_cursor_pos(obj, -1);

    lv_textarea_set_one_line(obj, true);

    // lv_group_add_obj(ec_grp, obj);

    if (g_keyboard) lv_obj_add_event_cb(obj, ecb_ta_def, LV_EVENT_ALL, g_keyboard);

    return m_last_obj = obj;
}

lv_obj_t* lv_settings::add_dropdown(const char* items)
{
    lv_obj_t* cont = obj_parent();

    lv_obj_t* obj = lv_dropdown_create(cont);
    lv_obj_set_flex_grow(obj, 2);

    lv_obj_set_size(obj, lv_pct(60), 30);
    lv_obj_set_style_pad_ver(obj, 6, 0);

    lv_dropdown_set_options(obj, items);

    // lv_group_add_obj(ec_grp, obj);

    m_sameline = false;

    return m_last_obj = obj;
}

lv_obj_t* lv_settings::add_chart(void)
{
    lv_obj_t* cont = obj_parent();

    lv_obj_t* obj = lv_chart_create(cont);

    return m_last_obj = obj;
}

lv_obj_t* lv_settings::add_offset_bar(int32_t val, int32_t min, int32_t max)
{
    lv_obj_t* cont = obj_parent();

    lv_obj_t* obj = lv_bar_create(cont);
    lv_obj_set_flex_grow(obj, 3);

    lv_obj_set_height(obj, 24);

    lv_bar_set_mode(obj, LV_BAR_MODE_RANGE);
    lv_bar_set_range(obj, min, max);
    lv_offset_bar_set_value(obj, val);

    lv_obj_add_event_cb(obj, event_offset_bar_def, LV_EVENT_DRAW_PART_END, NULL);

    return m_last_obj = obj;
}

void lv_settings::attach(lv_event_cb_t callback, lv_event_code_t event = LV_EVENT_ALL, void* user_data)
{
    lv_obj_t* obj;
    if (m_last_obj) {
        obj = m_last_obj;
        // lv_log("attach object event");
    } else if (m_curr_cont) {
        obj = m_curr_cont;
        // lv_log("attach container event");
    } else if (m_curr_sect) {
        obj = m_curr_sect;
        // lv_log("attach section event");
    } else if (m_curr_page) {
        obj = m_curr_page;
        // lv_log("attach page event");
    } else {
        return;
    }

    lv_obj_add_event_cb(obj, callback, event, user_data);
}

void lv_offset_bar_set_value(lv_obj_t* obj, int32_t val)
{
    if (val < 0) {
        lv_bar_set_start_value(obj, val, LV_ANIM_OFF);
        lv_bar_set_value(obj, 0, LV_ANIM_OFF);
    } else {
        lv_bar_set_start_value(obj, 0, LV_ANIM_OFF);
        lv_bar_set_value(obj, val, LV_ANIM_OFF);
    }
}
