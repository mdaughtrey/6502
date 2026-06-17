#pragma once

#include <stdint.h>

namespace menu
{   
    typedef enum
    {
        COMMAND,
        INTERACTIVE_INPUT,
        TERMINAL_INPUT
    }InputState;

    void handle(uint8_t input);
    extern InputState state;
}
