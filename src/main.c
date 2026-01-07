
#include <stdio.h>

#include "common/components.h"
#include "pgl/pgl.h"
#include "device/device.h"
#include "mesh/mesh.h"

#define HEART_TEXTURE_ROW 8
#define HEART_TEXTURE_COL 8

const colour_t heart_texture[HEART_TEXTURE_ROW][HEART_TEXTURE_COL] = {
    {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF},
    {0xFFFF, 0x0000, 0x0000, 0xFFFF, 0xFFFF, 0x0000, 0x0000, 0xFFFF},
    {0x0000, 0xFFFF, 0xF800, 0x0000, 0x0000, 0xFFFF, 0xF800, 0x0000},
    {0x0000, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0x0000},
    {0x0000, 0xF800, 0xF800, 0x3800, 0xF800, 0x3800, 0xF800, 0x0000},
    {0xFFFF, 0x0000, 0x3800, 0xF800, 0x3800, 0xF800, 0x0000, 0xFFFF},
    {0xFFFF, 0xFFFF, 0x0000, 0x3800, 0x3800, 0x0000, 0xFFFF, 0xFFFF},
    {0xFFFF, 0xFFFF, 0xFFFF, 0x0000, 0x0000, 0xFFFF, 0xFFFF, 0xFFFF},
};

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

    camera_component_t camera_camera = {Q_FOURTHPI, Q_FROM_FLOAT(0.1f), Q_FROM_FLOAT(100.0f)};
    pgl_projection(camera_camera.fovw, camera_camera.near, camera_camera.far);

    pgl_bind_texture((colour_t*)heart_texture, HEART_TEXTURE_ROW, HEART_TEXTURE_COL);

    uint32_t prev_time = time_us_32();

    while (true)
    {
        const uint32_t curr_time = time_us_32();
        const uint32_t fps = 1E6f / (curr_time - prev_time);
        prev_time = curr_time;
        printf("FPS: %lu\n", fps);

        pgl_clear(PGL_COLOUR_BUFFER_BIT | PGL_DEPTH_BUFFER_BIT);
        pgl_draw(cube_mesh.vertices, cube_mesh.indices, cube_mesh.index_count);

        const colour_t* colours = pgl_colour_buffer();
        device_display(colours);
    }

    return 0;
}
