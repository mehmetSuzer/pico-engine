
file(GLOB FILES *.c *.h)
add_library(mesh ${FILES})

target_link_libraries(mesh PUBLIC
    hardware_interp
    common
)

target_include_directories(mesh PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

