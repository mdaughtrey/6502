#include "bus_asserts.h"
#include "pico/stdlib.h"
#include "cmd_io.h"

namespace
{
	// internal helper — not exposed in the header
	inline void set_databus_out(bool out)
	{
		if (out)
			gpio_set_dir_masked64(cmd_io::DATA_MASK, cmd_io::DATA_MASK);
		else
			gpio_set_dir_masked64(cmd_io::DATA_MASK, 0);
	}
}

namespace bus_asserts
{
	void assert_address_bus(uint16_t addr)
	{
		cmd_io::set_address_bus_out(true);
		gpio_put_masked64(cmd_io::ADDR_MASK, static_cast<uint64_t>(addr));
	}

	void assert_databus(uint8_t data)
	{
		set_databus_out(true);
		for (auto ii = 0; ii < 8; ii++)
		{
			gpio_put(ii + 40, (data & (1 << ii)) ? 1 : 0);
		}
	}

} // namespace bus_asserts
