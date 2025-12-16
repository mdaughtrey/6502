#include "pico/stdlib.h"

#include "pin_defs.h"

namespace cmd_io
{
    #define DATA_TO_MASK(dd) (static_cast<uint64_t>(dd) << 40)
    #define MASK_TO_DATA(dd) (static_cast<uint64_t>(dd) >> 40)

//    const uint64_t ADDR_MASK = 0x000000000000ffff;
    const uint64_t ADDR_MASK = static_cast<uint64_t>(0xffff) << PIN_ADDR0;
    const uint32_t ADDR_MASK_HI = (ADDR_MASK >> 32u);
    const uint32_t ADDR_MASK_LO = (ADDR_MASK & 0xffffffff);

    const uint64_t DATA_MASK = static_cast<uint64_t>(0xff) << PIN_DATA0;
//    const uint64_t DATA_MASK = 0x0000ff0000000000;
    const uint32_t DATA_MASK_HI = (DATA_MASK >> 32u);
    const uint32_t DATA_MASK_LO = (DATA_MASK & 0xffffffff);

    const uint64_t RESET_MASK = static_cast<uint64_t>(1) << PIN_RESET;
    const uint32_t RESET_MASK_HI = (RESET_MASK >> 32u);
    const uint32_t RESET_MASK_LO = (RESET_MASK & 0xffffffff);

    const uint64_t CLOCK_MASK = static_cast<uint64_t>(1) << PIN_CLOCK;
    const uint32_t CLOCK_MASK_HI = (CLOCK_MASK >> 32);
    const uint32_t CLOCK_MASK_LO = (CLOCK_MASK & 0xffffffff);

    const uint64_t PHI0_MASK = static_cast<uint64_t>(1) << PIN_PHI0;
    const uint32_t PHI0_MASK_HI = (PHI0_MASK >> 32);
    const uint32_t PHI0_MASK_LO = (PHI0_MASK & 0xffffffff);

    const uint64_t IRQ_MASK = static_cast<uint64_t>(1) << PIN_IRQ;
    const uint32_t IRQ_MASK_HI = (IRQ_MASK >> 32);
    const uint32_t IRQ_MASK_LO = (IRQ_MASK & 0xffffffff);

    const uint64_t NMI_MASK = static_cast<uint64_t>(1) << PIN_NMI;
    const uint32_t NMI_MASK_HI = (NMI_MASK >> 32);
    const uint32_t NMI_MASK_LO = (NMI_MASK & 0xffffffff);

    const uint64_t RW_MASK = static_cast<uint64_t>(1) << PIN_RW;
    const uint32_t RW_MASK_HI = (RW_MASK >> 32u);
    const uint32_t RW_MASK_LO = (RW_MASK & 0xffffffff);

    const uint64_t READY_MASK = static_cast<uint64_t>(1) << PIN_READY;
    const uint32_t READY_MASK_HI = (READY_MASK >> 32u);
    const uint32_t READY_MASK_LO = (READY_MASK & 0xffffffff);

    const uint64_t SYNC_MASK = static_cast<uint64_t>(1) << PIN_SYNC;
    const uint32_t SYNC_MASK_HI = (SYNC_MASK >> 32u);
    const uint32_t SYNC_MASK_LO = (SYNC_MASK & 0xffffffff);

    const uint64_t BE_MASK = static_cast<uint64_t>(1) << PIN_BUS_ENABLE;
    const uint32_t BE_MASK_HI = (BE_MASK >> 32u);
    const uint32_t BE_MASK_LO = (BE_MASK & 0xffffffff);

    void assert_address_bus(uint16_t addr);
    void assert_databus(uint8_t data);
    void pin_status(void);
}; // namespace cmd_io
