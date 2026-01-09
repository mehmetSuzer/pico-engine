
file(GLOB FILES *.c *.h)
add_library(graphics ${FILES})

target_link_libraries(graphics PUBLIC
    pgl
)

target_include_directories(graphics PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

