#ifndef IOHOST_READ_H
#define IOHOST_READ_H

#include "types.h"

namespace iohost_read
{
    void init();
    void loop();
    void cmd_set_isr(bool);
}


#endif //  IOHOST_READ_H
