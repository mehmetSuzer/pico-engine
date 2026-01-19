
#ifndef PICO_ENGINE_GRAPHICS_MODEL_H
#define PICO_ENGINE_GRAPHICS_MODEL_H

#include "mesh.h"
#include "texture.h"
#include "common/components.h"

typedef struct model
{
    mesh_t mesh;
    texture_t texture;
} model_t;

void model_draw(const model_t* model, const transform_component_t* transform);

#endif // PICO_ENGINE_GRAPHICS_MODEL_H
