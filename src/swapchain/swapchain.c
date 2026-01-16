
#include "swapchain.h"

#define SWAPCHAIN_IMAGE_COUNT 2

typedef struct
{
    swapchain_image_t images[SWAPCHAIN_IMAGE_COUNT];
    uint32_t draw_index;
    uint32_t display_index;
} swapchain_t;

static swapchain_t swapchain = {
    .images = {{
        .colours = {{COLOUR_BLACK}},
    }},
    .draw_index = 1,
    .display_index = 0,
};

swapchain_image_t* swapchain_request_draw_image()
{
    return (swapchain.draw_index != swapchain.display_index) ? &swapchain.images[swapchain.draw_index] : NULL;
}

const swapchain_image_t* swapchain_request_display_image()
{
    const uint32_t next_index = (swapchain.display_index + 1) % SWAPCHAIN_IMAGE_COUNT;
    if (next_index != swapchain.draw_index)
        swapchain.display_index = next_index;
    return &swapchain.images[swapchain.display_index];
}

void swapchain_swap_images()
{
    swapchain.draw_index = (swapchain.draw_index + 1) % SWAPCHAIN_IMAGE_COUNT;
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            
