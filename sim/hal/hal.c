#include "hal.h"

#include "lvgl.h"

lv_display_t * sdl_hal_init(int32_t w, int32_t h) {
    lv_group_set_default(lv_group_create());

    lv_display_t *disp = lv_sdl_window_create(w, h);

    /* Set up the mouse as an input device.
        TODO: update this to just use click/scrollwheel */
    lv_indev_t *mouse = lv_sdl_mouse_create();
    lv_indev_set_group(mouse, lv_group_get_default());
    lv_indev_set_display(mouse, disp);

    // LV_IMAGE_DECLARE(mouse_cursor_icon);
    // lv_obj_t *cursor_obj = lv_image_create(lv_screen_active());
    // lv_image_set_src(cursor_obj, &mouse_cursor_icon);
    // lv_indev_set_cursor(mouse, cursor_obj);

    lv_indev_t *mousewheel = lv_sdl_mousewheel_create();
    lv_indev_set_display(mousewheel, disp);
    lv_indev_set_group(mousewheel, lv_group_get_default());

    /* Set up the keyboard as an input device */
    lv_indev_t *kb = lv_sdl_keyboard_create();
    lv_indev_set_display(kb, disp);
    lv_indev_set_group(kb, lv_group_get_default());

    return disp;
}