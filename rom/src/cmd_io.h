#include <string>

namespace cmd_io
{
    void init(void);
    bool cmd_set_clock_frequency(std::string);
    bool cmd_step_clock(std::string);
    bool cmd_pin_status(std::string);
    bool cmd_reset(std::string);
}; // namespace cmd_io
