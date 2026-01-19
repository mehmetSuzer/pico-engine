
file(GLOB FILES *.c *.h)
add_library(models ${FILES})

target_link_libraries(models PUBLIC
    graphics
)

target_include_directories(models PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

