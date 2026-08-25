#include <algorithm>
#include <bitset>
#include <cstring>
#include <cstdio>
#include "pico/stdlib.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include "types.h"
#include "rom_ram.h"
#include "pin_defs.h"
#include "cmd_io.h"
#include "ioutils.h"
// #include "varstacktest.h"
#include "bus_asserts.h"
#include "pin_scope.h"
#include "log_queue.h"

namespace rom_ram
{
    typedef struct
    {
        const char * title;
        uint16_t target;
        uint16_t length;
        uint8_t code[256];
    } Program;

    uint32_t rw_delay_us = 0;

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

    std::vector<uint8_t> read_memory(uint32_t address, uint32_t length);
    std::string dump_memory(uint16_t addr, uint16_t length);
    void write_memory(uint8_t * data, uint32_t length, uint16_t target_address);
    

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
        std::cout << dump_memory(addr, length) << std::endl;
        return false;
    }

    std::vector<uint8_t> read_memory(uint32_t address, uint32_t length)
    {
        std::vector<uint8_t> data(length, 0);
        PinScopeAddressRead scope;
        auto iter = data.begin();
        for (uint32_t addr = address; addr < address + length; addr++)
        {
            if (addr > 0xffff)
            {
                printf("read_memory: trying to access %04x\r\n", addr);
                return std::vector<uint8_t>(1, 0);
            }
            gpio_put_masked64(cmd_io::ADDR_MASK, addr);
            if (rw_delay_us) sleep_us(rw_delay_us);
            (*iter) = static_cast<uint8_t>(gpioc_hilo_in_get() >> 40);
//            VERBOSE("%04x -> %02x\r\n", addr, (*iter));
            if (rw_delay_us) sleep_us(rw_delay_us);
            iter++;
        }
        return data;
    }

    std::string dump_memory(uint16_t address, uint16_t length)
    {
        std::stringstream linetext;
        std::vector<uint8_t> data = read_memory(address, length);

        uint16_t lines = (length + 15) / 16;

        std::vector<uint8_t> dataline;
        for (auto line = 0; line < lines; line++)
        {
            linetext << std::endl << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << address + line * 16 << ": ";
            dataline.clear();
            for (auto i = 0; i < 16; i++)
            {
                if (((line * 16) + 1) < length)
                {
                    uint8_t datum = data[(line * 16) + i];
                    dataline.push_back(datum);
                    linetext << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << static_cast<int>(datum) << " ";
                }
            }
            linetext << "  ";
            for (auto iter = dataline.begin(); iter != dataline.end(); iter++)
            {
                if ((*iter) >= 32 && (*iter) <= 126)
                {
                    linetext << static_cast<char>(*iter);
                }
                else
                {
                    linetext << ".";
                }
            }
        }
        return linetext.str();
    }

    std::string dump_memory0(uint16_t address, uint16_t length)
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
        write_memory(iter->code, iter->length, iter->target);
        write_memory(reinterpret_cast<uint8_t *>(&iter->target), 2, 0xfffc);
        return false;
    }

    void write_memory(uint8_t * data, uint32_t length, uint16_t target_address)
    {
        PinScopeAddressWrite scope;
        for (auto ii = 0; ii < length; ii++)
        {
            if ((target_address + ii) > 0xffff)
            {
                printf("write_memory: trying to access %04x\r\n", target_address + ii);
                return;
            }
            uint64_t towrite = (target_address + ii) | (static_cast<uint64_t>(data[ii]) << PIN_DATA0);
            VERBOSE("write_memory:mask64: %s", std::bitset<64>(towrite).to_string().c_str());
            gpio_put_masked64(cmd_io::ADDR_MASK | cmd_io::DATA_MASK, (target_address + ii) | (static_cast<uint64_t>(data[ii]) << PIN_DATA0));

            if (rw_delay_us) sleep_us(rw_delay_us);

            gpio_put(PIN_RW, 0);
            if (rw_delay_us) sleep_us(rw_delay_us);
            gpio_put(PIN_RW, 1);
            if (rw_delay_us) sleep_us(rw_delay_us);
        }
    }

    bool cmd_upload_rom_image(CommandInput input = CommandInput())
    {
        printf("Loading %d bytes to %04x...", rom1_bin_len, 0x0000);
        write_memory(rom1_bin, rom1_bin_len, 0x0000);
        printf(" Done.\r\n");
        return false;
    }

    bool cmd_upload_test_image(CommandInput input = CommandInput())
    {
        uint8_t rom_image[65536];
        printf("Uploading test image...");
        for (auto ii = 0; ii < 65536; ii++)
        {
            rom_image[ii] = ii % 256;
        }
//        while (1)
        write_memory(rom_image, 65536, 0x0000);
        printf(" Done.\r\n");
        return false;
    }

    bool cmd_memory_test(CommandInput input = CommandInput())
    {
        uint8_t rom_image[65536];
        printf("Uploading test image...");
        for (auto ii = 0; ii < 65536; ii++)
        {
            rom_image[ii] = ii % 256;
        }
        write_memory(rom_image, 65536, 0x0000);
        std::vector<uint8_t> ram_image = read_memory(0, 65536);
        for (auto ii = 0; ii < 65536; ii++)
        {
            if (ram_image[ii] != ii % 256)
            {
                printf("Failed %04x: ROM %02x RAM %02x\r\n", ii, ii % 256, ram_image[ii]);
            }
        }
        return false;
    }

    bool cmd_memory_test_slow(CommandInput input = CommandInput())
    {
        for (auto ii = 0; ii < 65536; ii++)
        {
            uint8_t data(ii % 256);;
            printf("Write to %04x\r\n", ii);
            write_memory(&data, 1, ii);
            std::vector<uint8_t> got_back(read_memory(ii, 1));
            printf("Read %02x from  %04x\r\n", got_back[0], ii);
            if (got_back[0] != ii % 256)
            {
                printf("Failed %04x: got back %02x\r\n", ii, got_back[0]);
            }
        }
        printf("Complete\r\n");
        return false;
    }
    

    bool cmd_write_memory(CommandInput input = CommandInput())
    {
        if (input.empty())
        {
            return true;
        }
        uint16_t addr = std::stoi(input[1], nullptr, 16);
        uint8_t data = std::stoi(input[2], nullptr, 16);
        write_memory(&data, 1, addr);
        return false;
    }

    bool cmd_flood_ram(CommandInput input = CommandInput())
    {
        if (input.empty())
        {
            return true;
        }
        for (auto iter = input.begin(); iter != input.end(); iter++)
        {
            printf("input %s\r\n", iter->c_str());
        }
        uint8_t flood = std::stoi(input[0], nullptr, 16);
        uint8_t rom_image[65536];
        printf("Uploading test image...");
        for (auto ii = 0; ii < 65536; ii++)
        {
            rom_image[ii] = flood;
        }
        write_memory(rom_image, 65536, 0x0000);
        printf("Verifying...\r\n");
        std::vector<uint8_t> ram_image = read_memory(0, 65536);
        for (auto ii = 0; ii < 65536; ii++)
        {
            if (ram_image[ii] != flood)
            {
                printf("Failed %04x: %02x\r\n", ii, ram_image[ii]);
            }
        }
        printf("Complete\r\n");
        return false;
    }

    bool cmd_rw_memory_delay(CommandInput input = CommandInput())
    {
        if (input.empty())
        {
            return true;
        }
        rw_delay_us = std::stol(input[0], nullptr, 10);
        VERBOSE("rw_delay_us is %u\r\n", rw_delay_us);
        return false;
    }

}
