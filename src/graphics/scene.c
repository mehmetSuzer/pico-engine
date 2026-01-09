
#include "scene.h"

void scene_init(scene_t* scene, camera_t camera)
{
    scene->model_count = 0;
    scene->camera = camera;
}

void scene_add_model(scene_t* scene, model_t model)
{
    if (scene->model_count == SCENE_MAX_MODEL_COUNT) return;
    scene->models[scene->model_count++] = model;
}

void scene_draw(const scene_t* scene)
{
    camera_set_view_proj(&scene->camera);
    for (uint32_t i = 0; i < scene->model_count; ++i)
        model_draw(&scene->models[i]);
}
