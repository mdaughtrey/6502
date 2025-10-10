#include <cstdio>
#include "pico/stdlib.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>



namespace rom_ram
{
    const uint16_t RR_ROM_BASE = 0x8000;
    const uint16_t RR_ROM_SIZE = 0x8000;
    const uint16_t RR_RAM_BASE = 0x0000;
    const uint16_t RR_RAM_SIZE = 0x8000;
    uint8_t RAM[RR_RAM_SIZE];
    uint8_t ROM[RR_ROM_SIZE];
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

    bool cmd_page_to_rom(std::string value)
    {
        return false;
    }
    bool cmd_dump_rom(std::string value)
    {
        if (value.empty())
        {
            printf("Enter addr/length XXXX/XXXX: ");
            return true;
        }
        uint16_t addr = std::stoi(value.substr(0, 4), nullptr, 16);
        uint16_t length = std::stoi(value.substr(5, 4), nullptr, 16);
        uint16_t lines = (length + 15) / 16;

        for (auto line = 0; line < lines; line++)
        {
            std::stringstream linetext;
            linetext << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << addr + (line * 16) << ": ";
            for (auto i = 0; i < 16; i++)
            {
                linetext << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << static_cast<uint16_t>(ROM[addr + (line * 16) + i]) << " ";
            }
            linetext << "  ";
            for (auto i = 0; i < 16; i++)
            {
                auto c = ROM[addr + (line * 16) + i];
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
        return false;
    }
    bool cmd_dump_ram(std::string value)
    {
        return false;
    }
    bool cmd_rom_to_page(std::string value)
    {
        return false;
    }
}
