#include <stdio.h>
#include <stdlib.h>
#include <menu.h>

void cmd_hello();
void cmd_help();

Command commands_main[] = {
{'h', "help", cmd_help },
{'l', "Hello World", cmd_hello },
{0x01, "", [](){} }
};

Command * command_set = commands_main;

void menu_handle(uint8_t input)
{
    for (Command * iter = command_set; iter->key != '&'; iter++)
    {
        if (iter->key == input)
        {
            iter->fun();
            return;
        }
    }
}

void cmd_help()
{   
    for (Command * iter = command_set; iter->key != 0x01; iter++)
    {
        printf("%c: %s\r\n", iter->key, iter->help);
    }
}

void cmd_hello()
{
    printf("Hello from cmd_hello\r\n");
}
