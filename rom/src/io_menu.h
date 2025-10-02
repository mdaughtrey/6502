/*
 * =====================================================================================
 *
 *       Filename:  io_menu.h
 *
 *    Description: j 
 *
 *        Version:  1.0
 *        Created:  09/21/2025 11:10:17 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#include "top_menu.h"
#ifndef LOCAL_DEV
#include "hardware/pwm.h"
#endif // LOCAL_DEV


void io_cmd_help();
void io_cmd_set_clock();

Command command_io[] =
{
    { 'c', "set clock freq", io_cmd_set_clock },
    { 'h', "help", io_cmd_help },
};
