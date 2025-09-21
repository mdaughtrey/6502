#include <pico/stdio.h>
#include <pico/time.h>
#include "hardware/gpio.h"
#include <cstdio>

#include <menu.h>

#define LED_PIN 25
bool state = 1;

int main()
{
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, 1);
    stdio_init_all();
    while (1)
    {
        int8_t input = getchar_timeout_us(0);
        if (input != PICO_ERROR_TIMEOUT)
        {
            menu_handle(input);
        }
        gpio_put(LED_PIN, state);
        state = !state;
        sleep_ms(101);
    }
    return 0;
}
