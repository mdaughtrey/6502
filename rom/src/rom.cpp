#include <pico/stdio.h>
#include <pico/time.h>
#include "hardware/gpio.h"
#include <cstdio>

#include <cmd_io.h>
#include <rom_ram.h>
#include <via6522.h>
#include <menu.h>
#include <iohost.h>
#include <log_queue.h>
#include <pio_break.h>
#include <terminal.h>

int main()
{
    stdio_init_all();
    log_queue::init();
    cmd_io::init();
    rom_ram::init();
    via6522::init();
    iohost::init();
    pio_break::init();
    terminal::init();
    while (1)
    {
        int8_t input = getchar_timeout_us(0);
        if (input != PICO_ERROR_TIMEOUT)
        {
            menu::handle(input);
        }
        cmd_io::loop();
        via6522::loop();
        iohost::loop();
        log_queue::loop();
        terminal::loop();
    }
    return 0;
}
