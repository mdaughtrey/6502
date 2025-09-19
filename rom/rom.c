#include <stdio.h>
#include <pico/stdio.h>
#include <pico/time.h>
#include "hardware/gpio.h"

#define LED_PIN 25
bool state = 1;

int main()
{
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, 1);
    stdio_init_all();
    while (1)
    {
        printf("Hello World\n");
        gpio_put(LED_PIN, state);
        state = !state
        sleep_ms(100);
        printf("Hello!\n");
    }
    return 0;
}
