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
    volatile bool isr = false;
	volatile uint16_t last_isr_value = 0;
	volatile uint16_t last_fifo_value = 0;
    void iohost_read_isr()
    {
        isr = true;
//        std::cout << "ISR" << std::endl;
//        uint16_t value = pio_sm_get_blocking(pio0, 0);
//        std::cout << "Value: " << std::hex << value << std::endl;
//        irq_set_enabled(PIO0_IRQ_0, false);
        pio_interrupt_clear(pio0, 0);
    }

    void init()
    {
    }


    void cmd_init_irq(void)
    {
        int rc;
        int sm = 0;
        PIO pio = pio0;
        uint offset = pio_add_program(pio, &iohost_read_program);
        std::cout << "IOHost offset " << offset << std::endl;

        // State machine, instruction offset 0, base pin 0, pattern 0x0300
        rc = iohost_read_program_init(pio, sm, 0, 0, 0x0300);
        if (0 != rc)
        {
            std::cout << "IOHost Init Error " << rc << std::endl;
        }
        irq_set_exclusive_handler(PIO0_IRQ_0, iohost_read_isr);
        pio_sm_set_enabled(pio, sm, true);
    }

    void cmd_set_isr(bool set)
    {
        if (set)
        {
            pio_interrupt_clear(pio0, 0);
        }
        irq_set_enabled(PIO0_IRQ_0, set);
    }

    void loop()
    {
		while (!pio_sm_is_rx_fifo_empty(pio0, 0)) {
				uint32_t raw = pio_sm_get(pio0, 0);
				uint16_t value = raw & 0xFFFF;

				// First FIFO entry = autopush
				// Second FIFO entry = manual push
				last_fifo_value = value;

				std::cout << "FIFO: 0x" << std::hex << value << std::endl;
			}

        if (isr)
        {
            std::cout << "ISR Triggered" << std::endl;
            isr = false;
        }

    }


} // namespace iohost_read

