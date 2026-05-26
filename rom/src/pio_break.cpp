#include <pico/stdlib.h>
#include <hardware/pio.h>
#include <hardware/irq.h>
#include "hardware/clocks.h"
//#include <iomanip>
//#include <map>
//#include <string>
//#include <iostream>
#include <vector>
// #include <iohost_read.h>
// #include <iohost_read.pio.h>
#include <pin_defs.h>
// #include <rom_ram_internal.h>

namespace pio_break
{
    typedef std::pair<uint16_t, bool> BreakPoint;
    typedef std::vector<BreakPoint> BreakPoints;

    void isr(void)
    {
         clock_stop(clk_gpout3);
    }

    void init()
    {
    }
    
    bool set(uint16_t address)
    {
        return true;
    }

    bool clear(uint16_t address)
    {
        return true;
    }

    bool enable(uint16_t address)
    {
        return true;
    }

    bool disable(uint16_t address)
    {
        return true;
    }

} // namespace pio_break

