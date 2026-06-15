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
#include <log_queue.h>

namespace iohost_read
{
    typedef std::pair<const pio_program *, pio_sm_config (*)(uint)> Program;
    typedef std::map<std::string, Program> ProgramMap;
    typedef ProgramMap::iterator ProgramMapIterator;

    ProgramMap programs = {
        {"fifo_echo", Program(&fifo_echo_program, fifo_echo_program_get_default_config)},
        {"push_on_match", Program(&push_on_match_program, push_on_match_program_get_default_config)},
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
    std::vector<uint8_t> iohost_buffers;

    void iohost_read_isr()
    {
        isr = true;
        pio_sm_set_enabled(pio, sm, false);
        pio_interrupt_clear(pio, 0);
    }

    void init()
    {
    }

	bool cmd_initialize_test(CommandInput input = CommandInput())
	{
		offset = pio_add_program(pio, &irq_on_match_sync_program);
		smc = irq_on_match_sync_program_get_default_config(offset);

		sm = pio_claim_unused_sm(pio, true);
		VERBOSE("Claimed sm %u", sm);

		// Configure BEFORE init
		sm_config_set_in_pins(&smc, 0);
		sm_config_set_in_pin_count(&smc, 16);
		sm_config_set_jmp_pin(&smc, PIN_RW);
//	    sm_config_set_out_shift(&smc, false, false, 32);
//		sm_config_set_in_shift(&smc, false, false, 32);
	    sm_config_set_out_shift(&smc, false, false, 16);
		sm_config_set_in_shift(&smc, false, false, 16);
		sm_config_set_clkdiv(&smc, 1.0f);

		// Initialize SM
		pio_sm_init(pio, sm, offset, &smc);
		VERBOSE("initialized sm");

		// RP2350 interrupt routing
		irq_set_exclusive_handler(PIO0_IRQ_0, iohost_read_isr);
		pio_set_irq0_source_enabled(pio, pis_interrupt0, true);
		irq_set_enabled(PIO0_IRQ_0, true);
		VERBOSE("interrupts set");

		// Start SM
		pio_sm_set_enabled(pio, sm, true);
		VERBOSE("sm started");

		// Initial OSR value
		pio_sm_put(pio, sm, 0x0300);
		VERBOSE("Pushed 0x0300");

		return false;
	}


//    bool cmd_initialize_test(CommandInput input = CommandInput())
//    {
//        offset = pio_add_program(pio, &irq_on_match_sync_program);
//        smc =  irq_on_match_sync_program_get_default_config(offset);
//        sm = pio_claim_unused_sm(pio, true);
//        sm_config_set_in_pins(&smc, 0);
//        sm_config_set_in_pin_count(&smc, 16);
//        sm_config_set_out_shift(&smc, false, false, 16);    // autopull disabled
//        sm_config_set_in_shift(&smc, false, false, 16);      // autopush disabled
////        irq_set_exclusive_handler(PIO0_IRQ_0, iohost_read_isr);
//        pio_sm_init(pio, sm, offset, &smc);
//
//        pio_set_irq0_source_enabled(pio, pis_interrupt0, true);
////        irq_set_enabled(PIO0_IRQ_0, true);
//        pio_sm_set_enabled(pio, sm, true);
//        pio_sm_put(pio, sm, 0x0300);
//        return false;
//    }

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
//        std::cout << "IOHost offset " << offset << std::endl;
        sm = pio_claim_unused_sm(pio, true);

        // smc = iohost_read_program_get_default_config(offset);
        smc = iter->second.second(offset);
        sm_config_set_in_pins(&smc, 0);
        sm_config_set_in_pin_count(&smc, 16);
//        std::cout << "outShiftRight " << outShiftRight << " inShiftRight " << inShiftRight << std::endl;
//        sm_config_set_out_shift(&smc, outShiftRight, false, 16);    // autopull disabled
//        sm_config_set_in_shift(&smc, inShiftRight, false, 16);      // autopush disabled
        pio_sm_init(pio, sm, offset, &smc);
        // Push pattern into OSR
        irq_set_exclusive_handler(PIO0_IRQ_0, iohost_read_isr);
//        pio_sm_set_enabled(pio, sm, true);
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
        pio_sm_put(pio, sm, value);
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
        return false;
    }

    bool cmd_set_in_shift(CommandInput input = CommandInput())
    {
        if (input.empty())
        {
            return true;
        }
        inShiftRight = ('r' == input[0][0] ? true : false);
        return false;
    }

