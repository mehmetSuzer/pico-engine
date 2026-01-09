
#include "input.h"

static void input_gpio_set(uint pin, bool mode) 
{
    gpio_init(pin);
    gpio_set_dir(pin, mode);
}

void input_init_buttons()
{
    input_gpio_set(INPUT_KEY_A, GPIO_IN);
    gpio_pull_up(INPUT_KEY_A);
    input_gpio_set(INPUT_KEY_B, GPIO_IN);
    gpio_pull_up(INPUT_KEY_B);
    input_gpio_set(INPUT_KEY_X, GPIO_IN);
    gpio_pull_up(INPUT_KEY_X);
    input_gpio_set(INPUT_KEY_Y, GPIO_IN);
    gpio_pull_up(INPUT_KEY_Y);

    input_gpio_set(INPUT_KEY_FORWARD, GPIO_IN);
    gpio_pull_up(INPUT_KEY_FORWARD);
    input_gpio_set(INPUT_KEY_BACKWARD, GPIO_IN);
    gpio_pull_up(INPUT_KEY_BACKWARD);
    input_gpio_set(INPUT_KEY_LEFT, GPIO_IN);
    gpio_pull_up(INPUT_KEY_LEFT);
    input_gpio_set(INPUT_KEY_RIGHT, GPIO_IN);
    gpio_pull_up(INPUT_KEY_RIGHT);
    input_gpio_set(INPUT_KEY_CTRL, GPIO_IN);
    gpio_pull_up(INPUT_KEY_CTRL);
}

