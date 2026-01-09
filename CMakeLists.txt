
cmake_minimum_required(VERSION 3.13...3.27)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE "Release")

# Selecting the target board (pico, pico_w, pico2, pico2_w)
# Either run the following command while configuring the build folder
# cmake -DPICO_BOARD=pico2 ..
# Or uncomment the following line 
set(PICO_BOARD pico2)

# Modify the following macros based on 
# 1- Set the clock frequency of the Raspberry Pi Pico
# 2- Set the resolution of your screen
# 3- Set the fixed-point number (Q16_16, Q24_8, or Q8_24)
# 4- Set the colour format you prefer (RGB332 or RGB565)
# 5- Set the depth precision you need (DEPTH_8BIT or DEPTH_16BIT)
add_compile_definitions(
    CLOCK_FREQUENCY_KHZ=380000
    SCREEN_WIDTH=240
    SCREEN_HEIGHT=240
    Q16_16
    RGB565
    DEPTH_8BIT
)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(pico-engine
    VERSION 0.1.0
    LANGUAGES C CXX ASM
    DESCRIPTION "A Graphics Engine for Raspberry Pi Pico"
)

pico_sdk_init()

add_subdirectory(src/graphics)
add_subdirectory(src/pgl)
add_subdirectory(src/device)
add_subdirectory(src/swapchain)
add_subdirectory(src/common)
add_subdirectory(src/colour)
add_subdirectory(libs/qglm)

add_executable(${PROJECT_NAME} 
    src/main.c
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    graphics
    pgl
    swapchain
    device
    common
    colour
)

target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall
    -Wextra
    -Wshadow
)

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

pico_add_extra_outputs(${PROJECT_NAME})

