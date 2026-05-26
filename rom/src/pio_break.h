#ifndef PIO_BREAK_H
#define PIO_BREAK_H

#include "types.h"

namespace pio_break
{
    void init();
    bool set(CommandInput);
    bool clear(CommandInput);
    bool disable(CommandInput);
    bool enable(CommandInput);
}


#endif // PIO_BREAK_H
