
#include <stdio.h>

#include "pgl/pgl.h"
#include "device/device.h"

int main()
{
    device_init();
    
    pgl_viewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    pgl_clear_colour(COLOUR_BLACK);
    pgl_clear_depth(DEPTH_FURTHEST);

    uint32_t prev_time = time_us_32();

    while (true)
    {
        uint32_t curr_time = time_us_32();
        float dt = (curr_time - prev_time) / 1E6f;
        prev_time = curr_time;

        pgl_clear(PGL_COLOUR_BUFFER_BIT | PGL_DEPTH_BUFFER_BIT);

        printf("dt: % .6f\n", dt);
    }

    return 0;
}
