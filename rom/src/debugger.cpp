#include <string>
#include <list>
#include <iostream>
#include <bitset>
#include <vector>
// #include "hardware/structs/sio.h"
#include "pico/stdlib.h"
// #include "hardware/clocks.h"
// #include "hardware/pwm.h"
#include "types.h"
#include "debugger.h"

#include "common_defs.h"
#include "rom_ram_internal.h"

// uint32_t asm_main_size = sizeof(asm_main)

namespace debugger
{
    std::list<std::string> log_queue;
    void init(void)
    {
    }

    void loop(void)
    {
        for (auto iter = log_queue.begin(); iter != log_queue.end(); iter++)
        {
            std::cout << *iter << std::endl;
        }
        log_queue.clear();
    }

    bool set_breakpoint(CommandInput input)
    {
        return false;
    }

    bool clear_breakpoint(CommandInput input)
    {
        return false;
    }

    bool dump_memory(CommandInput input)
    {
        return false;
    }

    bool run(CommandInput input)
    {
        return false;
    }

    bool step(CommandInput input)
    {
        return false;
    }

} // debugger
