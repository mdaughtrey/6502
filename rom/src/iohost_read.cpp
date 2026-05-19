#include <pico/stdlib.h>
#include <hardware/pio.h>
#include <hardware/irq.h>
#include <iomanip>
#include <iostream>
#include "iohost_read.h"
#include <iohost_read.pio.h>
extern const pio_program_t iohost_read_program;

namespace iohost_read
{
    bool isr = false;
    void iohost_read_isr()
    {
        isr = true;
        pio_interrupt_clear(pio0, 0);
    }

    void init()
    {
        PIO pio = pio0;
        int sm = 0;
        uint offset = pio_add_program(pio, &iohost_read_program);
        std::cout << "IOHost offset " << offset << std::endl;

        iohost_read_program_init(pio, sm, offset, 0, 0x0300);
        irq_set_exclusive_handler(PIO0_IRQ_0, iohost_read_isr);
        cmd_set_isr(false);
    }


    void cmd_set_isr(bool set)
    {
        irq_set_enabled(PIO0_IRQ_0, set);
    }

    void loop()
    {
        if (isr)
        {
            std::cout << "ISR" << std::endl;
            isr = false;
        }
    }


} // namespace iohost_read

