
#ifndef PICO_ENGINE_GRAPHICS_SCENE_H
#define PICO_ENGINE_GRAPHICS_SCENE_H

#include "object.h"
#include "camera.h"

#define SCENE_MAX_OBJECT_COUNT 20

typedef struct scene
{
    object_t objects[SCENE_MAX_OBJECT_COUNT];
    camera_t camera;
    uint32_t object_count;
} scene_t;

void scene_init(scene_t* scene, camera_t camera);
void scene_add_object(scene_t* scene, object_t object);
void scene_draw(const scene_t* scene);

#endif // PICO_ENGINE_GRAPHICS_SCENE_H
