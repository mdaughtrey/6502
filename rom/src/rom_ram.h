#pragma once
#include <string>
#include <vector>
#include <pico/stdlib.h>
#include "types.h"

namespace rom_ram
{
    void init(void);
    void loop(void);
    bool cmd_dump_memory(CommandInput);
    bool cmd_list_programs(CommandInput);
    bool cmd_load_program_to_memory(CommandInput);
    bool cmd_write_memory(CommandInput);
    bool cmd_upload_rom_image(CommandInput);
    bool cmd_upload_test_image(CommandInput);
    bool cmd_write_to_memory(CommandInput);
}
