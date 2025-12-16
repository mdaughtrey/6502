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
#include "cmd_io.h"
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
    void assert_address_bus(uint16_t addr);
    void assert_databus(uint8_t data);
    void set_databus_out(bool out);

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
        bool be_low = false;
        if (!gpioc_hilo_in_get() & cmd_io::BE_MASK)
        {
            be_low = true;
        }

        if (!be_low)
        {
            gpio_put(PIN_BUS_ENABLE, BE_INACTIVE);
        }
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
        if (!be_low)
        {
            gpio_put(PIN_BUS_ENABLE, BE_ACTIVE);
        }
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
        for (auto ii = 0; ii < length; ii++)
        {
            assert_address_bus(target_address + ii);
            assert_databus(data[ii]);
            sleep_us(5);
            gpio_put(PIN_RW, 0);
            sleep_us(5);
            gpio_put(PIN_RW, 1);
            sleep_us(5);
        }
        gpio_set_dir_masked64(mask, 0);
        gpio_put(PIN_BUS_ENABLE, BE_ACTIVE);
    }

    bool cmd_upload_rom_image(CommandInput input = CommandInput())
    {
        printf("Loading %d bytes to %04x...", rom1_bin_len, 0x0000);
        write_to_memory(rom1_bin, rom1_bin_len, 0x0000);
        printf(" Done.\r\n");
        return false;
    }

    void assert_address_bus(uint16_t addr)
    {
        cmd_io::set_address_bus_out(true);
        gpio_put_masked64(cmd_io::ADDR_MASK, static_cast<uint64_t>(addr));
    }

    void assert_databus(uint8_t data)
    {
//        for (auto ii = 40; ii < 48; ii++)
//        {
//            gpio_init(ii);
//        }
        set_databus_out(true);
//        std::cout << "Data " << std::bitset<64>(static_cast<uint64_t>(data) << 40) << std::endl << "Mask " << std::bitset<64>(DATA_MASK) << std::endl;
        for (auto ii = 0; ii < 8; ii++)
        {
            if (data & (1 << ii))
            {
                gpio_put(ii + 40, 1);
            }
            else
            {
                gpio_put(ii + 40, 0);
            }
                    
        }
//        gpio_put_masked64(DATA_MASK, static_cast<uint64_t>(data) << 40);
    }
    inline void set_databus_out(bool out)
    {
//        std::cout << "set_databus_out " << out << " BEFORE " << std::bitset<32>(sio_hw->gpio_hi_oe) << std::bitset<32>(sio_hw->gpio_oe) << std::endl;
//        for (auto ii = 40; ii < 48; ii++)
//        {
//            gpio_set_dir(ii, GPIO_OUT);
//        }
        if (out)
            gpio_set_dir_masked64(cmd_io::DATA_MASK, cmd_io::DATA_MASK);
        else
          gpio_set_dir_masked64(cmd_io::DATA_MASK, 0);
//        std::cout << "set_databus_out " << out << " AFTER " << std::bitset<32>(sio_hw->gpio_hi_oe) << std::bitset<32>(sio_hw->gpio_oe) << std::endl;
    }

}
