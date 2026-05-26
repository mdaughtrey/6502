#ifndef PIO_BREAK_H
#define PIO_BREAK_H

#include "types.h"

namespace pio_break
{
    void init();
    bool set(uint16_t address);
    bool clear(uint16_t address);
    bool disable(uint16_t address);
    bool enable(uint16_t address);
}


#endif // PIO_BREAK_H
