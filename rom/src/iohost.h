#ifndef IOHOST_H
#define IOHOST_H

#include "types.h"

namespace iohost
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

    typedef enum 
    {
        LIO_SIGNALS = 0,
        LIO_TAIL,
        LIO_HEAD,
        LIO_DATA,
        LIO_LENGTH = LIO_DATA + 8,
        HIO_SIGNALS = LIO_LENGTH,
        HIO_TAIL,
        HIO_HEAD,
        HIO_DATA,
        HIO_LENGTH = HIO_DATA + 8 - LIO_LENGTH,
        BUFFERS_LENGTH = HIO_DATA + 8,
        BUFFERS_BASE = 0x0300
    }BufferAt;
}


#endif //  IOHOST_H
