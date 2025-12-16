#include <string>
#include <functional>
#include <pico/stdlib.h>

namespace rom_ram
{
    std::string dump_memory(uint16_t addr, uint16_t length);
    bool cmd_assert_address_bus(CommandInput);
}
