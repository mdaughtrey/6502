#include <cstring>
#include <cstdio>
#include "pico/stdlib.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>

#include "rom_ram.h"

namespace rom_ram
{
    typedef struct
    {
        const char * title;
        uint16_t length;
        uint8_t code[256];
    } Program;

    Program programs[] = {
        {"raminc", 16, {0xa9, 0x55, 0x85, 0x00, 0xe6, 0x00, 0x4c, 0x04, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
        {NULL, {0} }
    };

    void dump_memory(uint8_t * memory, uint16_t addr, uint16_t length);

    void init(void)
    {
    }

    void loop(void)
    {
    }

    bool cmd_upload_rom(std::string value)
    {
        return false;
    }

    bool cmd_program_to_rom(std::string value)
    {
        return false;
    }
    bool cmd_dump_memory(std::string value)
    {
        if (value.empty())
        {
            printf("Enter addr/length XXXX/XXXX: ");
            return true;
        }
        uint16_t addr = std::stoi(value.substr(0, 4), nullptr, 16);
        uint16_t length = std::stoi(value.substr(5, 4), nullptr, 16);
        if (addr >= RR_ROM_BASE)
        {
            dump_memory(rom_ram::ROM, addr - rom_ram::RR_ROM_BASE, length);
        }
        else
        {
            dump_memory(rom_ram::RAM, addr, length);
        }
        return false;
    }

    void dump_memory(uint8_t * memory, uint16_t addr, uint16_t length)
    {
        uint16_t lines = (length + 15) / 16;

        for (auto line = 0; line < lines; line++)
        {
            std::stringstream linetext;
            linetext << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << addr + (line * 16) << ": ";
            for (auto i = 0; i < 16; i++)
            {
                linetext << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << static_cast<uint16_t>(memory[addr + (line * 16) + i]) << " ";
            }
            linetext << "  ";
            for (auto i = 0; i < 16; i++)
            {
                auto c = addr + (line * 16) + i;
                if (c >= 32 && c <= 126)
                {
                    linetext << static_cast<char>(c);
                }
                else
                {
                    linetext << ".";
                }
            }
            std::cout << linetext.str() << std::endl;
        }
    }
    bool cmd_rom_to_program(std::string value)
    {
        return false;
    }
    bool cmd_list_programs(std::string value)
    {
        Program * iter;
        uint8_t count = 0;
        for (iter = programs; iter->title != NULL; iter++)
        {
            printf("%u. %s\n", count++, iter->title);
        }
        return false;
    }
    bool cmd_load_program_to_rom(std::string value)
    {
        if (value.empty())
        {
            printf("Enter program number (0-%u: ", sizeof(programs) - 1);
            return true;
        }
        uint8_t program_number = std::stoi(value);

        Program * iter = &programs[program_number];
        memcpy(ROM, iter->code, iter->length);
        printf("Loaded %s to ROM\n", iter->title);
        ROM[0xffff] = 0x80;
        ROM[0xfffe] = 0x00;
        ROM[0xfffd] = 0x80;
        ROM[0xfffc] = 0x00;
        return false;
    }
}
