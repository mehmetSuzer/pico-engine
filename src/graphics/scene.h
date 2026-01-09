
#ifndef PICO_ENGINE_GRAPHICS_SCENE_H
#define PICO_ENGINE_GRAPHICS_SCENE_H

#include "model.h"
#include "camera.h"

#define SCENE_MAX_MODEL_COUNT 10

typedef struct scene
{
    model_t models[SCENE_MAX_MODEL_COUNT];
    camera_t camera;
    uint32_t model_count;
} scene_t;

void scene_init(scene_t* scene, camera_t camera);
void scene_add_model(scene_t* scene, model_t model);
void scene_draw(const scene_t* scene);

#endif // PICO_ENGINE_GRAPHICS_SCENE_H
