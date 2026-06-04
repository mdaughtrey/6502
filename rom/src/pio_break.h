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
    bool cmd_clear_all(CommandInput);
    bool is_break(uint16_t & address);
    void clear(void);
    void assert_ready(bool);
    void release_ready(uint16_t address);
    void reset(uint16_t address);
}


#endif // PIO_BREAK_H
