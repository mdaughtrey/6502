#include <cstdio>
#include <iostream>
#include <vector>
#include <string>
#include <regex>
#include <types.h>
#include "menu.h"
#include "cmd_io.h"
#include "rom_ram.h"
#include "validator.h"

namespace menu
{

typedef struct
{
    int key;
    const char * help;
    Validator validator;
    bool (*fun)(CommandInput);
}Command;

void handle(uint8_t input);
bool cmd_help(CommandInput input);

InputState state = COMMAND;
// Validator validator = Validator();
// NumericValidator numeric_validator = NumericValidator();
// HexValidator hex_validator = HexValidator();
// PinStateValidator pin_state_validator = PinStateValidator();
// AddrLenValidator addr_len_validator = AddrLenValidator();

extern Command commands_top[];
extern Command commands_io[];
extern Command commands_rom_ram[];
Command * command_set = commands_top;
Command * current_command = NULL;
// std::string accumulator = "";

Command commands_top[] = {
{'h', "help", Validator("*"), cmd_help },
{'i', "I/O Menu", Validator("*"), [](CommandInput) -> bool{ command_set = commands_io; return false; }},
{'r', "ROM/RAM Menu", Validator("*"), [](CommandInput) -> bool{ command_set = commands_rom_ram; return false; }},
{0x01, "", Validator("*"), [](CommandInput input)->bool{ return false; } }
};

Command commands_io[] = {
{
    'a', "Assert Address Bus (Hex)",
    Validator("[0-9a-fA-F]{4}", "Enter addr XXXX"),
    cmd_io::cmd_assert_address_bus 
},
{'b', "Bus Inactive", Validator("*"), cmd_io::cmd_bus_inactive },
{'B', "Bus Active", Validator("*"), cmd_io::cmd_bus_active },
{'c', "Clock Line Low", Validator(""), cmd_io::cmd_clock_line_low },
{'C', "Clock Line High", Validator(""), cmd_io::cmd_clock_line_high },
{
    'f', "Set Clock Frequency",
    Validator("(\\d+)", "Enter a frequency"),
    cmd_io::cmd_set_clock_frequency 
},
{
    'd', "Assert Databus (Hex)",
    Validator("[0-9a-fA-F]{2}", "Enter hex value"),
    cmd_io::cmd_assert_databus 
},
{'D', "Deassert Databus", Validator(""), cmd_io::cmd_deassert_databus },
{'h', "help", Validator(""), cmd_help },
{
    'i', "I/O Value",
    Validator("([io])(\\d\\d)([01])", "[io]Pin[01] (i|o)NN(1|0)"),
    cmd_io::cmd_io 
},
{'m', "memory dump (XXXX/XXXX remembered)", Validator("[0-9a-fA-F]{4}/[0-9a-fA-F]{4}"), cmd_io::cmd_dump_memory },
{'M', "memory dump on clock (XXXX/XXXX remembered)", Validator("[0-9a-fA-F]{4}/[0-9a-fA-F]{4}"), cmd_io::cmd_dump_memory_on_clock },
//{'m', "Run Memory Operation", validator,cmd_io::cmd_memory_operation },
//{'M', "Run Memory Operation on Clock", validator, cmd_io::cmd_memory_operation_on_clock },
{'I', "Init Buses", Validator(""), cmd_io::cmd_init_buses },
{'p', "Pin Status", Validator(""), cmd_io::cmd_pin_status },
{'P', "Pin Status on Clock", Validator(""), cmd_io::cmd_pin_status_on_clock },
{'R', "Reset", Validator(""), cmd_io::cmd_reset },
{'s', "Step Clock", Validator(""), cmd_io::cmd_step_clock },
{'w', "Write Enable Low (write)", Validator("*"), cmd_io::cmd_we_lo },
{'W', "Write Enable High (read)", Validator("*"), cmd_io::cmd_we_hi },
{'x', "Main Menu", Validator(""), [](CommandInput)->bool { command_set = commands_top; return false; }},
{0x01, "", Validator(""), [](CommandInput input)->bool { return false;} }
};

Command commands_rom_ram[] = {
// {'b', "CPU Boot Address (XXXX)", hex_validator, rom_ram::cmd_cpu_boot_address },
{
    'd', "Dump Memory (XXXX)",
    Validator("([0-9a-fA-F]{4})/([0-9a-fA-F]{4})", "Enter addr/length (XXXX/XXXX)"),
    rom_ram::cmd_dump_memory
},
{'h', "help", Validator(""), cmd_help },
{'l', "List Programs", Validator(""), rom_ram::cmd_list_programs },
// {'u', "Upload ROM", hex_validator, rom_ram::cmd_upload_rom },
{
    'p', "Load Program to Memory (NN/XXXX)",
    Validator("(\\d+)/([0-9a-fA-F]{4})", "Program number/Target Address N+/XXXX"),
    rom_ram::cmd_load_program_to_memory 
},
// {'P', "ROM to Program", numeric_validator, rom_ram::cmd_rom_to_program },
{'x', "Main Menu", Validator(""), [](CommandInput)->bool { command_set = commands_top; return false; }},
{0x01, "", Validator(""), [](CommandInput input)->bool { return false;} }
};

void handle(uint8_t input)
{
    if (INTERACTIVE_INPUT == state)
    {
        if (input == 0x0d)
        {
            CommandInput matches = current_command->validator.validate();
            if (matches.empty())
            {
                printf("\r\nInvalid input, looking for %s\r\n", current_command->validator.expecting());
            }
            else
            {
                printf("\r\n");
                current_command->fun(matches);
            }
//            current_command->validator.clear();
            state = COMMAND;
        }
        else if (input == 0x03)
        {
//            current_command->validator.clear();
            state = COMMAND;
            printf("Cancelled\r\n");
        }
        else if (input == 0x7f)
        {
            current_command->validator.deletelast();
            printf("\r                                        \r%s> %s", current_command->validator.prompt().c_str(), current_command->validator.accumulated().c_str());
        }
        else
        {
            current_command->validator.accumulate(input);
            printf("%c", input);
        }
        return;
    }
    for (Command * iter = command_set; iter->key != 0x01; iter++)
    {
        if (iter->key == input)
        {
            current_command = iter;

            if (iter->fun(CommandInput()))
            {
//                std::cout << "Command " << iter->key << " prompt is [" << current_command->validator.prompt() << "]" << std::endl;
                if (!current_command->validator.prompt().empty())
                {
                    printf("%s: %s", current_command->validator.prompt().c_str(), current_command->validator.accumulated().c_str());
//                    std::cout << "> " << current_command->validator.prompt() << ": " << std::ends;
                }
                state = INTERACTIVE_INPUT;
            }
            else
            {
//                current_command->validator.clear();
                state = COMMAND;
                std::cout << "> " << current_command->validator.accumulated();
//                accumulator = "";
            }
            break;
        }
    }
}

bool cmd_help(CommandInput input = CommandInput())
{   
    for (Command * iter = command_set; iter->key != 0x01; iter++)
    {
        printf("%c: %s\r\n", iter->key, iter->help);
    }
    return false;
}
} // namespace menu
