
file(GLOB FILES *.c *.h)
add_library(swapchain ${FILES})

target_link_libraries(swapchain PUBLIC
    colour
)

target_include_directories(swapchain PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

