
#include <stdio.h>

#include "common/components.h"
#include "pgl/pgl.h"
#include "device/device.h"
#include "mesh/mesh.h"

static void draw_line(int32_t x0, int32_t y0, int32_t x1, int32_t y1, colour_t* buffer)
{
    const int32_t dx =  ABS(x1 - x0);
    const int32_t dy = -ABS(y1 - y0);
    const int32_t sx = (x0 < x1) ? 1 : -1;
    const int32_t sy = (y0 < y1) ? 1 : -1;

    int32_t error = dx + dy;
    int32_t x = x0;
    int32_t y = y0;

    while (true)
    {
        buffer[y * SCREEN_WIDTH + x] = COLOUR_RED;
        const int32_t two_error = 2 * error;
        if (two_error >= dy)
        {
            if (x == x1) break;
            error += dy;
            x += sx;
        }
        if (two_error <= dx)
        {
            if (y == y1) break;
            error += dx;
            y += sy;
        }
    }
}

int main()
{
    device_init();
    stdio_init_all();
    
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
        printf("dt: % .6f\n", dt);

        pgl_clear(PGL_COLOUR_BUFFER_BIT | PGL_DEPTH_BUFFER_BIT);
        pgl_draw(cube_mesh.vertices, cube_mesh.indices, cube_mesh.index_count);

        const colour_t* colours = pgl_colour_buffer();
        // draw_line(10, 40, 120, 150, colours);
        
        device_display(colours);
    }

    return 0;
}
