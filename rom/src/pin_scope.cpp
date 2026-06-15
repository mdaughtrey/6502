#include <iostream>
#include "pin_scope.h"
#include "pin_defs.h"
#include "cmd_io.h"

PinScopeReady::PinScopeReady()
{
    gpio_put(PIN_READY, 0);
}


PinScopeReady:: ~PinScopeReady()
{
    gpio_put(PIN_READY, 1);
}

PinScopeBusEnable::PinScopeBusEnable()
{
    gpio_put(PIN_BUS_ENABLE, BE_INACTIVE);
}

PinScopeBusEnable::~PinScopeBusEnable()
{
    gpio_put(PIN_BUS_ENABLE, BE_ACTIVE);
}


PinScopeAddressWrite::PinScopeAddressWrite()
    : mask(cmd_io::ADDR_MASK | cmd_io::DATA_MASK | cmd_io::RW_MASK)
{
    gpio_set_dir_masked64(mask, mask);
}

PinScopeAddressWrite::~PinScopeAddressWrite()
{
    gpio_set_dir_masked64(mask, 0);
}

PinScopeAddressRead::PinScopeAddressRead()
    : mask(cmd_io::ADDR_MASK | cmd_io::RW_MASK)
{
    gpio_set_dir_masked64(mask, mask);
}

PinScopeAddressRead::~PinScopeAddressRead()
{
    gpio_set_dir_masked64(mask, 0);
}
