
#ifndef PICO_ENGINE_GRAPHICS_MODEL_H
#define PICO_ENGINE_GRAPHICS_MODEL_H

#include "mesh.h"
#include "common/components.h"

typedef struct model
{
    transform_component_t transform;
    mesh_t mesh;
} model_t;

void model_draw(const model_t* model);

#endif // PICO_ENGINE_GRAPHICS_MODEL_H
