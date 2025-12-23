#pragma once
#include "pico/stdlib.h"

const uint64_t PIN_BUS_ENABLE = 31;
const uint64_t PIN_CLOCK = 25;
const uint64_t PIN_RESET = 33;
const uint64_t PIN_PHI0 = 34;
const uint64_t PIN_IRQ = 35;
const uint64_t PIN_NMI = 36;
const uint64_t PIN_RW = 37;
const uint64_t PIN_READY = 38;
const uint64_t PIN_SYNC = 39;
const uint64_t PIN_DATA0 = 40;
const uint64_t PIN_ADDR0 = 0;

const uint8_t RW_READ = 1;
const uint8_t RW_WRITE = 0;
const uint8_t BE_ACTIVE = 1;
const uint8_t BE_INACTIVE = 0;

// 47 - 40 D7 - D0
// 0 - 15 A0 - A15
