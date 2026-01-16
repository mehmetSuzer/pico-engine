
#ifndef PICO_ENGINE_SWAPCHAIN_SWAPCHAIN_H
#define PICO_ENGINE_SWAPCHAIN_SWAPCHAIN_H

#include <stddef.h>
#include <stdbool.h>
#include "colour/colour.h"

#ifndef SCREEN_HEIGHT
    #error "SCREEN_HEIGHT is not defined!"
#endif

#ifndef SCREEN_WIDTH
    #error "SCREEN_WIDTH is not defined!"
#endif

#if (SCREEN_HEIGHT * SCREEN_WIDTH) % 4 != 0
    #error "SCREEN_HEIGHT * SCREEN_WIDTH is not divislbe by 4. The number of pixels must be a multiple of 4!"
#endif

typedef struct
{
    colour_t colours[SCREEN_HEIGHT][SCREEN_WIDTH];
} swapchain_image_t;

swapchain_image_t* swapchain_request_draw_image();
const swapchain_image_t* swapchain_request_display_image();
void swapchain_swap_images();

#endif // PICO_ENGINE_SWAPCHAIN_SWAPCHAIN_H
