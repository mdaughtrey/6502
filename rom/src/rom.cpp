#include <pico/stdio.h>
#include <pico/time.h>
#include "hardware/gpio.h"
#include <cstdio>

#include <cmd_io.h>
#include <rom_ram.h>
#include <via6522.h>
#include <menu.h>
#include <iohost_read.h>

int main()
{
    stdio_init_all();
    cmd_io::init();
    rom_ram::init();
    via6522::init();
    iohost_read::init();
    while (1)
    {
        int8_t input = getchar_timeout_us(0);
        if (input != PICO_ERROR_TIMEOUT)
        {
            menu::handle(input);
        }
        cmd_io::loop();
        via6522::loop();
//        iohost_read::loop();
    }
    return 0;
}
