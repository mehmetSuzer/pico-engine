
#ifndef PICO_ENGINE_GRAPHICS_TEXTURE_H
#define PICO_ENGINE_GRAPHICS_TEXTURE_H

#include "colour/colour.h"

typedef struct
{
    const colour_t* texels;
    uint16_t width_bits;
    uint16_t height_bits;
} texture_t;

#endif // PICO_ENGINE_GRAPHICS_TEXTURE_H
