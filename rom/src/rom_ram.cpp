#include <algorithm>
#include <cstring>
#include <cstdio>
#include "pico/stdlib.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>

#include "types.h"
#include "rom_ram.h"
#include "pin_defs.h"
#include "cmd_io_internal.h"
#include "ioutils.h"
// #include "varstacktest.h"

namespace rom_ram
{
    typedef struct
    {
        const char * title;
        uint16_t target;
        uint16_t length;
        uint8_t code[256];
    } Program;

//    const uint16_t RR_ROM_BASE = 0x8000;
//    const uint16_t RR_ROM_SIZE = 0x8000;
//    const uint16_t RR_RAM_BASE = 0x0000;
//    const uint16_t RR_RAM_SIZE = 0x8000;
//    uint8_t RAM[RR_RAM_SIZE];
//    uint8_t ROM[RR_ROM_SIZE];
//    uint8_t MEMORY[4096];

    Program programs[] = {
//        {"raminc", 16, {0xa9, 0x55, 0x85, 0x00, 0xe6, 0x00, 0x4c, 0x04, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
//        {"raminc", 16, {0xa9, 0x55, 0x85, 0x00, 0xe6, 0x00, 0x4c, 0x04, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
        {"raminc", 0xc000, 6, {0xe6, 0x00, 0x4c, 0x00, 0xc0, 0x00} },
        {"ioselect", 0xc100, 33, {0xa9, 0xff, 0x8d, 0x03, 0x40, 0x8d, 0x02, 0x40, 0xa9, 0x55, 0x8d, 0x01, 0x40, 0xa9, 0xaa, 0x8d,
                                  0x00, 0x40, 0xa9, 0xaa, 0x8d, 0x01, 0x40, 0xa9, 0x55, 0x8d, 0x00, 0x40, 0x4c, 0x08, 0xc1, 0x00,
                                  0x00 }},
        {"chaser", 0xc100, 51, {
            0xa2, 0x00, 0x86, 0x00, 0xe8, 0x86, 0x01, 0xa9, 0xff, 0x8d, 0x02, 0x40,
            0x8d, 0x03, 0x40, 0xa9, 0xff, 0x45, 0x01, 0x8d, 0x00, 0x40, 0xa9, 0xff,
            0x45, 0x00, 0x8d, 0x01, 0x40, 0x06, 0x01, 0x90, 0x07, 0xa9, 0x01, 0x85,
            0x00, 0x4c, 0x0f, 0xc1, 0x06, 0x00, 0x90, 0xe3, 0xa9, 0x01, 0x85, 0x01,
            0x4c, 0x0f, 0xc1 }},
        {"boot vector", 0xfffc, 2, {0x00, 0xc0}},
        {NULL, {0} }
    };

    std::string dump_memory(uint16_t addr, uint16_t length);
    void write_to_memory(uint8_t * data, uint16_t length, uint16_t target_address);

    void init(void)
    {
    }

    void loop(void)
    {
    }

    bool cmd_upload_rom(CommandInput input)
    {
        return false;
    }

    bool cmd_program_to_rom(CommandInput input)
    {
        return false;
    }
    bool cmd_dump_memory(CommandInput input)
    {
        if (input.empty())
        {
            return true;
        }
        uint16_t addr = std::stoi(input[1], nullptr, 16);
        uint16_t length = std::stoi(input[2], nullptr, 16);
//        printf("addr %04x length %04x\r\n", addr, length);
        std::cout << dump_memory(addr, length) << std::endl;
        return false;
    }

    std::string dump_memory(uint16_t address, uint16_t length)
    {
        std::stringstream linetext;

        gpio_put(PIN_BUS_ENABLE, BE_INACTIVE);
        uint64_t mask = cmd_io::ADDR_MASK | cmd_io::RW_MASK;
        gpio_set_dir_masked64(mask, mask);
        gpio_put(PIN_RW, 1);        // Read

        length = std::min(length, static_cast<uint16_t>(0xffff-address));

        uint16_t lines = (length + 15) / 16;

        for (auto line = 0; line < lines; line++)
        {
            linetext << std::endl << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << address + line * 16 << ": ";
            uint8_t dataline[16];
            for (auto i = 0; i < 16; i++)
            {
//                cmd_io::assert_address_bus(address + (line * 16) + i);
                gpio_put_masked64(cmd_io::ADDR_MASK, address + (line * 16) + i);
                sleep_us(10);
//                uint64_t data64 = gpioc_hilo_in_get();
//                printf("data64 %016x\r\n", data64);
//                data64 = gpioc_hilo_in_get();
//                printf("data64 %016x\r\n", data64);
                uint8_t data = static_cast<uint8_t>(gpioc_hilo_in_get() >> 40);
                dataline[i] = data;
                linetext << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << static_cast<int>(data) << " ";
            }
            linetext << "  ";
            for (auto i = 0; i < 16; i++)
            {
                auto c = dataline[i];
                if (c >= 32 && c <= 126)
                {
                    linetext << static_cast<char>(c);
                }
                else
                {
                    linetext << ".";
                }
            }
//            linetext << std::endl;
        }
        gpio_set_dir_masked64(mask, 0);
        gpio_put(PIN_BUS_ENABLE, BE_ACTIVE);
        return linetext.str();
    }

//    bool cmd_rom_to_program(CommandInput value)
//    {
//        return false;
//    }
//
    bool cmd_list_programs(CommandInput input = CommandInput())
    {
        Program * iter;
        uint8_t count = 0;
        for (iter = programs; iter->title != NULL; iter++)
        {
            printf("%u. %s\n", count++, iter->title);
        }
        std::cout << std::endl;
        return false;
    }

    bool cmd_load_program_to_memory(CommandInput input = CommandInput())
    {
        if (input.empty())
        {
            return true;
        }
        uint8_t program_number = std::stoi(input[1]);

        Program * iter = &programs[program_number];
        write_to_memory(iter->code, iter->length, iter->target);
        write_to_memory(reinterpret_cast<uint8_t *>(&iter->target), 2, 0xfffc);
        return false;
    }

    void write_to_memory(uint8_t * data, uint16_t length, uint16_t target_address)
    {
        uint64_t mask = cmd_io::ADDR_MASK | cmd_io::DATA_MASK | cmd_io::RW_MASK;
        gpio_put(PIN_BUS_ENABLE, BE_INACTIVE);
        gpio_set_dir_masked64(mask, mask);
//        gpio_set_dir(PIN_RW, GPIO_OUT);
        for (auto ii = 0; ii < length; ii++)
        {
//            gpio_put(PIN_RW, 0);
//            sleep_us(1);
            cmd_io::assert_address_bus(target_address + ii);
            cmd_io::assert_databus(data[ii]);
//            cmd_io::pin_status();
            sleep_us(1);
//            printf("%04x <- %02x\r\n", target_address + ii, data[ii]);
            gpio_put(PIN_RW, 0);
            sleep_us(1);
            gpio_put(PIN_RW, 1);
            sleep_us(1);
        }
//        sleep_ms(1);
        gpio_set_dir_masked64(mask, 0);
        gpio_put(PIN_BUS_ENABLE, BE_ACTIVE);
//        cmd_io::pin_status();
    }

    bool cmd_upload_rom_image(CommandInput input = CommandInput())
    {
        printf("Loading %d bytes to %04x...", rom1_bin_len, 0x0000);
        write_to_memory(rom1_bin, rom1_bin_len, 0x0000);
        printf(" Done.\r\n");
        return false;
    }
}
