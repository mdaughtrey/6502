#include "pico/stdlib.h"

#include "pin_defs.h"

namespace cmd_io
{
    #define DATA_TO_MASK(dd) (static_cast<uint64_t>(dd) << 40)
    #define MASK_TO_DATA(dd) (static_cast<uint64_t>(dd) >> 40)
    
    const uint64_t ADDR_MASK = 0x000000000000ffff;
    const uint32_t ADDR_MASK_HI = (ADDR_MASK >> 32u);
    const uint32_t ADDR_MASK_LO = (ADDR_MASK & 0xffffffff);
    const uint64_t RESET_MASK = static_cast<uint64_t>(1 << PIN_RESET);
    const uint32_t RESET_MASK_HI = (RESET_MASK >> 32u);
    const uint32_t RESET_MASK_LO = (RESET_MASK & 0xffffffff);
    const uint64_t DATA_MASK = 0x0000ff0000000000;
    const uint32_t DATA_MASK_HI = (DATA_MASK >> 32u);
    const uint32_t DATA_MASK_LO = (DATA_MASK & 0xffffffff);
    const uint64_t RW_MASK = static_cast<uint64_t>(1 << PIN_RW);
    const uint32_t RW_MASK_HI = (RW_MASK >> 32u);
    const uint32_t RW_MASK_LO = (RW_MASK & 0xffffffff);
    const uint64_t CLOCK_MASK = static_cast<uint64_t>(1 << PIN_CLOCK);
    const uint32_t CLOCK_MASK_HI = (CLOCK_MASK >> 32);
    const uint32_t CLOCK_MASK_LO = (CLOCK_MASK & 0xffffffff);

    void assert_address_bus(uint16_t addr);
    void assert_databus(uint8_t data);
    void pin_status(void);
}; // namespace cmd_io
