
#ifndef PICO_ENGINE_PGL_PGL_H
#define PICO_ENGINE_PGL_PGL_H

#include <hardware/interp.h>

#include "common/macros.h"
#include "common/fixed_point.h"
#include "swapchain/swapchain.h"

typedef struct
{
    Q_VEC3 position;
    Q_VEC2 tex_coord; // Texture coordinates must have non-negative values
} pgl_vertex_t;

typedef enum
{
    PGL_COLOUR_BUFFER_BIT = 0b01,
    PGL_DEPTH_BUFFER_BIT  = 0b10,
} pgl_framebuffer_bit_t;

void pgl_model(Q_VEC3 position, Q_QUAT rotation, Q_VEC3 scale);
void pgl_view(Q_VEC3 eye, Q_VEC3 backward, Q_VEC3 up);
void pgl_projection(Q_TYPE fovw, Q_TYPE near, Q_TYPE far);
void pgl_viewport(int32_t x, int32_t y, uint32_t width, uint32_t height);

void pgl_clear_colour(colour_t colour);
void pgl_clear_depth(depth_t depth);
void pgl_clear(pgl_framebuffer_bit_t bits);

// Returns true when a frame is received from the swapchain
bool pgl_request_frame();
void pgl_bind_texture(const colour_t* texels, uint width_bits, uint height_bits);
void pgl_draw(const pgl_vertex_t* vertices, const uint16_t* indices, uint16_t index_count);

#endif // PICO_ENGINE_PGL_PGL_H

