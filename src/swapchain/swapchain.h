
#ifndef PICO_ENGINE_SWAPCHAIN_SWAPCHAIN_H
#define PICO_ENGINE_SWAPCHAIN_SWAPCHAIN_H

#include <stddef.h>
#include <stdbool.h>
#include "common/depth.h"
#include "colour/colour.h"

#ifndef SCREEN_HEIGHT
    #error "SCREEN_HEIGHT is not defined!"
#endif

#ifndef SCREEN_WIDTH
    #error "SCREEN_WIDTH is not defined!"
#endif

typedef struct
{
    colour_t colours[SCREEN_HEIGHT][SCREEN_WIDTH];
    depth_t   depths[SCREEN_HEIGHT][SCREEN_WIDTH];
} framebuffer_t;

framebuffer_t* swapchain_acquire_draw_image();
const framebuffer_t* swapchain_acquire_display_image();

void swapchain_request_swap();
void swapchain_swap_buffers();

#endif // PICO_ENGINE_SWAPCHAIN_SWAPCHAIN_H
