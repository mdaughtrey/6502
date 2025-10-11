#include <string>

namespace cmd_io
{
    void init(void);
    void loop(void);
    bool cmd_set_clock_frequency(std::string);
    bool cmd_step_clock(std::string);
    bool cmd_pin_status(std::string);
    bool cmd_pin_status_on_clock(std::string);
    bool cmd_reset(std::string);
    bool cmd_assert_databus(std::string);
    bool cmd_assert_databus_via_pullups(std::string);
    bool cmd_io(std::string);
    bool cmd_init_buses(std::string);
    bool cmd_enable_memory(std::string);
    bool cmd_disable_memory(std::string);
}; // namespace cmd_io
