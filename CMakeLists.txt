
cmake_minimum_required(VERSION 3.13...3.27)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Release)

# Selecting the target board (pico, pico_w, pico2, pico2_w)
# Either run the following command while configuring the build folder
# cmake -DPICO_BOARD=pico2 ..
# Or uncomment the following line 
set(PICO_BOARD pico2)

# Modify the following macros based on 
# 1- the resolution of your screen
# 2- the fixed-point number type you choose (Q16_16, Q24_8, or Q8_24)
# 3- the colour format you prefer (RGB332 or RGB565)
# 4- the depth precision you need (DEPTH_8BIT or DEPTH_16BIT)
add_compile_definitions(
    SCREEN_WIDTH=240
    SCREEN_HEIGHT=240

    Q16_16
    RGB332
    DEPTH_8BIT
)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(pico-engine
    VERSION 0.1.0
    LANGUAGES C CXX ASM
    DESCRIPTION "A Graphics Engine for Raspberry Pi Pico"
)

pico_sdk_init()

add_subdirectory(libs/qglm)

add_executable(${PROJECT_NAME} 
    src/main.c
)

target_link_libraries(${PROJECT_NAME} 
    PRIVATE
    pico_stdlib
    qglm
)

target_compile_options(${PROJECT_NAME} 
    PRIVATE
    -Wall
    -Wextra
    -Wshadow
)

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

pico_add_extra_outputs(${PROJECT_NAME})

