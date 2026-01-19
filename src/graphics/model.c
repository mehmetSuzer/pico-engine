
#include "model.h"

void model_draw(const model_t* model, const transform_component_t* transform)
{
    pgl_model(transform->position, transform->rotation, transform->scale);
    pgl_bind_texture(model->texture.texels, model->texture.width_bits, model->texture.height_bits);
    pgl_draw(model->mesh.vertices, model->mesh.indices, model->mesh.index_count);
}
