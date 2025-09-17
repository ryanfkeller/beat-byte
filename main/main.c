/*********************************************************************
 * Dependency Includes
 *********************************************************************/

#include <unistd.h>
#include <sys/lock.h>
#include <sys/param.h>

#include "freertos/FreeRTOS.h"

// ESP tools/device drivers we need
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_lcd_panel_io.h"   
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "driver/gpio.h"
#include "driver/uart.h"

// LVGL dependency
#include "lvgl.h"
#include "ui.h"

/*********************************************************************
 * LCD DEFINES
 *********************************************************************/

#define LCD_HOST                SPI3_HOST
#define LCD_PIXEL_CLK_HZ        (20*1000*1000)
#define LCD_CMD_BITS            8
#define LCD_PARAM_BITS          8
#define LCD_SPI_MODE            0

#define LCD_BK_LIGHT_ON_LVL     1
#define LCD_BK_LIGHT_OFF_LVL    !LCD_BK_LIGHT_ON_LVL

#define LCD_PIN_NUM_SCLK        2
#define LCD_PIN_NUM_MOSI        1
#define LCD_PIN_NUM_DC          41
#define LCD_PIN_NUM_RST         40
#define LCD_PIN_NUM_CS          42
#define LCD_PIN_NUM_BK_LIGHT    39

#define LCD_TRANS_QUEUE_DEPTH   10

#define LCD_MAX_LINES_PER_TXN   80

#define LCD_H_RES               240
#define LCD_V_RES               320

/*********************************************************************
 * LVGL DEFINES
 *********************************************************************/

#define LVGL_DRAW_BUF_LINES     32
#define LVGL_TICK_PERIOD_MS     2

#define LVGL_TASK_MAX_DELAY_MS  500
#define LVGL_TASK_MIN_DELAY_MS  1000 / CONFIG_FREERTOS_HZ

#define LVGL_TASK_STACK_SIZE    (10 * 1024)

#define LVGL_TASK_PRIORITY      2


/*********************************************************************
 * UART DEFINES
 *********************************************************************/

#define UART_PORT_NUM   UART_NUM_0
#define UART_BAUD_RATE  115200
#define UART_DATA_BITS  UART_DATA_8_BITS
#define UART_PARITY     UART_PARITY_DISABLE
#define UART_STOP_BITS  UART_STOP_BITS_1
#define UART_FLOW_CTRL  UART_HW_FLOWCTRL_DISABLE
#define UART_SOURCE_CLK UART_SCLK_DEFAULT

#define UART_RX_BUF_SIZE 256
#define UART_TX_BUF_SIZE 0

/*********************************************************************
 * USB DEFINES
 *********************************************************************/


/*********************************************************************
 * STATIC VARIABLES
 *********************************************************************/

static const char *TAG = "MAIN";
static _lock_t lvgl_api_lock;


/*********************************************************************
 * STATIC FUNCTIONS
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

static void lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    // Get the LCD handle and pixel offset
    esp_lcd_panel_handle_t panel_handle = lv_display_get_user_data(disp);
    int offsetx1 = area->x1;
    int offsetx2 = area->x2; 
    int offsety1 = area->y1;
    int offsety2 = area->y2;

    // because SPI LCD is big-endian, we need to swap the RGB bytes order
    lv_draw_sw_rgb565_swap(px_map, (offsetx2 + 1 - offsetx1) * (offsety2 + 1 - offsety1));

    // copy a buffer's content to a specific area of the display
    esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, px_map);
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
 * TASKS
 *********************************************************************/

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

/*********************************************************************
 * Hardware Setup
 *********************************************************************/

