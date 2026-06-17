#include <string>
#include <vector>

#include "types.h"

namespace terminal
{
    void init(void);
    void loop(void);
    void input(uint8_t input);
    bool active();
    bool cmd_set_interactive(CommandInput);
};
