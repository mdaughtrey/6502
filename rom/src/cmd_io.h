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
    bool cmd_assert_databus(CommandInput);
    bool cmd_assert_address_bus(CommandInput);
    bool cmd_deassert_databus(CommandInput);
    bool cmd_io(CommandInput);
    bool cmd_init_buses(CommandInput);
//    bool cmd_memory_operation(CommandInput);
//    bool cmd_memory_operation_on_clock(CommandInput);
    bool cmd_clock_line_low(CommandInput);
    bool cmd_clock_line_high(CommandInput);
    bool cmd_we_lo(CommandInput);
    bool cmd_we_hi(CommandInput);
    bool cmd_bus_active(CommandInput); 
    bool cmd_bus_inactive(CommandInput); 
    bool cmd_set_memory_dump(CommandInput);
    bool cmd_dump_memory_on_clock(CommandInput);
}; // namespace cmd_io
