#include <pico/stdlib.h>
#include <hardware/pio.h>
#include <hardware/irq.h>
#include "hardware/clocks.h"
#include "types.h"
//#include <iomanip>
//#include <map>
//#include <string>
//#include <iostream>
#include <algorithm>
#include <vector>
// #include <iohost_read.h>
// #include <iohost_read.pio.h>
#include <pin_defs.h>
#include "log_queue.h"
// #include <rom_ram_internal.h>

namespace pio_break
{
    typedef std::pair<uint16_t, bool> BreakPoint;
    typedef std::vector<BreakPoint> BreakPoints;
    typedef BreakPoints::iterator BreakPointIter;

    BreakPoints breakpoints;

    void isr(void)
    {
         clock_stop(clk_gpout3);
    }

    void init()
    {
        breakpoints.clear();
    }
    
    bool set(CommandInput input = CommandInput())
    {
        auto address = std::stoi(input[1], nullptr, 16);

        auto it = std::find_if(breakpoints.begin(), breakpoints.end(),
            [&](const std::pair<int,bool>& p) { return p.first == address; });

        if (it == breakpoints.end()) 
        {
            VERBOSE("Adding breakpoint at %04x", address);
            breakpoints.emplace_back(address, true);   // or {value, flag}
        }
        return true;
    }

    bool clear(CommandInput input = CommandInput())
    {
        auto address = std::stoi(input[1], nullptr, 16);
        breakpoints.erase(
            std::remove_if(
                breakpoints.begin(),
                breakpoints.end(),
                [&](const std::pair<int,bool>& p) { return p.first == address; }
            ),
            breakpoints.end()
        );
        return true;
    }

    bool enable(CommandInput input = CommandInput())
    {
        return true;
    }

    bool disable(CommandInput input = CommandInput())
    {
        return true;
    }

} // namespace pio_break

