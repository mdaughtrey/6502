#include <cstdio>
#include <string>
#include "menu.h"
#include "cmd_io.h"

namespace menu
{

class Validator
{
    public:
        virtual bool validate(uint8_t input)
        {
            return true;
        }
};

class NumericValidator : public Validator
{
    public:
        bool validate(uint8_t input)
        {
            return input >= '0' && input <= '9';
        }
};

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

extern Command commands_top[];
extern Command commands_io[];
Command * command_set = commands_top;
Command * current_command = NULL;
std::string accumulator = "";


Command commands_top[] = {
{'h', "help", validator, cmd_help },
{'i', "I/O Menu", validator, [](std::string) -> bool{ command_set = commands_io; return false; }},
{0x01, "", validator, [](std::string input)->bool{ return false; } }
};

Command commands_io[] = {
{'c', "Set Clock Frequency", numeric_validator, cmd_io::cmd_set_clock_frequency },
{'h', "help", validator, cmd_help },
{'p', "Pin Status", validator, cmd_io::cmd_pin_status },
{'s', "Step Clock", validator, cmd_io::cmd_step_clock },
{'r', "Reset", validator, cmd_io::cmd_reset },
{'x', "Main Menu", validator, [](std::string)->bool { command_set = commands_top; return false; }},
{0x01, "", validator, [](std::string input)->bool { return false;} }
};

void handle(uint8_t input)
{
    if (INTERACTIVE_INPUT == state)
    {
        if (input == 0x0d)
        {
            current_command->fun(accumulator);
            state = COMMAND;
            accumulator = "";
        }
        else if (input == 0x1b)
        {
            accumulator = "";
            state = COMMAND;
        }
        else if (current_command->validator.validate(input))
        {
            printf("%c", input);
            accumulator += input;
        }
    }
    for (Command * iter = command_set; iter->key != 0x01; iter++)
    {
        if (iter->key == input)
        {
            current_command = iter;

            if (iter->fun(accumulator))
            {
                state = INTERACTIVE_INPUT;
            }
            else
            {
                state = COMMAND;
                accumulator = "";
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
