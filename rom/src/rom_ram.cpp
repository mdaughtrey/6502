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

namespace rom_ram
{
    typedef struct
    {
        const char * title;
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
        {"raminc", 6, {0xe6, 0x00, 0x4c, 0x00, 0xc0, 0x00} },
        {"boot vector", 2, {0x00, 0xc0} },
        {NULL, {0} }
    };

    void dump_memory(uint16_t addr, uint16_t length);
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
        gpio_put(PIN_BUS_ENABLE, BE_INACTIVE);

        for (auto iter = input.begin(); iter != input.end(); iter++)
        {
            printf("%s ", iter->c_str());
        }

        if (input.empty())
        {
            printf("Enter addr/length XXXX/XXXX: ");
            return true;
        }
        printf("\r\n");
        uint16_t addr = std::stoi(input[1], nullptr, 16);
        uint16_t length = std::stoi(input[2], nullptr, 16);
        printf("addr %04x length %04x\r\n", addr, length);
        dump_memory(addr, length);
        gpio_put(PIN_BUS_ENABLE, BE_ACTIVE);
        return false;
    }

    void dump_memory(uint16_t address, uint16_t length)
    {
        uint64_t mask = cmd_io::ADDR_MASK | cmd_io::RW_MASK;
        gpio_set_dir_masked64(mask, mask);
        gpio_set_dir_masked64(cmd_io::DATA_MASK, 0);
        gpio_put(PIN_BUS_ENABLE, BE_INACTIVE);

        length = std::min(length, static_cast<uint16_t>(0xffff-address));

        uint16_t lines = (length + 15) / 16;

        for (auto line = 0; line < lines; line++)
        {
            std::stringstream linetext;
            linetext << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << address + line * 16 << ": ";
            uint8_t dataline[16];
            for (auto i = 0; i < 16; i++)
            {
                cmd_io::assert_address_bus(address + (line * 16) + i);
                sleep_us(1);
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
            std::cout << linetext.str() << std::endl;
        }
        gpio_set_dir_masked64(mask, 0);
        gpio_put(PIN_BUS_ENABLE, BE_ACTIVE);
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
            printf("Program number/Target Address 0-1/XXXX: ");
            return true;
        }
        std::cout << std::endl;
        uint8_t program_number = std::stoi(input[1]);

        Program * iter = &programs[program_number];
        write_to_memory(iter->code, iter->length, std::stoi(input[2], nullptr, 16));
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
            cmd_io::pin_status();
            sleep_us(1);
            printf("%04x <- %02x\r\n", target_address + ii, data[ii]);
            gpio_put(PIN_RW, 0);
            sleep_us(1);
            gpio_put(PIN_RW, 1);
            sleep_us(1);
        }
//        sleep_ms(1);
        gpio_set_dir_masked64(mask, 0);
        gpio_put(PIN_BUS_ENABLE, BE_ACTIVE);
        cmd_io::pin_status();
    }

//    bool cmd_cpu_boot_address(std::string value)
//    {
//        if (value.empty())
//        {
//            printf("Enter CPU boot address (XXXX): ");
//            return true;
//        }
//        std::cout << std::endl;
//        uint16_t address = std::stoi(value, nullptr, 16);
//        write_to_memory(reinterpret_cast<uint8_t *>(&address), 2, 0xfffc);
//        return false;
//    }

}
