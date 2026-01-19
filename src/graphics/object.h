
#ifndef PICO_ENGINE_GRAPHICS_OBJECT_H
#define PICO_ENGINE_GRAPHICS_OBJECT_H

#include "model.h"

typedef struct
{
    transform_component_t transform;
    model_t model;
} object_t;

#endif // PICO_ENGINE_GRAPHICS_OBJECT_H
