#pragma once

#include "esp_lcd_panel_io.h"

struct Lcd {
    void (*init)(void);
    void (*enable_backlight)(bool);
    void (*enable_panel)(bool);
    esp_lcd_panel_handle_t *handle;
    esp_lcd_panel_io_handle_t *io_handle;
};

extern const struct Lcd lcd;
 
