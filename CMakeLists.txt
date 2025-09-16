cmake_minimum_required(VERSION 3.16)

if(NOT ESP_PLATFORM)
    project(beat-byte C CXX)
    message(STATUS "Configuring for simulation build")
    set(CMAKE_C_STANDARD 99)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
    set(WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
    find_package(SDL2 REQUIRED)

    set(COMPONENTS_DIR "${CMAKE_SOURCE_DIR}/components")

    set(LV_BUILD_CONF_PATH "${CMAKE_SOURCE_DIR}/sim/lv_conf.h")
    add_subdirectory(${COMPONENTS_DIR}/lvgl)
    target_include_directories(lvgl PUBLIC ${SDL2_INCLUDE_DIRS})

    file(GLOB UI_SOURCES "${COMPONENTS_DIR}/ui/*.c")

    # Set main sources and libs (TODO: Update this to my stuff)
    set(MAIN_SIM_SOURCES sim/hal/hal.c sim/main_sim.c ${UI_SOURCES})
    set(MAIN_SIM_LIBS lvgl lvgl::thorvg ${SDL2_LIBRARIES})


    add_executable(main_sim ${MAIN_SIM_SOURCES})
    target_compile_definitions(main_sim PRIVATE LV_CONF_INCLUDE_SIMPLE _DEFAULT_SOURCE)
    target_link_libraries(main_sim ${MAIN_SIM_LIBS})
    target_include_directories(main_sim PRIVATE ${COMPONENTS_DIR}/ui)

else()
    message(STATUS "Configuring for ESP-IDF build")
    include($ENV{IDF_PATH}/tools/cmake/project.cmake)
    project(beat-byte)
endif()