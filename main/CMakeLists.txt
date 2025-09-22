#LVGL sources
set(LVGL_DIR "../components/lvgl")

#Peripheral sources and reqs
set(PERIPHERAL_DIR "./peripherals")
file(GLOB_RECURSE PERIPHERAL_SRCS ${PERIPHERAL_DIR}/*.c )
set(PERIPHERAL_REQS driver esp_lcd fatfs)

#Bt sources and reqs
set(BT_DIR "./bluetooth")
file(GLOB_RECURSE BT_SRCS ${BT_DIR}/*.c )
set(BT_REQS bt)

#GUI sources and reqs
set(GUI_DIR "./gui")
file(GLOB_RECURSE GUI_SRCS ${GUI_DIR}/*.c )
set(GUI_REQS lvgl esp_timer)
message(STATUS ${GUI_SRCS})

idf_component_register(
    SRCS main.c ${PERIPHERAL_SRCS} ${GUI_SRCS} ${BT_SRCS}
    INCLUDE_DIRS . ${PERIPHERAL_DIR} ${GUI_DIR} ${LVGL_DIR} ${BT_DIR}
    REQUIRES freertos bt nvs_flash ${PERIPHERAL_REQS} ${GUI_REQS} ${BT_REQS})

idf_build_set_property(CXX_COMPILE_OPTIONS "-Wno-missing-field-initializers" APPEND)