
file(GLOB FILES *.c *.h)
add_library(pgl ${FILES})

target_link_libraries(pgl PUBLIC
    pico_stdlib
    hardware_interp
    common
    swapchain
)

target_include_directories(pgl PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

