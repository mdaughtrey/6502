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
    bool bus_active;
    PinScopeBusEnable();
    ~PinScopeBusEnable();
};

class PinScopeAddressWrite : public PinScopeBusEnable
{
    uint64_t mask;
public:
    PinScopeAddressWrite();
    ~PinScopeAddressWrite();
};

class PinScopeAddressRead : public PinScopeBusEnable
{
    uint64_t mask;
public:
    PinScopeAddressRead();
    ~PinScopeAddressRead();
};

class PinScopeReadWrite
{
    uint64_t mask;
public:
    PinScopeReadWrite();
    ~PinScopeReadWrite();
};



#endif // PIN_SCOPE_H
