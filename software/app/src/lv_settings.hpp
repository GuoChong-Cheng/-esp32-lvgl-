#pragma once

#include "lvgl.h"
#include "lib/data_structure/linklist.hpp"
#include "bsp/board_conf.h"

extern lv_group_t* ec_grp;

class lv_settings {  // imitate imgui
private:
    LinkList<lv_obj_t*> m_levels;     // pages
    LinkList<lv_obj_t*> m_sublevels;  // sections

    lv_obj_t* m_menu;
    lv_obj_t* m_curr_page;
    lv_obj_t* m_curr_sect;
    lv_obj_t* m_curr_cont;
    lv_obj_t* m_last_obj;

    lv_coord_t m_offset;

    bool m_sameline;

    lv_obj_t* obj_parent(void);  // current object's parent

public:
    lv_obj_t* last_obj(void) { return m_last_obj; }

    lv_settings(lv_obj_t* parent = nullptr, lv_event_cb_t cbk_page_changed = nullptr, void* user_data = nullptr);

    lv_obj_t* menu(void) const { return m_menu; }

    lv_obj_t* start_section(void);
    void      stop_section(void);

    lv_obj_t* add_page(const char* icon, const char* text = nullptr);
    void      end_page(void);

    void add_sameline(void);

    lv_obj_t* add_separator(void);
    lv_obj_t* add_label(const char* icon, const char* text, uint8_t ret = 2);  // 0:img,1:lbl,2:cont
    lv_obj_t* add_silder(int32_t val = 0, int32_t min = 0, int32_t max = 100);
    lv_obj_t* add_offset_bar(int32_t val = 0, int32_t min = 0, int32_t max = 100);  // 位置偏移条
    lv_obj_t* add_switch(bool chk = false);
    lv_obj_t* add_lineedit(const char* text = nullptr);
    lv_obj_t* add_dropdown(const char* items = nullptr);
    lv_obj_t* add_chart(void);

    void attach(lv_event_cb_t callback, lv_event_code_t event, void* user_data = NULL);
};

void lv_offset_bar_set_value(lv_obj_t* obj, int32_t val);
