#include "gui.h"

#include <unistd.h>
#include <sys/lock.h>
#include <sys/param.h>

#include "lcd.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "uart.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "lvgl.h"
#include "driver/uart.h"
#include "system_config.h"

/*********************************************************************
 * STATIC VARS
 *********************************************************************/

static const char *TAG = "GUI";
static _lock_t lvgl_api_lock;

/*********************************************************************
 * PRIVATE FUNCTIONS
 *********************************************************************/

static bool notify_lvgl_flush_ready(
    esp_lcd_panel_io_handle_t panel_io,
    esp_lcd_panel_io_event_data_t *edata,
    void *user_ctx)
{
    lv_display_t *disp = (lv_display_t *)user_ctx;
    lv_display_flush_ready(disp);
    return false;
}

static void increase_lvgl_tick(void *arg)
{
    /* Tell LVGL how many milliseconds has elapsed */
    lv_tick_inc(LVGL_TICK_PERIOD_MS);
}

static void lvgl_port_task(void *arg)
{
    ESP_LOGI(TAG, "Starting LVGL task");
    uint32_t time_till_next_ms = 0;
    while (1) {
        _lock_acquire(&lvgl_api_lock);
        time_till_next_ms = lv_timer_handler();
        _lock_release(&lvgl_api_lock);
        // in case of triggering a WDT
        time_till_next_ms = MAX(time_till_next_ms, LVGL_TASK_MIN_DELAY_MS);
        // in case of lvgl display not ready yet
        time_till_next_ms = MIN(time_till_next_ms, LVGL_TASK_MAX_DELAY_MS);
        usleep(1000* time_till_next_ms);
    }
}

static void lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    // Get the LCD handle and pixel offset
    // esp_lcd_panel_handle_t panel_handle = lv_display_get_user_data(disp);
    int offsetx1 = area->x1;
    int offsetx2 = area->x2; 
    int offsety1 = area->y1;
    int offsety2 = area->y2;

    // because SPI LCD is big-endian, we need to swap the RGB bytes order
    lv_draw_sw_rgb565_swap(px_map, (offsetx2 + 1 - offsetx1) * (offsety2 + 1 - offsety1));

    // copy a buffer's content to a specific area of the display
    esp_lcd_panel_draw_bitmap(*lcd.handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, px_map);
}

static void uart_indev_read_cb(lv_indev_t *indev_driver, lv_indev_data_t *data)
{
    // Lets see if data is available
    size_t len;
    uart_get_buffered_data_len(UART_PORT_NUM, &len);

    if (len > 0) {
        // Great, data is available. Lets read the data and store it
        uint8_t buf;
        int read_len = uart_read_bytes(UART_PORT_NUM, &buf, 1, 10); // read just one byte
    
        if (read_len > 0) {
            // Whew, we read one byte. Lets see what it was and map it to the appropriate
            // LV key for the action
            // Also, if its a valid key, let LVGL know that we pressed a key. 
            switch (buf) {
                case 'w': case 'W':
                    ESP_LOGI(TAG, "w");
                    data->state = LV_INDEV_STATE_PRESSED;
                    data->key = LV_KEY_PREV;
                    break;
                case 's': case 'S':
                    ESP_LOGI(TAG, "s");
                    data->state = LV_INDEV_STATE_PRESSED;
                    data->key = LV_KEY_NEXT;
                    break;
                case 'a': case 'A':
                    ESP_LOGI(TAG, "a");
                    data->state = LV_INDEV_STATE_PRESSED;
                    data->key = LV_KEY_LEFT;
                    break;
                case 'd': case 'D':
                    ESP_LOGI(TAG, "d");
                    data->state = LV_INDEV_STATE_PRESSED;
                    data->key = LV_KEY_RIGHT;
                    break;
                case '\r': case '\n':
                    ESP_LOGI(TAG, "enter");
                    data->state = LV_INDEV_STATE_PRESSED;
                    data->key = LV_KEY_ENTER;
                    break;
                case 27:
                    ESP_LOGI(TAG, "esc");
                    data->state = LV_INDEV_STATE_PRESSED;
                    data->key = LV_KEY_ESC;
                    break;
                default:
                    break;
            }

            // Set flag to tell LVGL there is more to read so
            // we can call this function some more.
            data->continue_reading = len > read_len;
            return;
        }
    }

    // Nothing left to read! Now this function can finally rest.
    data->state = LV_INDEV_STATE_RELEASED;
    return;
}

/*********************************************************************
 * PUBLIC FUNCTIONS
 *********************************************************************/

static void init() {
    lcd.init();
    uart.init();

    ESP_LOGI(TAG, "Initalizing LVGL library");
    lv_init();

    lv_display_t *display = lv_display_create(LCD_H_RES, LCD_V_RES);

    size_t draw_buffer_sz = LCD_H_RES * LVGL_DRAW_BUF_LINES * sizeof(lv_color16_t);
    void *buf1 = spi_bus_dma_memory_alloc(LCD_SPI_HOST, draw_buffer_sz, 0);
    void *buf2 = spi_bus_dma_memory_alloc(LCD_SPI_HOST, draw_buffer_sz, 0);

    lv_display_set_buffers(display, buf1, buf2, draw_buffer_sz, LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_user_data(display, lcd.handle);

    lv_display_set_color_format(display, LV_COLOR_FORMAT_RGB565);
    lv_display_set_flush_cb(display, lvgl_flush_cb);

    ESP_LOGI(TAG, "Installing LVGL tick timer");
    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &increase_lvgl_tick,
        .name = "lvgl_tick"
    };
    esp_timer_handle_t lvgl_tick_timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, LVGL_TICK_PERIOD_MS * 1000));

    ESP_LOGI(TAG, "Register io panel event callback for LVGL flush ready notification");
    const esp_lcd_panel_io_callbacks_t cbs = {
        .on_color_trans_done = notify_lvgl_flush_ready,
    };
    ESP_ERROR_CHECK(esp_lcd_panel_io_register_event_callbacks(*lcd.io_handle, &cbs, display));

    ESP_LOGI(TAG, "Create LVGL task");
    xTaskCreate(lvgl_port_task, "LVGL", LVGL_TASK_STACK_SIZE, NULL, LVGL_TASK_PRIORITY, NULL);

    lv_group_t *group = lv_group_create();
    lv_group_set_default(group);

    lv_indev_t *uart_indev = lv_indev_create();
    lv_indev_set_type(uart_indev, LV_INDEV_TYPE_KEYPAD);
    lv_indev_set_read_cb(uart_indev, uart_indev_read_cb);
    lv_indev_set_group(uart_indev, group);
    lv_indev_enable(uart_indev, true);

    lcd.enable_panel(true);
}

/*********************************************************************
 * PUBLIC INTERFACE
 *********************************************************************/

 const struct Gui gui = {
    .init = init
 };








