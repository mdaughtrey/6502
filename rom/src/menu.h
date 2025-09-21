#include <stdint.h>

typedef struct
{
    int key;
    const char * help;
    void (*fun)();
}Command;

void menu_handle(uint8_t input);
