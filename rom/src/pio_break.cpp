#include <pico/stdlib.h>
#include <hardware/pio.h>
#include <hardware/irq.h>
#include "hardware/clocks.h"
#include "types.h"
//#include <map>
//#include <string>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <vector>
// #include <iohost_read.h>
// #include <iohost_read.pio.h>
#include <pin_defs.h>
#include "log_queue.h"
#include "pio_break.pio.h"
#include "pio_break.h"
// #include <rom_ram_internal.h>

namespace pio_break
{
    typedef struct
    {
        uint16_t address;
        bool enabled;
        int sm;
    } BreakPoint;
    typedef std::vector<BreakPoint> BreakPoints;
    typedef BreakPoints::iterator BreakPointIter;
    const int MAX_BREAKPOINTS = 4;

    BreakPoints breakpoints;
    PIO pio = pio1;
    pio_sm_config smc;
    uint offset;
    bool isr_set = false;

    void isr(void)
    {
        VERBOSE("ISR");
        isr_set = true;

//        clock_stop(clk_gpout3);
    }

    bool is_break(void)
    {
        return isr_set;
    }

    void clear(void)
    {
        isr_set = false;
        pio_interrupt_clear(pio, 0); // Release the IRQ, PIO program cycles
    }

    void init()
    {
        breakpoints.clear();
        offset = pio_add_program(pio, &break_program);
        smc =  break_program_get_default_config(offset);
        sm_config_set_in_pins(&smc, 0);
        sm_config_set_in_pin_count(&smc, 16);
        sm_config_set_sideset_pins(&smc, PIN_READY);
        sm_config_set_out_shift(&smc, false, false, 16);    // autopull disabled
        sm_config_set_in_shift(&smc, false, false, 16);      // autopush disabled
        sm_config_set_jmp_pin(&smc, PIO1_IRQ_0);
        pio_gpio_init(pio, PIN_READY);
//        assert_ready(false);
        irq_set_exclusive_handler(PIO1_IRQ_0, isr);

        pio_set_irq0_source_enabled(pio, pis_interrupt0, true);
        irq_set_enabled(PIO1_IRQ_0, true);
    }
    
    bool cmd_set(CommandInput input = CommandInput())
    {
        if (input.empty())
        {
            return true;
        }

        if (breakpoints.size() >= MAX_BREAKPOINTS)
        {
            std::cout << "Maximum number of breakpoints reached." << std::endl;
            return false;
        }
        uint16_t address = std::stoi(input[1], nullptr, 16);
        VERBOSE("Address %04x", address);

        auto it = std::find_if(breakpoints.begin(), breakpoints.end(),
            [&](const BreakPoint & b) { return b.address == address; });

        if (it != breakpoints.end()) 
        {
            VERBOSE("Address %04x already set", address);
            return false;
        }

        VERBOSE("Adding breakpoint at %04x", address);
        int sm = pio_claim_unused_sm(pio, true);
        VERBOSE("sm %d", sm);
        pio_sm_init(pio, sm, offset, &smc);
        pio_sm_set_pins_with_mask(pio, sm, 0, 1u << PIN_READY);
        pio_sm_set_enabled(pio, sm, true);
        pio_sm_put_blocking(pio, sm, address);
        breakpoints.push_back({address, true, sm});   
        return false;
    }

    bool cmd_clear(CommandInput input = CommandInput())
    {
        if (input.empty())
        {
            return true;
        }
        auto address = std::stoi(input[1], nullptr, 16);
        breakpoints.erase(
            std::remove_if(
                breakpoints.begin(),
                breakpoints.end(),
                [&](const BreakPoint& b) { return b.address == address; }
            ),
            breakpoints.end()
        );
        VERBOSE("Cleared");
        return false;
    }

    bool cmd_enable(CommandInput input = CommandInput())
    {
        if (input.empty())
        {
            return true;
        }
        return false;
    }

    bool cmd_disable(CommandInput input = CommandInput())
    {
        if (input.empty())
        {
            return true;
        }
        return false;
    }

    bool cmd_list(CommandInput input = CommandInput())
    {
        VERBOSE("There are %d breakpoints\r\n", breakpoints.size());
        for (auto iter : breakpoints)
        {
            std::cout << "Breakpoint at " << std::hex << std::setfill('0') << std::setw(4) << iter.address << std::endl;
        }
        return false;
    }

    bool cmd_clear_all(CommandInput input = CommandInput())
    {
        for (auto iter: breakpoints)
        {
            pio_sm_unclaim(pio, iter.sm);
        }
        breakpoints.clear();
        return false;
    }

    void assert_ready(bool set)
    {
        if (set)
        {
            gpio_set_dir(PIN_READY, GPIO_OUT);
            gpio_put(PIN_READY, 0);
        }
        else
        {
            gpio_set_dir(PIN_READY, GPIO_IN);
            gpio_pull_up(PIN_READY);
        }
    }

} // namespace pio_break
