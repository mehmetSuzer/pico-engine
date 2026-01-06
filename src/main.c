
#include <stdio.h>

#include "common/components.h"
#include "pgl/pgl.h"
#include "device/device.h"
#include "mesh/mesh.h"

int main()
{
    device_init();
    
    pgl_viewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    pgl_clear_colour(COLOUR_BLACK);
    pgl_clear_depth(DEPTH_FURTHEST);

    transform_component_t cube_transform = {{{Q_ZERO, Q_ZERO, Q_M_TWO}}, Q_QUAT_IDENTITY, Q_VEC3_ONE};
    pgl_model(cube_transform.position, cube_transform.rotation, cube_transform.scale);

    transform_component_t camera_transform = {{{Q_ZERO, Q_ZERO, Q_ZERO}}, Q_QUAT_IDENTITY, Q_VEC3_ONE};
    pgl_view(
        camera_transform.position, 
        q_quat_rotate_vec3(camera_transform.rotation, Q_VEC3_BACKWARD),
        q_quat_rotate_vec3(camera_transform.rotation, Q_VEC3_UP));

    uint32_t prev_time = time_us_32();

    while (true)
    {
        uint32_t curr_time = time_us_32();
        float dt = (curr_time - prev_time) / 1E6f;
        prev_time = curr_time;

        pgl_clear(PGL_COLOUR_BUFFER_BIT | PGL_DEPTH_BUFFER_BIT);
        // device_display();

        printf("dt: % .6f\n", dt);
    }

    return 0;
}
