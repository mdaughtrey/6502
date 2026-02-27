cmake_minimum_required(VERSION 3.13...3.27)
# set (PICO_PLATFORM rp2350)
include(pico_sdk_import.cmake)
project(rom)
pico_sdk_init()

set (SOURCES src/rom.cpp src/menu.cpp src/cmd_io.cpp src/rom_ram.cpp src/via6522.cpp src/debugger.cpp src/bus_asserts.cpp)
# set (HEADER_FILES src/comm.h)
add_executable(rom ${SOURCES})
#pico_add_extra_outputs(rom)

#message(STATUS "BINARY ${CMAKE_CURRENT_BINARY_DIR}")
#message(STATUS "SOURCE ${CMAKE_SOURCE_DIR}")
#message(STATUS "OBJCOPY ${CMAKE_OBJCOPY}")

#add_custom_command(
#    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/asm_main.o
#    COMMAND ${CMAKE_OBJCOPY} -I binary -O elf32-littlearm -B arm ${CMAKE_SOURCE_DIR}/../programs/ioutils/main.asm ${CMAKE_CURRENT_BINARY_DIR}/asm_main.o
#    DEPENDS ${CMAKE_SOURCE_DIR}/../programs/ioutils/main.asm
#)
#target_sources(rom PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/asm_main.o)

target_link_libraries(rom pico_stdlib pico_time hardware_pwm hardware_clocks)
target_include_directories(rom PUBLIC src/)
pico_add_extra_outputs(rom)
pico_enable_stdio_usb(rom 1)
pico_enable_stdio_uart(rom 0)


