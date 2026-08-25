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
    bus_active = gpio_get(PIN_BUS_ENABLE);
    if (bus_active)
    {
        gpio_put(PIN_BUS_ENABLE, BE_INACTIVE);
    }
    gpio_set_dir_masked64(mask, mask);
    gpio_put(PIN_RW, RW_READ);
}

PinScopeAddressWrite::~PinScopeAddressWrite()
{
    gpio_set_dir_masked64(mask, 0);
    if (bus_active)
    {
        gpio_put(PIN_BUS_ENABLE, BE_ACTIVE);
    }
}

PinScopeAddressRead::PinScopeAddressRead()
    : mask(cmd_io::ADDR_MASK | cmd_io::RW_MASK)
{
    bus_active = gpio_get(PIN_BUS_ENABLE);
    if (bus_active)
    {
        gpio_put(PIN_BUS_ENABLE, BE_INACTIVE);
    }
    gpio_set_dir_masked64(mask, mask);
    gpio_put(PIN_RW, RW_READ);
}

PinScopeAddressRead::~PinScopeAddressRead()
{
    gpio_set_dir_masked64(mask, 0);
    if (bus_active)
    {
        gpio_put(PIN_BUS_ENABLE, BE_ACTIVE);
    }
//    gpio_put(PIN_BUS_ENABLE, BE_ACTIVE);
}

PinScopeReadWrite::PinScopeReadWrite()
    : mask(cmd_io::RW_MASK)
{
    gpio_set_dir_masked64(mask, mask);
    gpio_put(PIN_RW, RW_READ);
}

PinScopeReadWrite::~PinScopeReadWrite()
{
    gpio_put(PIN_RW, RW_READ);
    gpio_set_dir_masked64(mask, 0);
}
