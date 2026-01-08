
#include "input.h"

static void input_gpio_set(uint pin, bool mode) 
{
    gpio_init(pin);
    gpio_set_dir(pin, mode);
}

void input_init_buttons()
{
    input_gpio_set(DEVICE_KEY_A, GPIO_IN);
    gpio_pull_up(DEVICE_KEY_A);
    input_gpio_set(DEVICE_KEY_B, GPIO_IN);
    gpio_pull_up(DEVICE_KEY_B);
    input_gpio_set(DEVICE_KEY_X, GPIO_IN);
    gpio_pull_up(DEVICE_KEY_X);
    input_gpio_set(DEVICE_KEY_Y, GPIO_IN);
    gpio_pull_up(DEVICE_KEY_Y);

    input_gpio_set(DEVICE_KEY_FORWARD, GPIO_IN);
    gpio_pull_up(DEVICE_KEY_FORWARD);
    input_gpio_set(DEVICE_KEY_BACKWARD, GPIO_IN);
    gpio_pull_up(DEVICE_KEY_BACKWARD);
    input_gpio_set(DEVICE_KEY_LEFT, GPIO_IN);
    gpio_pull_up(DEVICE_KEY_LEFT);
    input_gpio_set(DEVICE_KEY_RIGHT, GPIO_IN);
    gpio_pull_up(DEVICE_KEY_RIGHT);
    input_gpio_set(DEVICE_KEY_CTRL, GPIO_IN);
    gpio_pull_up(DEVICE_KEY_CTRL);
}

void input_set_button_irq_callbacks(gpio_irq_callback_t callback, uint32_t event_mask, bool enabled) 
{
    gpio_set_irq_enabled_with_callback(DEVICE_KEY_FORWARD,  event_mask, enabled, callback);
    gpio_set_irq_enabled(DEVICE_KEY_BACKWARD, event_mask, enabled);
    gpio_set_irq_enabled(DEVICE_KEY_LEFT,     event_mask, enabled);
    gpio_set_irq_enabled(DEVICE_KEY_RIGHT,    event_mask, enabled);
    gpio_set_irq_enabled(DEVICE_KEY_CTRL,     event_mask, enabled);
    gpio_set_irq_enabled(DEVICE_KEY_A,        event_mask, enabled);
    gpio_set_irq_enabled(DEVICE_KEY_B,        event_mask, enabled);
    gpio_set_irq_enabled(DEVICE_KEY_X,        event_mask, enabled);
    gpio_set_irq_enabled(DEVICE_KEY_Y,        event_mask, enabled);
}
