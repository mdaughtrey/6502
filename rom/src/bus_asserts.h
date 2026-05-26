#pragma once

#include <cstdint>

namespace bus_asserts
{
	void assert_address_bus(uint16_t addr);
	void assert_databus(uint8_t data);
} // namespace bus_asserts
