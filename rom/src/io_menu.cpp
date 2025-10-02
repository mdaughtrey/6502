/*
 * =====================================================================================
 *
 *       Filename:  io_menu.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  09/22/2025 11:02:32 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include "io_menu.h"
#include "pico/stdlib.h"
#include "hardware/pwm.h"

const int PIN_CLOCK = 20;

struct
{
    uint32_t clock_frequency;
} config;

void io_cmd_help()
{   
    for (Command * iter = command_set; iter->key != 0x01; iter++)
    {
        printf("%c: %s\r\n", iter->key, iter->help);
    }
}

void io_cmd_set_clock()
{
    master_input_uint32(io_callback_set_clock);
}

void io_callback_set_clock(uint32_t value)
{
    uint8_t freq = atoi(value);
    if (freq)
    {
        config.clock_frequency = freq; 
         _io_set_pwm_frequency(freq);
    }
}
