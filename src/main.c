
#include <stdio.h>
#include <stdlib.h>
#include <hardware/clocks.h>
#include <hardware/vreg.h>

#include "device/lcd.h"
#include "device/input.h"
#include "graphics/scene.h"

#define HEART_TEXTURE_WIDTH_BITS  3
#define HEART_TEXTURE_HEIGHT_BITS 3

const colour_t heart_texture[1 << HEART_TEXTURE_HEIGHT_BITS][1 << HEART_TEXTURE_WIDTH_BITS] = {
    {0xFFFF, 0xFFFF, 0xFFFF, 0x0000, 0x0000, 0xFFFF, 0xFFFF, 0xFFFF},
    {0xFFFF, 0xFFFF, 0x0000, 0x3800, 0x3800, 0x0000, 0xFFFF, 0xFFFF},
    {0xFFFF, 0x0000, 0x3800, 0xF800, 0x3800, 0xF800, 0x0000, 0xFFFF},
    {0x0000, 0xF800, 0xF800, 0x3800, 0xF800, 0x3800, 0xF800, 0x0000},
    {0x0000, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0xF800, 0x0000},
    {0x0000, 0xFFFF, 0xF800, 0x0000, 0x0000, 0xFFFF, 0xF800, 0x0000},
    {0xFFFF, 0x0000, 0x0000, 0xFFFF, 0xFFFF, 0x0000, 0x0000, 0xFFFF},
    {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF},
};

static void configure_clock() 
{
#if defined(CLOCK_FREQUENCY_KHZ)
    vreg_set_voltage(VREG_VOLTAGE_MAX);
    set_sys_clock_khz(CLOCK_FREQUENCY_KHZ, true);
    setup_default_uart();

    // Wait and do it again to clear up some UART issues
    sleep_ms(100u);
    set_sys_clock_khz(CLOCK_FREQUENCY_KHZ, true);
    setup_default_uart();

    // Get the processor sys_clk frequency in Hz
    const uint32_t freq = clock_get_hz(clk_sys);

    // clk_peri does not have a divider, so input and output frequencies will be the same
    clock_configure(clk_peri, 0, CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS, freq, freq);
#endif
}

#define PHYSICS_UPDATE_PERIOD_US 20000
#define Q_PHYSICS_UPDATE_PERIOD  Q_FROM_FLOAT(PHYSICS_UPDATE_PERIOD_US / 1000000.0f)

static void process_input(Q_VEC3* delta_position, Q_QUAT* delta_rotation)
{
    const Q_TYPE lin_speed = Q_THREE;
    const Q_TYPE ang_speed = q_mul_int(Q_2_PI, 3);

    const Q_TYPE delta_speed = q_mul(lin_speed, Q_PHYSICS_UPDATE_PERIOD);
    const Q_TYPE delta_angle = q_mul(ang_speed, Q_PHYSICS_UPDATE_PERIOD);

    *delta_position = Q_VEC3_ZERO;
    *delta_rotation = Q_QUAT_IDENTITY;

    if (input_key_pressed(INPUT_KEY_FORWARD))
        *delta_position = q_vec3_add(*delta_position, Q_VEC3_FORWARD);
    if (input_key_pressed(INPUT_KEY_BACKWARD))
        *delta_position = q_vec3_add(*delta_position, Q_VEC3_BACKWARD);
    if (input_key_pressed(INPUT_KEY_X))
        *delta_position = q_vec3_add(*delta_position, Q_VEC3_UP);
    if (input_key_pressed(INPUT_KEY_Y))
        *delta_position = q_vec3_add(*delta_position, Q_VEC3_DOWN);

    if (input_key_pressed(INPUT_KEY_LEFT))
    {
        if (input_key_pressed(INPUT_KEY_B))
            *delta_rotation = q_quat_mul_quat(*delta_rotation, q_quat_angle_axis(delta_angle, Q_VEC3_UP));
        else
            *delta_position = q_vec3_add(*delta_position, Q_VEC3_LEFT);
        }

    if (input_key_pressed(INPUT_KEY_RIGHT))
    {
        if (input_key_pressed(INPUT_KEY_B))
            *delta_rotation = q_quat_mul_quat(*delta_rotation, q_quat_angle_axis(delta_angle, Q_VEC3_DOWN));
        else
            *delta_position = q_vec3_add(*delta_position, Q_VEC3_RIGHT);
    }

    if (q_ne(delta_position->x, Q_ZERO) || q_ne(delta_position->y, Q_ZERO) || q_ne(delta_position->z, Q_ZERO))
        *delta_position = q_vec3_scale(q_vec3_normalise(*delta_position), delta_speed);
}

int main()
{
    stdio_init_all();
    configure_clock();
    input_init_buttons();
    lcd_init();

    pgl_viewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    pgl_clear_colour(COLOUR_BLACK);
    pgl_clear_depth(DEPTH_FURTHEST);

    scene_t scene;
    scene_init(&scene, (camera_t){
        .transform = {{{Q_ZERO, Q_ZERO, Q_ZERO}}, Q_QUAT_IDENTITY, Q_VEC3_ONE},
        .camera = {Q_FOURTHPI, Q_FROM_FLOAT(0.1f), Q_FROM_FLOAT(100.0f)},
    });

    for (uint32_t i = 0; i < SCENE_MAX_MODEL_COUNT; ++i)
    {
        scene_add_model(&scene, (model_t){
            .transform = {
                {{Q_FROM_INT(8 - rand()%16), Q_FROM_INT(1 - rand()%2), Q_FROM_INT(8 - rand()%16)}}, 
                q_quat_angle_axis(q_mul(Q_THIRDPI, Q_FROM_INT(rand()%10)), q_vec3_normalise(Q_VEC3_ONE)), 
                Q_VEC3_ONE,
            },
            .mesh = cube_mesh,
        });
    }

    pgl_bind_texture((colour_t*)heart_texture, HEART_TEXTURE_WIDTH_BITS, HEART_TEXTURE_HEIGHT_BITS);

    uint32_t prev_time_us = time_us_32();
    uint32_t lag_us = 0;
    uint32_t prev_frame_time_us = prev_time_us;

    while (true)
    {
        const uint32_t curr_time_us = time_us_32();
        const uint32_t dt_us = curr_time_us - prev_time_us;
        prev_time_us = curr_time_us;
        lag_us += dt_us;

        // ------------------------------------------- INPUT ------------------------------------------- //

        while (lag_us >= PHYSICS_UPDATE_PERIOD_US)
        {
            Q_VEC3 delta_position;
            Q_QUAT delta_rotation;
            process_input(&delta_position, &delta_rotation);
        
            Q_VEC3* position = &scene.camera.transform.position;
            *position = q_vec3_add(delta_position, *position);
        
            Q_QUAT* rotation = &scene.camera.transform.rotation;
            *rotation = q_quat_mul_quat(delta_rotation, *rotation);

            lag_us -= PHYSICS_UPDATE_PERIOD_US;
        }

        // --------------------------------------------------------------------------------------------- //

        if (pgl_request_frame())
        {
            const uint32_t dt_frame_us = curr_time_us - prev_frame_time_us;
            prev_frame_time_us = curr_time_us;
            const uint32_t fps = 1000000 / dt_frame_us;
            printf("FPS: %lu - Delta Time: %lu us\n", fps, dt_frame_us);

            pgl_clear(PGL_COLOUR_BUFFER_BIT | PGL_DEPTH_BUFFER_BIT);
            scene_draw(&scene);
            swapchain_request_swap();
        }
    }

    return 0;
}
