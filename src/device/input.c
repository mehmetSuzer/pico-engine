
#include "input.h"

void input_init_buttons()
{
    gpio_init(INPUT_KEY_A);
    gpio_pull_up(INPUT_KEY_A);
    gpio_init(INPUT_KEY_B);
    gpio_pull_up(INPUT_KEY_B);
    gpio_init(INPUT_KEY_X);
    gpio_pull_up(INPUT_KEY_X);
    gpio_init(INPUT_KEY_Y);
    gpio_pull_up(INPUT_KEY_Y);

    gpio_init(INPUT_KEY_FORWARD);
    gpio_pull_up(INPUT_KEY_FORWARD);
    gpio_init(INPUT_KEY_BACKWARD);
    gpio_pull_up(INPUT_KEY_BACKWARD);
    gpio_init(INPUT_KEY_LEFT);
    gpio_pull_up(INPUT_KEY_LEFT);
    gpio_init(INPUT_KEY_RIGHT);
    gpio_pull_up(INPUT_KEY_RIGHT);
    gpio_init(INPUT_KEY_CTRL);
    gpio_pull_up(INPUT_KEY_CTRL);
}

