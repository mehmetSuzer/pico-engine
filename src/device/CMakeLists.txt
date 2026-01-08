
file(GLOB FILES *.c *.h)
add_library(device ${FILES})

target_link_libraries(device PUBLIC
    pico_stdlib
    hardware_spi
    hardware_dma
    hardware_pwm
    swapchain
)

target_include_directories(device PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

