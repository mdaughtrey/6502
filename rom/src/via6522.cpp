#include <string>
#include <list>
#include <iostream>
#include <bitset>
#include <vector>
#include "hardware/structs/sio.h"
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/pwm.h"
#include "types.h"
#include "via6522.h"

#include "common_defs.h"
#include "rom_ram_internal.h"

namespace via6522
{

typedef struct
{
    const char * name;
    uint16_t port;
    std::string (*decode)(uint8_t);
} Register;

    std::list<std::string> log_queue;

std::string default_decode(uint8_t data)
{
    char buffer[32];
    sprintf(buffer, "%02x %s", data, std::bitset<8>(data).to_string().c_str());
    return buffer;
}

std::string register_decode(uint8_t data, const char * names[])
{
    std::string buffer;
    char cbuffer[32];
    sprintf(cbuffer, "%02x %s ", data, std::bitset<8>(data).to_string().c_str());
    buffer += cbuffer;
    for (int8_t ii = 7; ii >= 0; ii--)
    {
        if (!(data & (1 << ii)))
        {
            buffer += '~';
        }
        buffer += names[ii];
        buffer += ' ';
    }
    return buffer;
}

std::string ier_decode(uint8_t data)
{
    const char * names[] = {
        "CA2", "CA1", "SR", "CB2", "CB1", "T2", "T1", "SC"
    };
    return register_decode(data, names);
}

std::string ifr_decode(uint8_t data)
{
    const char * names[] = {
        "CA2", "CA1", "SR", "CB2", "CB1", "T2", "T1", "IRQ"
    };
    return register_decode(data, names);
}

std::string acr_decode(uint8_t data)
{
    const char * names[] = {
        "PA", "PB", "SRC", "SRB", "SRA", "T2", "T1B", "T1A"
    };
    return register_decode(data, names);
}

const uint16_t IOBASE = 0x4000;
Register registers[] = {
    { "ORBIRB", IOBASE, default_decode },
    { "ORAIRA", IOBASE + 1, default_decode },
    { "DDRB", IOBASE + 2, default_decode },
    { "DDRA", IOBASE + 3, default_decode },
    { "T1CL", IOBASE + 4, default_decode },
    { "T1CH", IOBASE + 5, default_decode },
    { "T1LL", IOBASE + 6, default_decode },
    { "T1LH", IOBASE + 7, default_decode },
    { "T2CL", IOBASE + 8, default_decode },
    { "T2CH", IOBASE + 9, default_decode },
    { "SR",   IOBASE + 10, default_decode },
    { "ACR",  IOBASE + 11, acr_decode },
    { "PCR",  IOBASE + 12, default_decode },
    { "IFR",  IOBASE + 13, ifr_decode },
    { "IER",  IOBASE + 14, ier_decode },
    { "ORAIRA0", IOBASE + 1, default_decode },
    { "STOP", 0, default_decode }
};

// ; Auxiliary Control Register
// ACR_T1A         = %10000000     ; Timer1 Control A
// ACR_T1B         = %01000000     ; Timer1 Control B
// ACR_T2          = %00100000     ; Timer2 Control 
// ACR_SRA         = %00010000     ; Shift Register Control A
// ACR_SRB         = %00001000     ; Shift Register Control B
// ACR_SRC         = %00000100     ; Shift Register Control C
// ACR_PB          = %00000010     ; Peripheral B Latch Enable
// ACR_PA          = %00000001     ; Peripheral A Latch Enable
//
// ; Interrupt Enable Register
// IER_SET_CLEAR   = %1000000
// IER_TIMER1      = %01000000     ; Timer1
// IER_TIMER2      = %00100000     ; Timer2
// IER_CB1         = %00010000     ; Peripheral B Control 1 (read handshake)
// IER_CB2         = %00001000     ; Peripheral B Control 2 (write handshake)
// IER_SHIFT_REG   = %00000100     ; Shift register
// IER_CA1         = %00000010     ; Peripheral A Control 1 (read handshake)
// IER_CA2         = %00000001     ; Peripheral A Control 2 (write handshake)
// 

    void init(void)
    {
    }

    void loop(void)
    {
        for (auto iter = log_queue.begin(); iter != log_queue.end(); iter++)
        {
            std::cout << *iter << std::endl;
        }
        log_queue.clear();
    }

    bool cmd_dump_registers(CommandInput input = CommandInput())
    {
        std::vector<uint8_t> rdata = rom_ram::read_memory(static_cast<uint32_t>(IOBASE), static_cast<uint32_t>(IOBASE+14));
        for (uint8_t ii = 0; ii < 16; ii++)
        {
            char buffer[128];
            sprintf(buffer, "%8s %s", registers[ii].name, registers[ii].decode(rdata[ii]).c_str());
            log_queue.push_back(buffer);
        }
        return false;
    }

    bool cmd_set_io_base(CommandInput input = CommandInput())
    {
        if (input.empty())
        {
            return true;
        }
        return false;
    }

    bool cmd_set_register(CommandInput input = CommandInput())
    {
        if (input.empty())
        {
            return true;
        }
        uint8_t regindex = std::stoi(input[1], nullptr, 16);
        uint8_t data = std::stoi(input[2], nullptr, 16);
        printf("regindex %02x data %02x\r\n", regindex, data);
        rom_ram::write_to_memory(&data, 1, IOBASE + regindex);
        return false;
    }

} // via6522
