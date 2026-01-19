
#include "scene.h"

void scene_init(scene_t* scene, camera_t camera)
{
    scene->object_count = 0;
    scene->camera = camera;
}

void scene_add_object(scene_t* scene, object_t object)
{
    if (scene->object_count == SCENE_MAX_OBJECT_COUNT) return;
    scene->objects[scene->object_count++] = object;
}

void scene_draw(const scene_t* scene)
{
    camera_set_view_proj(&scene->camera);
    for (uint32_t i = 0; i < scene->object_count; ++i)
        model_draw(&scene->objects[i].model, &scene->objects[i].transform);
}

