
#ifndef PICO_ENGINE_DEVICE_INPUT_H
#define PICO_ENGINE_DEVICE_INPUT_H

#include <pico/stdlib.h>

#define DEVICE_KEY_A          15u
#define DEVICE_KEY_B          17u
#define DEVICE_KEY_X          19u
#define DEVICE_KEY_Y          21u

#define DEVICE_KEY_FORWARD     2u
#define DEVICE_KEY_BACKWARD   18u
#define DEVICE_KEY_LEFT       16u
#define DEVICE_KEY_RIGHT      20u
#define DEVICE_KEY_CTRL        3u

void input_init_buttons();

void input_set_button_irq_callbacks(gpio_irq_callback_t callback, uint32_t event_mask, bool enabled);

#endif // PICO_ENGINE_DEVICE_INPUT_H
