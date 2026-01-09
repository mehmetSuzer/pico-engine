
#include "model.h"

void model_draw(const model_t* model)
{
    pgl_model(model->transform.position, model->transform.rotation, model->transform.scale);
    pgl_draw(model->mesh.vertices, model->mesh.indices, model->mesh.index_count);
}
