
#include "uart.h"

#include "esp_log.h"
#include "driver/uart.h"

#include "system_config.h"

/*********************************************************************
 * STATIC VARS
 *********************************************************************/

static const char *TAG = "UART";

/*********************************************************************
 * PUBLIC FUNCTIONS
 *********************************************************************/

/* Initialize the UART interface */
static void init() {
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
    ESP_LOGI(TAG, "Uart initialized");
}

/*********************************************************************
 * PUBLIC INTERFACE
 *********************************************************************/

 const struct Uart uart = {
    .init = init
 };
