#include "ui.h"
#include "lvgl.h"

#include "screens.h"

void create_ui(void) {
    create_screens();
    lv_screen_load(screens.menu);
}