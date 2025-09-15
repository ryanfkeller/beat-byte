cmake_minimum_required(VERSION 3.16)
project(beat-byte-sim C CXX)

# Set C and C++ standards
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the output path for the executable
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

# Find and include SDL2 library
find_package(SDL2 REQUIRED)

# Add LVGL subdirectory
add_subdirectory(lvgl)
target_include_directories(lvgl PUBLIC ${SDL2_INCLUDE_DIRS})

# Add UI sources
file(GLOB UI_SOURCES "${PROJECT_SOURCE_DIR}/ui/*.c")

# Set main sources and libs (TODO: Update this to my stuff)
set(MAIN_SOURCES src/hal/hal.c src/main.c ${UI_SOURCES})
set(MAIN_LIBS lvgl lvgl::examples lvgl::demos lvgl::thorvg ${SDL2_LIBRARIES})

# Set the executable
add_executable(main ${MAIN_SOURCES})
target_compile_definitions(main PRIVATE LV_CONF_INCLUDE_SIMPLE _DEFAULT_SOURCE)
target_link_libraries(main ${MAIN_LIBS})

# Set compilation flags