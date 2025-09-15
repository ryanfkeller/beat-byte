#include "bt_device_list.h"

#include "lvgl/lvgl.h"

/**********************
 *  STATIC PROTOTYPES
 **********************/
static lv_obj_t * add_list_button(lv_obj_t * parent, uint32_t device_id);
static void btn_click_event_cb(lv_event_t * e);
static void list_delete_event_cb(lv_event_t * e);
static const char *get_bt_device(uint32_t id);

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_obj_t *list;
static const lv_font_t *font_small;
static const lv_font_t *font_medium;
static lv_style_t style_scrollbar;
static lv_style_t style_btn;
static lv_style_t style_button_pr;
static lv_style_t style_button_chk;
static lv_style_t style_button_dis;
static lv_style_t style_title;
static lv_style_t style_artist;
static lv_style_t style_time;


static const char *bt_devices[] = {
    "Ryan's Headphones",
    "Ryan's Earbuds",
    "Sedona's Smart-Bone"
};

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

 lv_obj_t *lv_bt_device_list_create(lv_obj_t *parent) {
    font_small = &lv_font_montserrat_14;
    font_medium = &lv_font_montserrat_22;
    uint32_t id = 0;
    while (true)
    {
        const char *bt_device = get_bt_device(id);
        if (!bt_device) break;
        lv_obj_t *label = lv_label_create(parent);
        lv_label_set_text(label, bt_device);
        id++;
    }
 }

 static const char *get_bt_device(uint32_t id){
    if (id >= sizeof(bt_devices)/sizeof(bt_devices[0])) return 0;
    return bt_devices[id];
 }