#include <cstdio>
#include <string>
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
    Validator & validator;
    bool (*fun)(std::string);
}Command;

void handle(uint8_t input);
bool cmd_help(std::string input);

InputState state = COMMAND;
Validator validator = Validator();
NumericValidator numeric_validator = NumericValidator();
HexValidator hex_validator = HexValidator();
PinStateValidator pin_state_validator = PinStateValidator();
AddrLenValidator addr_len_validator = AddrLenValidator();

extern Command commands_top[];
extern Command commands_io[];
extern Command commands_rom_ram[];
Command * command_set = commands_top;
Command * current_command = NULL;
// std::string accumulator = "";

Command commands_top[] = {
{'h', "help", validator, cmd_help },
{'i', "I/O Menu", validator, [](std::string) -> bool{ command_set = commands_io; return false; }},
{'r', "ROM/RAM Menu", validator, [](std::string) -> bool{ command_set = commands_rom_ram; return false; }},
{0x01, "", validator, [](std::string input)->bool{ return false; } }
};

Command commands_io[] = {
{'c', "Set Clock Frequency", numeric_validator, cmd_io::cmd_set_clock_frequency },
{'d', "Assert Databus (Hex)", hex_validator, cmd_io::cmd_assert_databus },
{'D', "Assert Databus via Pullups (Hex)", hex_validator, cmd_io::cmd_assert_databus_via_pullups },
{'h', "help", validator, cmd_help },
{'i', "I/O Command", pin_state_validator, cmd_io::cmd_io },
{'m', "Enable Memory Operations", validator,cmd_io::cmd_enable_memory },
{'M', "Disable Memory Operations", validator, cmd_io::cmd_disable_memory },
{'I', "Init Buses", pin_state_validator, cmd_io::cmd_init_buses },
{'p', "Pin Status", validator, cmd_io::cmd_pin_status },
{'P', "Pin Status on Clock", validator, cmd_io::cmd_pin_status_on_clock },
{'s', "Step Clock", validator, cmd_io::cmd_step_clock },
{'r', "Reset", validator, cmd_io::cmd_reset },
{'x', "Main Menu", validator, [](std::string)->bool { command_set = commands_top; return false; }},
{0x01, "", validator, [](std::string input)->bool { return false;} }
};

Command commands_rom_ram[] = {
{'d', "Dump Memory (RAM 0000-7fff, ROM 8000-ffff)", addr_len_validator, rom_ram::cmd_dump_memory },
{'h', "help", validator, cmd_help },
{'l', "List Programs", validator, rom_ram::cmd_list_programs },
{'u', "Upload ROM", hex_validator, rom_ram::cmd_upload_rom },
{'p', "Load Program  to ROM", numeric_validator, rom_ram::cmd_load_program_to_rom },
{'P', "ROM to Program", numeric_validator, rom_ram::cmd_rom_to_program },
{'x', "Main Menu", validator, [](std::string)->bool { command_set = commands_top; return false; }},
{0x01, "", validator, [](std::string input)->bool { return false;} }
};

void handle(uint8_t input)
{
    if (INTERACTIVE_INPUT == state)
    {
        if (input == 0x0d)
        {
            current_command->fun(current_command->validator.accumulated());
            current_command->validator.clear();
            state = COMMAND;
        }
        else if (input == 0x1b)
        {
            current_command->validator.clear();
            state = COMMAND;
        }
        else if (current_command->validator.validate(input))
        {
            printf("%c", input);
//            accumulator += input;
        }
        else
        {
            printf("Invalid input, looking for %s\r\n", current_command->validator.expecting());
            current_command->validator.clear();
            state = COMMAND;
        }
        return;
    }
    for (Command * iter = command_set; iter->key != 0x01; iter++)
    {
        if (iter->key == input)
        {
            current_command = iter;

            if (iter->fun(iter->validator.accumulated()))
            {
                state = INTERACTIVE_INPUT;
            }
            else
            {
                current_command->validator.clear();
                state = COMMAND;
//                accumulator = "";
            }
            break;
        }
    }
}

bool cmd_help(std::string input)
{   
    for (Command * iter = command_set; iter->key != 0x01; iter++)
    {
        printf("%c: %s\r\n", iter->key, iter->help);
    }
    return false;
}
} // namespace menu