    typedef enum 
    {
        LIO_SIGNALS = 0,
        LIO_TAIL,
        LIO_HEAD,
        HIO_SIGNALS,
        HIO_TAIL,
        HIO_HEAD,
        LIO_DATA,
        HIO_DATA = LIO_DATA + 8,
        BUFFERS_LENGTH = HIO_DATA + 8,
        BUFFERS_BASE = 0x0300
    }BufferAt;

	std::vector<std::string> buffer_names = {
        "LIO_SIGNALS",
        "LIO_TAIL",
        "LIO_HEAD",
        "HIO_SIGNALS",
        "HIO_TAIL",
        "HIO_HEAD",
        "LIO_DATA",
        "HIO_DATA"
	};
	

	void dump_iohost_memory()
	{
        iohost_buffers = rom_ram::read_memory(BUFFERS_BASE, BUFFERS_LENGTH);
        //gpio_put(PIN_READY, 1);
        std::vector<uint8_t>::const_iterator iter_buffer = iohost_buffers.begin();
        std::vector<std::string>::const_iterator iter_name = buffer_names.begin();

		std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(*iter_buffer++) << " " << *iter_name++ << std::endl; // LIO_SIGNALS
		std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(*iter_buffer++) << " " << *iter_name++ << std::endl; // LIO_TAIL
		std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(*iter_buffer++) << " " << *iter_name++ << std::endl; // LIO_HEAD
		std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(*iter_buffer++) << " " << *iter_name++ << std::endl; // HIO_SIGNALS
		std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(*iter_buffer++) << " " << *iter_name++ << std::endl; // HIO_TAIL
		std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(*iter_buffer++) << " " << *iter_name++ << std::endl; // HIO_HEAD

        // LIO_DATA
        std::cout << std::endl << *iter_name++;
        for (int ii = 0; ii < 8; ii++)
        {
            std::cout << " " << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(*iter_buffer++);
        }

        // HIO_DATA
        std::cout << std::endl << *iter_name++;
        for (int ii = 0; ii < 8; ii++)
        {
            std::cout << " " << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(*iter_buffer++);
        }
        std::cout << std::endl;
	}

	bool cmd_dump_iohost_memory(CommandInput input = CommandInput())
	{
        dump_iohost_memory();
        return false;
	}


    void process_isr()
    {
        std::vector<uint8_t> local_data;
        isr = false;
//            std::cout << "ISR Loop" << std::endl;
//            std::cout << "Reading Buffers" << std::endl;
        dump_iohost_memory();
        if ((0xff == iohost_buffers[LIO_SIGNALS]) || (!(iohost_buffers[LIO_SIGNALS] & 0x80)))   // TOHOST_READY
        {
//                std::cout << "Not Ready" << std::endl;
//                pio_interrupt_clear(pio, 0);
            pio_sm_set_enabled(pio, sm, true);
            pio_sm_init(pio, sm, offset, &smc);
            pio_sm_set_enabled(pio, sm, true);
            pio_sm_put(pio, sm, 0x0300);
            return;
        }
//            std::cout << "Ready" << std::endl;

        uint8_t head = iohost_buffers[LIO_HEAD];
        uint8_t tail = iohost_buffers[LIO_TAIL];
        if (head == tail)
        {
//                std::cout << "Empty Buffer" << std::endl;
            pio_sm_set_enabled(pio, sm, true);
            return;
        }
        while (head != tail)
        {
            VERBOSE("Head %u Tail %u", head, tail);
            local_data.push_back(iohost_buffers[LIO_DATA+tail]);
            tail++;
            tail &= 7;
        }
        iohost_buffers[LIO_SIGNALS] &= ~0x80;
        iohost_buffers[LIO_TAIL] = tail;
        // Write back signals and head
        std::cout << "Writing back" << std::endl;
        rom_ram::write_memory(&iohost_buffers[0], 2, BUFFERS_BASE);
        pio_interrupt_clear(pio, 0);

        // Reinit the state machine
        pio_sm_init(pio, sm, offset, &smc);
        pio_sm_set_enabled(pio, sm, true);
 		pio_sm_put(pio, sm, 0x0300);
//            std::cout << "Writing back Done" << std::endl;

        std::cout << "Local Data Dump" << std::endl;
        for (auto iter : local_data)
        {
            std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)iter << " ";
        }
        std::cout << std::endl;
    }

    void loop()
    {
        if (isr)
        {
            process_isr();
        }
    }


} // namespace iohost_read

