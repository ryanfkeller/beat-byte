#pragma once

#include "lvgl.h"

struct Screens {
    lv_obj_t *menu;
    lv_obj_t *player;
};

extern struct Screens screens;

void create_screens();