static void hw_init()
{
    // Configure UART
    uart_config_t uart_config = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_BITS,
        .parity = UART_PARITY,
        .stop_bits = UART_STOP_BITS,
        .flow_ctrl = UART_FLOW_CTRL,
        .source_clk = UART_SOURCE_CLK,
    };

    ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT_NUM, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(UART_PORT_NUM, UART_RX_BUF_SIZE, UART_TX_BUF_SIZE, 0, NULL, 0 ));

    ESP_LOGI(TAG, "UART initialized");

    // Configure GPIO for backlight
    gpio_config_t bk_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << LCD_PIN_NUM_BK_LIGHT, // GPIO pin 4
    };
    ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));
    ESP_LOGI(TAG, "GPIO initialized");

    // Configure the SPI bus
    spi_bus_config_t buscfg = {
        .sclk_io_num = LCD_PIN_NUM_SCLK,
        .mosi_io_num = LCD_PIN_NUM_MOSI,
        .miso_io_num = -1,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = LCD_H_RES*LCD_MAX_LINES_PER_TXN*sizeof(uint16_t), //transfer 80 lines of pixels (RGB565) at most in one SPI txn
    };
    ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));
    ESP_LOGI(TAG, "SPI initialized");
}

/*********************************************************************
 * MAIN APPLICATION
 *********************************************************************/

void app_main(void)
{
    ESP_LOGI(TAG, "*** Beat-Byte Main Starting ***");

    ESP_LOGI(TAG, "Initializing Hardware");
    hw_init();

    ESP_LOGI(TAG, "Turn on LCD backlight");
    gpio_set_level(LCD_PIN_NUM_BK_LIGHT, LCD_BK_LIGHT_ON_LVL);

    ESP_LOGI(TAG, "Install LCD panel IO");
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = LCD_PIN_NUM_DC,
        .cs_gpio_num = LCD_PIN_NUM_CS,
        .pclk_hz = LCD_PIXEL_CLK_HZ,
        .lcd_cmd_bits = LCD_CMD_BITS,
        .lcd_param_bits = LCD_PARAM_BITS,
        .spi_mode = LCD_SPI_MODE,
        .trans_queue_depth = LCD_TRANS_QUEUE_DEPTH,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &io_handle));

    esp_lcd_panel_handle_t panel_handle = NULL;
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = LCD_PIN_NUM_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 16,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle));

    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, false, false));

    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, false));

    ESP_LOGI(TAG, "Initalize LVGL library");
    lv_init();
    lv_display_t *display = lv_display_create(LCD_H_RES, LCD_V_RES);
    size_t draw_buffer_sz = LCD_H_RES * LVGL_DRAW_BUF_LINES * sizeof(lv_color16_t);

    void *buf1 = spi_bus_dma_memory_alloc(LCD_HOST, draw_buffer_sz, 0);
    assert(buf1);
    void *buf2 = spi_bus_dma_memory_alloc(LCD_HOST, draw_buffer_sz, 0);
    assert(buf2);

    lv_display_set_buffers(display, buf1, buf2, draw_buffer_sz, LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_user_data(display, panel_handle);

    lv_display_set_color_format(display, LV_COLOR_FORMAT_RGB565);

    lv_display_set_flush_cb(display, lvgl_flush_cb);

    ESP_LOGI(TAG, "Install LVGL tick timer");
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
    ESP_ERROR_CHECK(esp_lcd_panel_io_register_event_callbacks(io_handle, &cbs, display));

    ESP_LOGI(TAG, "Create LVGL task");
    xTaskCreate(lvgl_port_task, "LVGL", LVGL_TASK_STACK_SIZE, NULL, LVGL_TASK_PRIORITY, NULL);

    lv_group_t *group = lv_group_create();
    lv_group_set_default(group);

    lv_indev_t *uart_indev = lv_indev_create();
    lv_indev_set_type(uart_indev, LV_INDEV_TYPE_KEYPAD);
    lv_indev_set_read_cb(uart_indev, uart_indev_read_cb);
    lv_indev_set_group(uart_indev, group);
    lv_indev_enable(uart_indev, true);

    create_ui();
    



    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));
}
