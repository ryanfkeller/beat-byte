idf_component_register(
    SRCS main.c
    PRIV_REQUIRES ui
    REQUIRES freertos lvgl esp_timer esp_lcd)