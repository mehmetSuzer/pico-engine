
#ifndef PICO_ENGINE_GRAPHICS_CAMERA_H
#define PICO_ENGINE_GRAPHICS_CAMERA_H

#include "common/components.h"
#include "pgl/pgl.h"

typedef struct camera
{
    transform_component_t transform;
    camera_component_t camera;
} camera_t;

void camera_set_view_proj(const camera_t* camera);

#endif // PICO_ENGINE_GRAPHICS_CAMERA_H
