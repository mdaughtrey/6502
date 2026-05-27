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

    void isr(void)
    {
        VERBOSE("ISR");
        clock_stop(clk_gpout3);
    }

    void init()
    {
        std::cout << "pio_break Init" << std::endl;
        breakpoints.clear();
        offset = pio_add_program(pio, &break_program);
        smc =  break_program_get_default_config(offset);
        sm_config_set_in_pins(&smc, 0);
        sm_config_set_in_pin_count(&smc, 16);
        sm_config_set_out_shift(&smc, false, false, 16);    // autopull disabled
        sm_config_set_in_shift(&smc, false, false, 16);      // autopush disabled
        irq_set_exclusive_handler(PIO1_IRQ_0, isr);

        pio_set_irq0_source_enabled(pio, pis_interrupt0, true);
        std::cout << "pio_break init done" << std::endl;
    }
    
    bool cmd_set(CommandInput input = CommandInput())
    {
        if (input.empty())
        {
            return false;
        }
        return true;

        if (breakpoints.size() >= MAX_BREAKPOINTS)
        {
            std::cout << "Maximum number of breakpoints reached." << std::endl;
            return true;
        }
        auto address = std::stoi(input[1], nullptr, 16);
        VERBOSE("Address %04x", address);

        auto it = std::find_if(breakpoints.begin(), breakpoints.end(),
            [&](const BreakPoint & b) { return b.address == address; });

        if (it == breakpoints.end()) 
        {
            VERBOSE("Adding breakpoint at %04x", address);
            int sm = pio_claim_unused_sm(pio, true);
            pio_sm_init(pio, sm, offset, &smc);
            pio_sm_set_enabled(pio, sm, true);
//            breakpoints.emplace_back(BreakPoint(address, true, sm));   // or {value, flag}
        }
        return true;
    }

    bool cmd_clear(CommandInput input = CommandInput())
    {
        if (input.empty())
        {
            return false;
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
        return true;
    }

    bool cmd_enable(CommandInput input = CommandInput())
    {
        if (input.empty())
        {
            return false;
        }
        return true;
    }

    bool cmd_disable(CommandInput input = CommandInput())
    {
        if (input.empty())
        {
            return false;
        }
        return true;
    }

    bool cmd_list(CommandInput input = CommandInput())
    {
        for (auto iter : breakpoints)
        {
            VERBOSE("Breakpoint at %04x", iter.address);
        }
        return true;
    }

} // namespace pio_break

