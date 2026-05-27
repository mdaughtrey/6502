#ifndef PIO_BREAK_H
#define PIO_BREAK_H

#include "types.h"

namespace pio_break
{
    void init();
    bool cmd_set(CommandInput);
    bool cmd_clear(CommandInput);
    bool cmd_disable(CommandInput);
    bool cmd_enable(CommandInput);
    bool cmd_list(CommandInput);
}


#endif // PIO_BREAK_H
