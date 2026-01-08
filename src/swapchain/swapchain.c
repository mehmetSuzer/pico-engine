
#include "swapchain.h"

#define SWAPCHAIN_BUFFER_COUNT 2
#define SWAPCHAIN_BUFFER_MASK  (SWAPCHAIN_BUFFER_COUNT - 1)

typedef struct
{
    framebuffer_t buffers[SWAPCHAIN_BUFFER_COUNT];
    uint32_t draw_index;
    volatile bool draw_buffer_available;
    volatile bool swap_requested;
} swapchain_t;

static swapchain_t swapchain = {
    .buffers = {{
        .colours = {{COLOUR_BLACK}},
        .depths = {{DEPTH_FURTHEST}},
    }},
    .draw_index = 0,
    .draw_buffer_available = true,
    .swap_requested = false,
};

framebuffer_t* swapchain_acquire_draw_image()
{
    if (!swapchain.draw_buffer_available)
        return NULL;

    swapchain.draw_buffer_available = false;
    return (framebuffer_t*)&swapchain.buffers[swapchain.draw_index];
}

const framebuffer_t* swapchain_acquire_display_image()
{
    const uint32_t display_index = (swapchain.draw_index + 1) & SWAPCHAIN_BUFFER_MASK;
    return (const framebuffer_t*)&swapchain.buffers[display_index];
}

void swapchain_request_swap()
{
    swapchain.swap_requested = true;
}

void swapchain_swap_buffers()
{
    if (!swapchain.swap_requested) return;
    swapchain.draw_index = (swapchain.draw_index + 1) & SWAPCHAIN_BUFFER_MASK;
    swapchain.swap_requested = false;
    swapchain.draw_buffer_available = true;
}

