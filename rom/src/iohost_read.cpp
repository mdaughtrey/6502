#include <pico/stdlib.h>
#include <hardware/pio.h>
#include <hardware/irq.h>
#include <iomanip>
#include <map>
#include <string>
#include <iostream>
#include <iohost_read.h>
#include <iohost_read.pio.h>
#include <pin_defs.h>
#include <rom_ram_internal.h>
//extern const pio_program_t iohost_read_program;

namespace iohost_read
{
    typedef std::pair<const pio_program *, pio_sm_config (*)(uint)> Program;
    typedef std::map<std::string, Program> ProgramMap;
    typedef ProgramMap::iterator ProgramMapIterator;

    ProgramMap programs = {
        {"fifo_echo", Program(&fifo_echo_program, fifo_echo_program_get_default_config)},
        {"push_on_match", Program(&push_on_match_program, push_on_match_program_get_default_config)},
//        {"irq_on_match", Program(&irq_on_match_program, irq_on_match_program_get_default_config)},
        {"read_pins", Program(&read_pins_program, read_pins_program_get_default_config)},
        {"irq_on_match_sync", Program(&irq_on_match_sync_program, irq_on_match_sync_program_get_default_config)}
    };
    volatile bool isr = false;
    volatile uint16_t last_isr_value = 0;
    volatile uint16_t last_fifo_value = 0;
    PIO pio = pio0;
    pio_sm_config smc;
    const pio_program_t * current_program = NULL;
    uint offset;
    int isr_dropdead;
    int sm = 0;
    bool inShiftRight = false;
    bool outShiftRight = false;

    void iohost_read_isr()
    {
//        uint8_t memory_map[10];
        isr = true;
//        std::cout << "ISR Triggered" << std::endl;
//        gpio_put(PIN_READY, 0);
//
//        
//        std::cout << "Reading memory" << std::endl;
//        std::vector<uint8_t> buffers = rom_ram::read_memory(0x0300, 16);
//        std::cout << "Done" << std::endl;
//
//        gpio_put(PIN_READY, 1);

//        for (auto iter : buffers)
//        {
//            std::cout << std::hex << std::setw(2) << std::setfill('0') << (uint8_t)iter << " ";
//        }
//        std::cout << std::endl;


        // Assert address 0x0300 - 0x030f
        // Read data
        // Assert bus enable
        // Deassert sync (?)
        // Deassert address 0x0300 - 0x030f
//        uint16_t value = pio_sm_get_blocking(pio0, 0);
//        std::cout << "ISR Drop Dead: " << isr_dropdead << " Value: " << std::hex << value << std::endl;
//        if (!--isr_dropdead)
//        {
//            std::cout << "ISR Drop Dead" << std::endl;
//            irq_set_enabled(PIO0_IRQ_0, false);
//        }
        pio_sm_set_enabled(pio, sm, false);
        pio_interrupt_clear(pio0, 0);
    }

    void init()
    {
    }

    bool cmd_initialize_test(CommandInput input = CommandInput())
    {
        cmd_load_pio({"irq_on_match_sync"});
        cmd_set_isr(true);
        cmd_push_to_fifo({"0300"});
        return false;
    }

    bool cmd_list_programs(CommandInput input = CommandInput())
    {
        std::cout << "Available programs:" << std::endl;
        for (auto iter : programs)
        {
            std::cout << iter.first << std::endl;
        }
        return false;
    }

    bool cmd_load_pio(CommandInput input = CommandInput())
    {
        if (input.empty())
        {
            return true;
        }
        if (current_program)
        {
           pio_remove_program(pio, current_program, offset);
           current_program = NULL;
        }
        pio_sm_drain_tx_fifo(pio, sm);
        ProgramMapIterator iter = programs.begin();
        for (; iter != programs.end(); iter++)
        {
            if (input[0] == iter->first)
            {
                offset = pio_add_program(pio, iter->second.first);
                current_program = iter->second.first;
                break;
            }
        }
        if (!current_program)
        {
            std::cout << "Program not found" << std::endl;
            return false;
        }
//        pio_set_gpio_base(pio, 0);
//        offset = pio_add_program(pio, &iohost_read_program);
        std::cout << "IOHost offset " << offset << std::endl;
        sm = pio_claim_unused_sm(pio, true);

        // smc = iohost_read_program_get_default_config(offset);
        smc = iter->second.second(offset);
        sm_config_set_in_pins(&smc, 0);
        sm_config_set_in_pin_count(&smc, 16);
        std::cout << "outShiftRight " << outShiftRight << " inShiftRight " << inShiftRight << std::endl;
        sm_config_set_out_shift(&smc, outShiftRight, false, 16);    // autopull disabled
        sm_config_set_in_shift(&smc, inShiftRight, false, 16);      // autopush disabled
        pio_sm_init(pio, sm, offset, &smc);
        // Push pattern into OSR
        irq_set_exclusive_handler(PIO0_IRQ_0, iohost_read_isr);
        pio_sm_set_enabled(pio, sm, true);
        pio_set_irq0_source_enabled(pio, pis_interrupt0, true);
        return false;
    }

