cmake_minimum_required(VERSION 3.13...3.27)
# set (PICO_PLATFORM rp2350)
include(pico_sdk_import.cmake)
project(rom)
pico_sdk_init()

set (SOURCES src/rom.cpp src/menu.cpp src/cmd_io.cpp src/rom_ram.cpp src/via6522.cpp)
# set (HEADER_FILES src/comm.h)
add_executable(rom ${SOURCES})
#pico_add_extra_outputs(rom)

target_link_libraries(rom pico_stdlib pico_time hardware_pwm hardware_clocks)
target_include_directories(rom PUBLIC src/)
pico_add_extra_outputs(rom)
pico_enable_stdio_usb(rom 1)
pico_enable_stdio_uart(rom 0)


