#ifndef IOHOST_READ_H
#define IOHOST_READ_H

#include "types.h"

namespace iohost_read
{
    void init();
    void loop();
    void cmd_set_isr(bool);
    bool cmd_load_pio(CommandInput);
    bool cmd_push_to_fifo(CommandInput);
    bool cmd_read_from_fifo(CommandInput);
    bool cmd_reset_pio(CommandInput);
    bool cmd_set_out_shift(CommandInput);
    bool cmd_set_in_shift(CommandInput);
    bool cmd_list_programs(CommandInput);
    bool cmd_initialize_test(CommandInput);
	bool cmd_dump_iohost_memory(CommandInput);
    bool cmd_terminal_mode(CommandInput);
}


#endif //  IOHOST_READ_H
