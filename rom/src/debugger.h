#include <string>
#include <vector>

#include "types.h"

namespace debugger
{
    void init(void);
    void loop(void);
    bool cmd_show_source_file(CommandInput);
    bool set_breakpoint(CommandInput);
    bool clear_breakpoint(CommandInput);
    bool dump_memory(CommandInput);
    bool run(CommandInput);
    bool step(CommandInput);
};
