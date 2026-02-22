#include <string>
#include <functional>
#include <vector>
#include <pico/stdlib.h>

namespace rom_ram
{
    std::string dump_memory(uint16_t addr, uint16_t length);
    bool cmd_assert_address_bus(CommandInput);
    std::vector<uint8_t> read_memory(uint32_t address, uint32_t length);
    void write_to_memory(uint8_t * data, uint32_t length, uint16_t target_address);
}
