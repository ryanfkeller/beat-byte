#include "screens.h"
#include "lvgl/lvgl.h"
#include "bt_device_list.h"


struct Screens screens;

enum {
    LV_MENU_ITEM_BUILDER_VARIANT_1,
    LV_MENU_ITEM_BUILDER_VARIANT_2
};
typedef uint8_t lv_menu_builder_variant_t;

static void switch_handler(lv_event_t * e);
static lv_obj_t *create_text(lv_obj_t *parent, const char *icon, const char *txt, lv_menu_builder_variant_t builder_variant);
static lv_obj_t *create_slider(lv_obj_t *parent, const char *icon, const char *txt, int32_t min, int32_t max, int32_t val);
static lv_obj_t *create_switch(lv_obj_t *parent, const char *icon, const char *txt, bool chk);


static lv_obj_t *create_subpage(lv_obj_t *menu, lv_obj_t *parent, lv_obj_t *child, const char* txt);
static lv_obj_t *create_menu_label(lv_obj_t *page, const char *txt);
static lv_obj_t *create_menu_switch(lv_obj_t * parent, const char * txt, bool chk);

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
    create_switch(section, NULL, "Enable Bluetooth", false);
    lv_menu_separator_create(bluetooth_page);
    create_text(bluetooth_page, NULL, "My Devices", LV_MENU_ITEM_BUILDER_VARIANT_1);
    section = lv_menu_section_create(bluetooth_page);
    create_text(section, NULL, "Test1", LV_MENU_ITEM_BUILDER_VARIANT_1);
    create_text(section, NULL, "Test2", LV_MENU_ITEM_BUILDER_VARIANT_1);
    lv_menu_separator_create(bluetooth_page);
    create_text(bluetooth_page, NULL, "Other Devices", LV_MENU_ITEM_BUILDER_VARIANT_1);
    section = lv_menu_section_create(bluetooth_page);
    create_text(section, NULL, "Test3", LV_MENU_ITEM_BUILDER_VARIANT_1);
    create_text(section, NULL, "Test4", LV_MENU_ITEM_BUILDER_VARIANT_1);




    

    // // Create our initial list of paired bluetooth devices
    // create_menu_label(bluetooth_page, "Connected Devices");
    // lv_obj_t *section = lv_menu_section_create(bluetooth_page);
    // lv_bt_device_list_create(section);

    lv_obj_t *root_page = lv_menu_page_create(menu, NULL);
    lv_obj_set_style_pad_hor(root_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(menu), 0), 0);
    section = lv_menu_section_create(root_page);
    cont = create_text(section, LV_SYMBOL_SETTINGS, "Bluetooth", LV_MENU_ITEM_BUILDER_VARIANT_1);
    lv_menu_set_load_page_event(menu, cont, bluetooth_page);

    // cont = lv_menu_cont_create(main_page);
    // label = lv_label_create(cont);
    // lv_label_set_text(label, "Files");
    // lv_menu_set_load_page_event(menu, cont, files_page);

    // // Populate the library page
    // cont = lv_menu_cont_create(library_page);
    // label = lv_label_create(cont);
    // lv_label_set_text(label, "Artists");


    lv_menu_set_page(menu, root_page);
}

static lv_obj_t *create_subpage(lv_obj_t *menu, lv_obj_t *parent, lv_obj_t *child, const char *txt) {
    lv_obj_t *obj = create_menu_label(parent, txt);
    lv_menu_set_load_page_event(menu, obj, child);
    return obj;
}

static lv_obj_t *create_menu_label(lv_obj_t *page, const char *txt) {
    lv_obj_t *obj = lv_menu_cont_create(page);
    lv_obj_t *label = lv_label_create(obj);
    lv_label_set_text(label, txt);
    return obj;
}

static lv_obj_t *create_menu_switch(lv_obj_t * parent, const char * txt, bool chk)
{
    lv_obj_t *obj = create_menu_label(parent, txt);
    lv_obj_t *sw = lv_switch_create(obj);
    lv_obj_add_state(sw, chk ? LV_STATE_CHECKED : LV_STATE_DEFAULT);
    return obj;
}

void create_screens() {
    lv_disp_t *disp = lv_disp_get_default();
    create_menu_screen();
}


// Ripped straight from an LVGL example
static lv_obj_t *create_text(lv_obj_t * parent, const char * icon, const char * txt,
                                        lv_menu_builder_variant_t builder_variant)
{
    lv_obj_t *obj = lv_menu_cont_create(parent);

    lv_obj_t *img = NULL;
    lv_obj_t *label = NULL;

    if(icon) {
        img = lv_img_create(obj);
        lv_img_set_src(img, icon);
    }

    if(txt) {
        label = lv_label_create(obj);
        lv_label_set_text(label, txt);
        lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL_CIRCULAR);
        lv_obj_set_flex_grow(label, 1);
    }

    if(builder_variant == LV_MENU_ITEM_BUILDER_VARIANT_2 && icon && txt) {
        lv_obj_add_flag(img, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);
        lv_obj_swap(img, label);
    }

    return obj;
}

static lv_obj_t *create_slider(lv_obj_t * parent, const char * icon, const char * txt, int32_t min, int32_t max, int32_t val)
{
    lv_obj_t *obj = create_text(parent, icon, txt, LV_MENU_ITEM_BUILDER_VARIANT_2);

    lv_obj_t *slider = lv_slider_create(obj);
    lv_obj_set_flex_grow(slider, 1);
    lv_slider_set_range(slider, min, max);
    lv_slider_set_value(slider, val, LV_ANIM_OFF);

    if(icon == NULL) {
        lv_obj_add_flag(slider, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);
    }

    return obj;
}

static lv_obj_t *create_switch(lv_obj_t *parent, const char *icon, const char *txt, bool chk)
{
    lv_obj_t *obj = create_text(parent, icon, txt, LV_MENU_ITEM_BUILDER_VARIANT_1);

    lv_obj_t *sw = lv_switch_create(obj);
    lv_obj_add_state(sw, chk ? LV_STATE_CHECKED : 0);

    return obj;
}