#include "screens.h"
#include "lvgl.h"
#include "bluetooth.h"
#include "esp_log.h"

#define TAG "screens"


struct Screens screens;


static lv_obj_t *create_menu_switch(lv_obj_t *page, const char *txt, void (*cb_func)(lv_event_t*), bool chk);
static lv_obj_t *create_menu_item(lv_obj_t *page, const char *icon, const char *txt, bool selectable);

static lv_group_t *group;
static lv_display_t *disp;

static void bt_switch_event_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target_obj(e);
    
    if(code == LV_EVENT_VALUE_CHANGED) {
        // LV_UNUSED(obj);
        // ESP_LOGI(TAG, "State: %s\n", lv_obj_has_state(obj, LV_STATE_CHECKED) ? "On" : "Off");
        if (lv_obj_has_state(obj, LV_STATE_CHECKED)) {
        //bt_init();
            ESP_LOGI(TAG, "Enabling bluetooth");
            bt_enable(true);
        }
        else {
            ESP_LOGI(TAG, "Disabling bluetooth");
            bt_enable(false);
        }
    }
    
}


static void create_menu_screen() {
    screens.menu = lv_obj_create(NULL);
    lv_obj_set_size(screens.menu, 240, 320);

    lv_obj_t *menu = lv_menu_create(screens.menu);
    lv_color_t bg_color = lv_obj_get_style_bg_color(menu, 0);
    if(lv_color_brightness(bg_color) > 127) {
        lv_obj_set_style_bg_color(menu, lv_color_darken(lv_obj_get_style_bg_color(menu, 0), 10), 0);
    }else{
        lv_obj_set_style_bg_color(menu, lv_color_darken(lv_obj_get_style_bg_color(menu, 0), 50), 0);
    }
    lv_obj_set_size(menu, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL));
    lv_obj_center(menu);

    lv_obj_t *cont;
    lv_obj_t *section;

    // Create sub-pages
    lv_obj_t *bluetooth_page = lv_menu_page_create(menu, "Bluetooth");
    lv_obj_set_style_pad_hor(bluetooth_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(menu), 0),0);
    section = lv_menu_section_create(bluetooth_page);
    create_menu_switch(section, "Enable Bluetooth", bt_switch_event_handler, false);
    lv_menu_separator_create(bluetooth_page);
    create_menu_item(bluetooth_page, NULL, "My Devices", false);
    section = lv_menu_section_create(bluetooth_page);

    create_menu_item(section, NULL, "Test1", true);
    create_menu_item(section, NULL, "Test2", true);

    lv_menu_separator_create(bluetooth_page);

    create_menu_item(bluetooth_page, NULL, "Other Devices", false);
    section = lv_menu_section_create(bluetooth_page);
    create_menu_item(section, NULL, "Test3", true);
    create_menu_item(section, NULL, "Test4", true);


    lv_obj_t *root_page = lv_menu_page_create(menu, NULL);
    lv_obj_set_style_pad_hor(root_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(menu), 0), 0);
    section = lv_menu_section_create(root_page);
    cont = create_menu_item(section, LV_SYMBOL_SETTINGS, "Bluetooth", true);
    lv_menu_set_load_page_event(menu, cont, bluetooth_page);


    lv_menu_set_page(menu, root_page);
}


void create_screens() {
    group = lv_group_get_default();
    disp = lv_disp_get_default();
    create_menu_screen();
}

static lv_obj_t *create_menu_item(lv_obj_t *page, const char *icon, const char *txt, bool selectable) {
    lv_obj_t *obj = lv_menu_cont_create(page);
    lv_obj_t *img = NULL;
    lv_obj_t *label = NULL;

    if (icon) {
        img = lv_img_create(obj);
        lv_img_set_src(img, icon);
    }

    if (txt) {
        label = lv_label_create(obj);
        lv_label_set_text(label, txt);
        lv_label_set_long_mode(label, LV_LABEL_LONG_MODE_SCROLL_CIRCULAR);
        lv_obj_set_flex_grow(label, 1);
    }

    if (selectable) {
        lv_group_add_obj(group, obj);
    }

    return obj;
}

static lv_obj_t *create_menu_switch(lv_obj_t *page, const char *txt, void (*cb_func)(lv_event_t*), bool default_checked)
{
    lv_obj_t *obj = create_menu_item(page, NULL, txt, false);
    lv_obj_t *sw = lv_switch_create(obj);
    lv_obj_add_state(sw, default_checked ? LV_STATE_CHECKED : LV_STATE_DEFAULT);
    lv_group_add_obj(group, sw);

    if (cb_func) {
        lv_obj_add_event_cb(sw, cb_func, LV_EVENT_ALL, NULL);
    }
    return obj;
}