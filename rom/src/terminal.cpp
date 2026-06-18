#include <string>
#include <list>
#include <iostream>
#include <iomanip>
#include <hardware/pio.h>
#include <hardware/irq.h>
#include <iohost.pio.h>
#include <bitset>
#include <vector>
#include "pico/stdlib.h"
#include "types.h"
#include "terminal.h"
#include "common_defs.h"
#include <log_queue.h>
#include <pin_defs.h>
#include <rom_ram_internal.h>
#include <iohost.h>

namespace terminal
{
    using namespace iohost;
    typedef enum
    {
        INACTIVE,
        INTERACTIVE,
        ONE_ESC
    }TerminalState;


    TerminalState state = INACTIVE;
    PIO pio = pio0;
    pio_sm_config smc;
    const pio_program_t * current_program = NULL;
    uint offset;
    int sm = 0;
    volatile bool isr = false;
    std::vector<uint8_t> terminal_buffers;

    void handle(uint8_t input);
    void read_buffer();
    void write_buffer(uint8_t data);

    bool terminal_active = false;
    void init(void)
    {
        state = INACTIVE;
    }

    bool active()
    {
        return state != INACTIVE;
    }


    void terminal_isr()
    {
        isr = true;
        pio_sm_set_enabled(pio, sm, false);
        pio_interrupt_clear(pio, 0);
    }

    bool cmd_set_interactive(CommandInput input = CommandInput())
    {
        std::cout << "Interactive Terminal, <ESC><ESC> to exit" << std::endl;
        state = INTERACTIVE;
		offset = pio_add_program(pio, &irq_on_match_sync_program);
		smc = irq_on_match_sync_program_get_default_config(offset);

		sm = pio_claim_unused_sm(pio, true);
		VERBOSE("Claimed sm %u", sm);

		// Configure BEFORE init
		sm_config_set_in_pins(&smc, 0);
		sm_config_set_in_pin_count(&smc, 16);
		sm_config_set_jmp_pin(&smc, PIN_RW);
	    sm_config_set_out_shift(&smc, false, false, 16);
		sm_config_set_in_shift(&smc, false, false, 16);
		sm_config_set_clkdiv(&smc, 1.0f);

		// Initialize SM
		pio_sm_init(pio, sm, offset, &smc);
		VERBOSE("initialized sm");

		// RP2350 interrupt routing
		irq_set_exclusive_handler(PIO0_IRQ_0, terminal_isr);
		pio_set_irq0_source_enabled(pio, pis_interrupt0, true);
		irq_set_enabled(PIO0_IRQ_0, true);
		VERBOSE("interrupts set");

		// Start SM
		pio_sm_set_enabled(pio, sm, true);
		VERBOSE("sm started");

		// Initial OSR value
		pio_sm_put(pio, sm, 0x0300);
		VERBOSE("Pushed 0x0300");
        read_buffer();
        VERBOSE("Flush Buffers");

        return false;
    }

    void input(uint8_t input)
    {
//        std::cout << std::hex << (int)input << std::endl;
        switch (input)
        {
            case 0x1b: 
                if (ONE_ESC == state)
                {
                    std::cout << "Exiting Interactive Terminal" << std::endl;
                    pio_sm_set_enabled(pio, sm, false);
                    pio_remove_program(pio, &irq_on_match_sync_program, offset);
                    state = INACTIVE;
                }
                else
                {
                    state = ONE_ESC;
                }
                break;

            default:
                if (state == ONE_ESC)
                {
                    state = INTERACTIVE;
                }
                write_buffer(input);
                break;
        }
    }


    void read_buffer()
    {
        std::vector<uint8_t> local_data;
        isr = false;
        terminal_buffers = rom_ram::read_memory(BUFFERS_BASE, LIO_LENGTH);
        BufferSet * buffer_set = reinterpret_cast<BufferSet*>(terminal_buffers.data());

        if (!(buffer_set[0].signals & 0x80))
        {
            pio_sm_init(pio, sm, offset, &smc);
            pio_sm_set_enabled(pio, sm, true);
            pio_sm_put(pio, sm, 0x0300);
            return;
        }

        uint8_t & head = buffer_set[0].head;
        uint8_t & tail = buffer_set[0].tail;
        if (head == tail)
        {
            pio_sm_set_enabled(pio, sm, true);
            return;
        }
        while (head != tail)
        {
            local_data.push_back(buffer_set[0].data[tail++]);
            tail &= 7;
        }
        buffer_set[0].signals &= ~0x80;
//        terminal_buffers[LIO_TAIL] = tail;
        // Write back signals and head
        rom_ram::write_memory(&terminal_buffers[0], 2, BUFFERS_BASE);
        pio_interrupt_clear(pio, 0);

        // Reinit the state machine
        pio_sm_init(pio, sm, offset, &smc);
        pio_sm_set_enabled(pio, sm, true);
 		pio_sm_put(pio, sm, 0x0300);

        for (auto iter : local_data)
        {
            std::cout << static_cast<char>(iter) << std::flush;
        }
    }

    void write_buffer(uint8_t data)
    {
        terminal_buffers = rom_ram::read_memory(BUFFERS_BASE + HIO_BASE, LIO_LENGTH);
        BufferSet * buffer_set = reinterpret_cast<BufferSet*>(terminal_buffers.data());
        uint8_t & signals(buffer_set[0].signals);
        uint8_t head = buffer_set[0].head;
        uint8_t & tail(buffer_set[0].tail);

        buffer_set[0].data[head++] = data;
        head &= 7;
        if (head == tail)
        {
            printf("\r\n*** Tx Buffer Full *** \r\n");
            sleep_ms(1000);
            return;
        }
        buffer_set[0].head = head;
        buffer_set[0].signals |= 0x80;
    }

    void loop(void)
    {
        if (isr)
        {
            read_buffer();
        }
    }

} // terminal
