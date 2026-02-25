#include <string>
#include <vector>

#include "types.h"

namespace debugger
{
    void init(void);
    void loop(void);
    bool cmd_show_source_file(CommandInput);
};
