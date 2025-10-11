#pragma once
#include <string>
#include <pico/stdlib.h>

namespace rom_ram
{
    const uint16_t RR_ROM_BASE = 0x8000;
    const uint16_t RR_ROM_SIZE = 0x8000;
    const uint16_t RR_RAM_BASE = 0x0000;
    const uint16_t RR_RAM_SIZE = 0x8000;
    uint8_t RAM[RR_RAM_SIZE];
    uint8_t ROM[RR_ROM_SIZE];

    void init(void);
    void loop(void);
    bool cmd_upload_rom(std::string);
    bool cmd_program_to_rom(std::string);
    bool cmd_dump_memory(std::string);
    bool cmd_rom_to_program(std::string);
    bool cmd_list_programs(std::string);
    bool cmd_load_program_to_rom(std::string);
}
