#include "lcd.h"

#include "esp_log.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "driver/gpio.h"

#include "system_config.h"

/*********************************************************************
 * STATIC VARS
 *********************************************************************/

static const char *TAG = "LCD";
static esp_lcd_panel_handle_t handle;
static esp_lcd_panel_io_handle_t io_handle;

/*********************************************************************
 * PRIVATE FUNCTIONS
 *********************************************************************/

/* Configure the GPIO for the LCD backlight control */
static void gpio_init() {
    gpio_config_t bk_gpio_config = {
        .pin_bit_mask = 1ULL << LCD_BK_GPIO_NUM,
        .mode = GPIO_MODE_OUTPUT,
    };
    ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));
    ESP_LOGI(TAG, "GPIO initialized");
}

/* Configure the SPI bus for the LCD interface */
static void spi_init() {
    spi_bus_config_t buscfg = {
        .mosi_io_num = LCD_SPI_MOSI_GPIO_NUM,
        .miso_io_num = -1,
        .sclk_io_num = LCD_SPI_SCLK_GPIO_NUM,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = LCD_H_RES*LCD_SPI_MAX_LINES_PER_TXN*sizeof(uint16_t), //transfer 80 lines of pixels (RGB565) at most in one SPI txn
    };
    ESP_ERROR_CHECK(spi_bus_initialize(LCD_SPI_HOST, &buscfg, 1));
    ESP_LOGI(TAG, "SPI initialized");
}

/* Configure the LCD panel with the initialized drivers */
void panel_init() {
    ESP_LOGI(TAG, "Installing LCD panel IO");
    esp_lcd_panel_io_spi_config_t io_config = {
        .cs_gpio_num = LCD_SPI_CS_GPIO_NUM,
        .dc_gpio_num = LCD_SPI_DC_GPIO_NUM,
        .spi_mode = LCD_SPI_MODE,
        .pclk_hz = LCD_SPI_PCLK_HZ,
        .trans_queue_depth = LCD_SPI_TRANS_QUEUE_DEPTH,
        .lcd_cmd_bits = LCD_SPI_CMD_BITS,
        .lcd_param_bits = LCD_SPI_PARAM_BITS,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_SPI_HOST, &io_config, &io_handle));

    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = LCD_RST_GPIO_NUM,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 16,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_config, &handle));

    ESP_ERROR_CHECK(esp_lcd_panel_reset(handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(handle));
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(handle, true));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(handle, false, false));
}

/*********************************************************************
 * PUBLIC FUNCTIONS
 *********************************************************************/

/* Enable or disable the LCD backlight */
static void enable_backlight(bool enable) {
    int set_level = enable ? LCD_BK_LIGHT_ON_LVL : LCD_BK_LIGHT_OFF_LVL;
    gpio_set_level((gpio_num_t)LCD_BK_GPIO_NUM, set_level);   
}

/* Turn on or off the LCD panel */
static void enable_panel(bool enable) {
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(handle, enable));  
}

/* Fully initialize the LCD interface in sequence */
static void init() {
    gpio_init();
    spi_init();

    enable_backlight(true);
    panel_init();
    enable_panel(true);
}

/*********************************************************************
 * PUBLIC INTERFACE
 *********************************************************************/

const struct Lcd lcd = {
    .init = init,
    .enable_backlight = enable_backlight,
    .enable_panel = enable_panel,
    .handle = &handle,
    .io_handle = &io_handle
};
