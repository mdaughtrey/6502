#ifndef IOHOST_READ_H
#define IOHOST_READ_H

#include "types.h"

namespace iohost_read
{
    void init();
    void loop();
    void cmd_set_isr(bool);
    void cmd_init_irq(void);
    void cmd_read_rx_fifo(void);
}


#endif //  IOHOST_READ_H
