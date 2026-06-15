#ifndef PIN_SCOPE_H
#define PIN_SCOPE_H

#include "pico/stdlib.h"

class PinScopeReady
{
public:
    PinScopeReady();
    ~PinScopeReady();
};

class PinScopeBusEnable
{
public:
    PinScopeBusEnable();
    ~PinScopeBusEnable();
};

class PinScopeAddressWrite
{
    uint64_t mask;
public:
    PinScopeAddressWrite();
    ~PinScopeAddressWrite();
};

class PinScopeAddressRead
{
    uint64_t mask;
public:
    PinScopeAddressRead();
    ~PinScopeAddressRead();
};



#endif // PIN_SCOPE_H