    void cmd_set_isr(bool set)
    {
		std::cout << "set ISR " << set << std::endl;
        if (set)
        {
            isr_dropdead = 10;
            pio_interrupt_clear(pio0, 0);
        }
        irq_set_enabled(PIO0_IRQ_0, set);
    }

    bool cmd_read_from_fifo(CommandInput input = CommandInput())
    {
		std::cout << "Reading FIFO" << std::endl;
		int count = 8;
    	while ((!pio_sm_is_rx_fifo_empty(pio0, 0)) && count--) 
    	{
    		uint32_t raw = pio_sm_get(pio0, 0);
//    		uint16_t value = raw & 0xFFFF;

    		// First FIFO entry = autopush
    		// Second FIFO entry = manual push
//    		last_fifo_value = value;

    		std::cout << "0x" << std::hex << std::setw(8) << std::setfill('0') << raw << " " << std::endl;
    	}
		std::cout << "Done" << std::endl;
        return false;
    }

    bool cmd_push_to_fifo(CommandInput input = CommandInput())
    {
        if (input.empty())
        {
            return true;
        }
        uint16_t value = std::stoi(input[0], nullptr, 16);
        std::cout << "FIFO Pushing " << std::hex << std::setw(4) << std::setfill('0') << value << std::endl;
        pio_sm_put_blocking(pio, sm, value);
        std::cout << "Pushed" << std::endl;
        return false;
    }

    bool cmd_reset_pio(CommandInput input = CommandInput())
    {
        pio_sm_init(pio, sm, offset, &smc);
        pio_sm_set_enabled(pio, sm, true);
        return false;
    }

    bool cmd_set_out_shift(CommandInput input = CommandInput())
    {
        if (input.empty())
        {
            return true;
        }
        outShiftRight = ('r' == input[0][0] ? true : false);
//        sm_config_set_out_shift(&smc, ('r' == input[0][0] ? true : false), false,  16);
        return false;
    }

    bool cmd_set_in_shift(CommandInput input = CommandInput())
    {
        if (input.empty())
        {
            return true;
        }
        inShiftRight = ('r' == input[0][0] ? true : false);
        //sm_config_set_in_shift(&smc, ('r' == input[0][0] ? true : false), false,  16);
        return false;
    }

    typedef enum 
    {
        LIO_SIGNALS = 0,
        LIO_HEAD,
        LIO_TAIL,
        HIO_SIGNALS,
        HIO_HEAD,
        HIO_TAIL,
        LIO_DATA,
        HIO_DATA = LIO_DATA + 8,
        BUFFERS_LENGTH = HIO_DATA + 8,
        BUFFERS_BASE = 0x0300
    }BufferAt;

    void loop()
    {
        if (isr)
        {
            std::vector<uint8_t> local_data;
            isr = false;
            std::cout << "ISR Loop" << std::endl;
            std::cout << "Reading Buffers" << std::endl;
            gpio_put(PIN_READY, 0);
            std::vector<uint8_t> buffers = rom_ram::read_memory(BUFFERS_BASE, BUFFERS_LENGTH);
            for (auto iter : buffers)
            {
                std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)iter << " ";
            }
            std::cout << std::endl << "Reading Buffers Complete" << std::endl;
            gpio_put(PIN_READY, 1);
            if (!buffers[LIO_SIGNALS] & 0x80)   // TOHOST_READY
            {
                std::cout << "Not Ready" << std::endl;
                pio_sm_set_enabled(pio, sm, true);
                return;
            }
            std::cout << "Ready" << std::endl;

            uint8_t head = buffers[LIO_HEAD];
            uint8_t tail = buffers[LIO_TAIL];
            if (head == tail)
            {
                std::cout << "Empty Buffer" << std::endl;
                pio_sm_set_enabled(pio, sm, true);
                return;
            }
            while (head != tail)
            {
                std::cout << "Head " << (int)head << " Tail " << (int)tail << std::endl;
                local_data.push_back(buffers[LIO_DATA+tail]);
                tail++;
                tail &= 7;
            }
            gpio_put(PIN_READY, 0);
            buffers[LIO_SIGNALS] &= ~0x80;
            buffers[LIO_HEAD] = head;
            // Write back signals and head
            std::cout << "Writing back" << std::endl;
            gpio_put(PIN_READY, 0);
            rom_ram::write_to_memory(&buffers[LIO_SIGNALS], 2, BUFFERS_BASE + LIO_SIGNALS);
            gpio_put(PIN_READY, 1);
            std::cout << "Writing back Done" << std::endl;

            std::cout << "Local Data Dump" << std::endl;
            for (auto iter : local_data)
            {
                std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)iter << " ";
            }
            std::cout << std::endl;
            pio_sm_set_enabled(pio, sm, true);
        }
    }


} // namespace iohost_read

