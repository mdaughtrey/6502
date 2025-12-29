#include <string>
#include <vector>

#include "types.h"

namespace via6522
{
    void init(void);
    void loop(void);
    bool cmd_dump_registers(CommandInput);
    bool cmd_set_io_base(CommandInput);
    bool cmd_set_register(CommandInput);
};
