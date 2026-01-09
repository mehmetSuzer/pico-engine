
#include "camera.h"

void camera_set_view_proj(const camera_t* camera)
{
    const Q_VEC3 backward = q_quat_rotate_vec3(camera->transform.rotation, Q_VEC3_BACKWARD);
    const Q_VEC3 up = q_quat_rotate_vec3(camera->transform.rotation, Q_VEC3_UP);
    pgl_view(camera->transform.position, backward, up);
    pgl_projection(camera->camera.fovw, camera->camera.near, camera->camera.far);
}

