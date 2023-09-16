#pragma once

#include "SD.h"
#include "bsp/board_conf.h"

extern bool g_sdcrad_ok;

bool sd_init(void);
void sd_list(const char* path = "/");

#if 0

void setup{
    if(sd_init()){
        sd_list();
    }
}

#endif