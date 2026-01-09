
#ifndef PICO_ENGINE_DEVICE_INPUT_H
#define PICO_ENGINE_DEVICE_INPUT_H

#include <pico/stdlib.h>

#define INPUT_KEY_A          15u
#define INPUT_KEY_B          17u
#define INPUT_KEY_X          19u
#define INPUT_KEY_Y          21u
#define INPUT_KEY_FORWARD     2u
#define INPUT_KEY_BACKWARD   18u
#define INPUT_KEY_LEFT       16u
#define INPUT_KEY_RIGHT      20u
#define INPUT_KEY_CTRL        3u

void input_init_buttons();

static inline bool input_key_released(uint key)
{
    return gpio_get(key);
}

static inline bool input_key_pressed(uint key)
{
    return !input_key_released(key);
}

#endif // PICO_ENGINE_DEVICE_INPUT_H
