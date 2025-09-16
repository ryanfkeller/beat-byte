#include <stdint.h>
#include <unistd.h>

#include "lvgl.h"
#include "hal/hal.h"
#include "ui.h"


int main(int argc, char **argv) {
    (void) argc;
    (void) argv;

    lv_init();

    sdl_hal_init(240,320);

    create_ui();

    while(1) {
        /* Periodically call the lv_task handler.
         * It could be done in a timer interrupt or an OS task too.*/
        uint32_t sleep_time_ms = lv_timer_handler();
        if(sleep_time_ms == LV_NO_TIMER_READY){
	        sleep_time_ms =  LV_DEF_REFR_PERIOD;
        }
        usleep(sleep_time_ms*1000);
    }
}