#include <string>
#include <vector>

#include "types.h"

namespace cmd_io
{
    void init(void);
    void loop(void);
    bool cmd_set_clock_frequency(CommandInput);
    bool cmd_step_clock(CommandInput);
    bool cmd_pin_status(CommandInput);
    bool cmd_pin_status_on_clock(CommandInput);
    bool cmd_reset(CommandInput);
//    bool cmd_assert_databus(CommandInput);
    bool cmd_assert_address_bus(CommandInput);
//    bool cmd_deassert_databus(CommandInput);
    bool cmd_io(CommandInput);
    bool cmd_init_buses(CommandInput);
//    bool cmd_memory_operation(CommandInput);
//    bool cmd_memory_operation_on_clock(CommandInput);
//    bool cmd_clock_line_low(CommandInput);
//    bool cmd_clock_line_high(CommandInput);
    bool cmd_clock_stop(CommandInput);
//    bool cmd_we_lo(CommandInput);
//    bool cmd_we_hi(CommandInput);
    bool cmd_bus_active(CommandInput); 
    bool cmd_bus_inactive(CommandInput); 
    bool cmd_set_memory_dump(CommandInput);
    bool cmd_dump_memory_on_clock(CommandInput);
    bool cmd_set_breakpoint(CommandInput);
    bool cmd_clear_breakpoint(CommandInput);
    bool cmd_list_breakpoints(CommandInput);
    bool cmd_clear_clocked_tasks(CommandInput);
    bool cmd_dump_memory(CommandInput);
    void cmd_verbose_logging(bool);
    void set_address_bus_out(bool);
    bool cmd_toggle_pin_10hz(CommandInput);
    bool cmd_test_io_pins(CommandInput);

    extern const uint64_t ADDR_MASK;
    extern const uint64_t DATA_MASK;
    extern const uint64_t ADDR_MASK;
    extern const uint32_t ADDR_MASK_HI;
    extern const uint32_t ADDR_MASK_LO;
    extern const uint64_t RESET_MASK;
    extern const uint32_t RESET_MASK_HI;
    extern const uint32_t RESET_MASK_LO;
    extern const uint64_t DATA_MASK;
    extern const uint32_t DATA_MASK_HI;
    extern const uint32_t DATA_MASK_LO;
    extern const uint64_t BE_MASK;
    extern const uint32_t BE_MASK_HI;
    extern const uint32_t BE_MASK_LO;
    extern const uint64_t RW_MASK;
    extern const uint32_t RW_MASK_HI;
    extern const uint32_t RW_MASK_LO;
    extern const uint64_t RESET_MASK;
    extern const uint32_t RESET_MASK_HI;
    extern const uint32_t RESET_MASK_LO;
    extern const uint64_t CLOCK_MASK;
    extern const uint32_t CLOCK_MASK_HI;
    extern const uint32_t CLOCK_MASK_LO;
}; // namespace cmd_io
