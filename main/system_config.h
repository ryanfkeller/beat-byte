#pragma once

/*********************************************************************
 * UART Settings
 *********************************************************************/

#define UART_PORT_NUM UART_NUM_0

#define UART_BAUD_RATE 115200
#define UART_DATA_BITS UART_DATA_8_BITS
#define UART_PARITY UART_PARITY_DISABLE
#define UART_STOP_BITS UART_STOP_BITS_1
#define UART_FLOW_CTRL UART_HW_FLOWCTRL_DISABLE
#define UART_SOURCE_CLK UART_SCLK_DEFAULT

#define UART_RX_BUF_SIZE 256
#define UART_TX_BUF_SIZE 0

/*********************************************************************
 * LCD Settings
 *********************************************************************/
#define LCD_SPI_HOST SPI3_HOST

#define LCD_SPI_CS_GPIO_NUM 5
#define LCD_SPI_DC_GPIO_NUM 17
#define LCD_SPI_SCLK_GPIO_NUM 18
#define LCD_SPI_MOSI_GPIO_NUM 19

#define LCD_SPI_MODE 0
#define LCD_SPI_PCLK_HZ 20000000
#define LCD_SPI_TRANS_QUEUE_DEPTH 10
#define LCD_SPI_CMD_BITS 8
#define LCD_SPI_PARAM_BITS 8
#define LCD_SPI_MAX_LINES_PER_TXN 80

#define LCD_BK_GPIO_NUM 4
#define LCD_BK_LIGHT_ON_LVL 1
#define LCD_BK_LIGHT_OFF_LVL !LCD_BK_LIGHT_ON_LVL

#define LCD_RST_GPIO_NUM 16

#define LCD_H_RES 240
#define LCD_V_RES 320

/*********************************************************************
 * LVGL Settings
 *********************************************************************/

#define LVGL_DRAW_BUF_LINES 32
#define LVGL_TICK_PERIOD_MS 2

#define LVGL_TASK_MAX_DELAY_MS 500
#define LVGL_TASK_MIN_DELAY_MS 1000 / CONFIG_FREERTOS_HZ

#define LVGL_TASK_STACK_SIZE 10 * 1024

#define LVGL_TASK_PRIORITY 2

/*********************************************************************
 * SD Settings
 *********************************************************************/

#define SD_MAX_CHAR_SIZE 64
#define SD_MOUNT_POINT "/sdcard"

#define SD_SPI_MISO_GPIO_NUM 14
#define SD_SPI_MOSI_GPIO_NUM 27
#define SD_SPI_CLK_GPIO_NUM 26
#define SD_SPI_CS_GPIO_NUM 25