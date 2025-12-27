#include <bitset>
#include <cstdio>
#include <functional>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <list>
#include <utility>
#include <deque>

#include "hardware/structs/sio.h"
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/pwm.h"
#include "types.h"
#include "cmd_io.h"
#include "cmd_io_internal.h"

#include "rom_ram.h"
#include "rom_ram_internal.h"
#include "pin_defs.h"


#define VERBOSE(...) \
if (verbose) \
{ \
    char buffer[256]; \
    sprintf(buffer, __VA_ARGS__); \
    log_queue.push_back(buffer); \
}

namespace cmd_io
{
    void set_databus_out(bool out);
    void set_address_bus_out(bool out);
    bool cmd_pin_status(CommandInput input);
    bool cmd_init_buses(CommandInput);
    void pin_status(void);
    void breakpoint_check(void);
    void set_clock_frequency(float frequency_hz);
//    void memory_operation(void);
    void run_clocked_tasks(bool clock_state);
//    void assert_address_bus(uint16_t addr);
    void assert_databus(uint8_t data);
//    std::deque<void (*)(void)> task_queue;
    std::list<std::pair<std::string, void (*)(void)> > clocked_tasks;
    std::list<std::string> log_queue;
    std::list<uint16_t> breakpoints;
    repeating_timer_t lfo_timer;
    bool clock_pin_state = true;
    bool verbose = false;
    uint16_t memdump_addr = 0;
    uint16_t memdump_length = 0;

    void init(void)
    {
        gpio_init(PIN_RESET);
        gpio_set_dir(PIN_RESET, GPIO_OUT);
        gpio_put(PIN_RESET, 1);
        cmd_init_buses(CommandInput());
    }

    bool cmd_init_buses(CommandInput input = CommandInput())
    {
        uint64_t mask = RESET_MASK | CLOCK_MASK | NMI_MASK | PHI0_MASK | BE_MASK | READY_MASK;
        VERBOSE("Pin initialization mask is %s", std::bitset<64>(mask).to_string().c_str());
        for (auto ii = 0; ii < 64; ii++)
        {
            if (mask & (1ull<<ii))
            {
                gpio_init(ii);
                VERBOSE("Setting %d to OUT", ii);
                gpio_set_dir(ii, GPIO_OUT);
                gpio_put(ii, 1);
            }
        }
//        gpio_init(PIN_BUS_ENABLE);
//        gpio_set_dir(PIN_BUS_ENABLE, GPIO_OUT);
        gpio_put(PIN_BUS_ENABLE, BE_INACTIVE);
        for (auto ii = 0; ii < 64; ii++)
        {
            if ((1ull << ii) & (ADDR_MASK | DATA_MASK | RW_MASK))
            {
                gpio_init(ii);
                VERBOSE("Setting %d to IN", ii);
            }
        }
        set_address_bus_out(false);
        gpio_put(PIN_BUS_ENABLE, BE_ACTIVE);
        return false;
    }

    void loop(void)
    {
        for (auto iter = log_queue.begin(); iter != log_queue.end(); iter++)
        {
            std::cout << *iter << std::endl;
        }
        log_queue.clear();
//        if (!gpio_get(PIN_CLOCK))
//        {
//            continue;
//        }
//        if (!task_queue.empty())
//        {
//            task_queue.front()();
//            task_queue.pop_front();
//        }
    }

    static bool lfo_timer_callback(repeating_timer_t *t)
    {
        clock_pin_state = !clock_pin_state;
        gpio_put(PIN_CLOCK, clock_pin_state);
        run_clocked_tasks(clock_pin_state);
        return true;
    }

    void run_clocked_tasks(bool clock_state)
    {
//        VERBOSE("Run_clocked_tasks: %u tasks", clocked_tasks.size())
        if (!clocked_tasks.empty())
        {
            log_queue.push_back(clock_state ? "Clock 1" : "Clock 0");
        }
        for (auto iter = clocked_tasks.begin(); iter != clocked_tasks.end(); iter++)
        {
            VERBOSE("Clocked task %s", iter->first.c_str())
            iter->second();
        }
    }

    void set_clock_frequency_low(float frequency_hz)
    {
        VERBOSE("set_clock_frequency_low\r\n");
        VERBOSE("frequency %f\r\n", frequency_hz);

        uint64_t period = static_cast<uint64_t>(1.0/frequency_hz*5e5); // *500.0;
        VERBOSE("set_clock_frequency_low called with %f, period is %llu us\r\n", frequency_hz, period)
        if (gpio_get_function(PIN_CLOCK) != GPIO_FUNC_SIO)
        {
            gpio_init(PIN_CLOCK);
            gpio_set_dir(PIN_CLOCK, GPIO_OUT);
        }
        clock_pin_state = false;
        bool result = add_repeating_timer_us(period, &lfo_timer_callback, NULL, &lfo_timer);
        VERBOSE("add_repeating_timer_ms returns %d, set_clock_frequency_returns", result);
    }

    bool cmd_set_clock_frequency(CommandInput input = CommandInput())
    {
        if (input.empty())
        {
            return true;
        }
        float frequency_hz = std::stof(input[1]);
        VERBOSE("Requesting clock frequency %f\r\n", frequency_hz);
        set_clock_frequency(frequency_hz);
        return false;
    }

    void set_clock_frequency(float frequency_hz)
    {
        cancel_repeating_timer(&lfo_timer);
        
        uint32_t sys_clk = clock_get_hz(clk_usb);
        VERBOSE("Clock frequency %f Hz", frequency_hz)
 //       printf("sys_clk %u\r\n", sys_clk);
        if (frequency_hz == 0.0)
        {
            gpio_init(PIN_CLOCK);
            return;
        }
//        gpio_set_dir(PIN_CLOCK, GPIO_OUT);
        VERBOSE("Requested clock frequency %f\r\n", frequency_hz);
        if (frequency_hz < 1000.0)
        {
            VERBOSE("Requesting set_clock_frequency_low\r\n");
            set_clock_frequency_low(frequency_hz);
            return;
        }
        float divider = sys_clk / frequency_hz * 1.0;  // Max wrap is 65535
       // if (divider < 1.0) divider = 1.0;
        VERBOSE("divider %f\r\n", divider);
        uint32_t idivider = static_cast<int>(divider);
        VERBOSE("idivider %u\r\n", idivider);

        uint16_t wrap = static_cast<int>((divider - idivider) * 65536) -1 ;
        VERBOSE("wrap %u\r\n", wrap);
        // clock_gpio_init_int_frac16(PIN_CLOCK, CLOCKS_CLK_GPOUT0_CTRL_AUXSRC_VALUE_CLK_SYS, idivider, wrap);
        //
        clock_gpio_init_int_frac16(PIN_CLOCK, CLOCKS_CLK_GPOUT3_CTRL_AUXSRC_VALUE_CLK_USB, idivider, wrap);
    }

    bool cmd_step_clock(CommandInput input = CommandInput())
    {
//        if (gpio_get_function(PIN_CLOCK) != GPIO_FUNC_SIO)
//        {
//              gpio_init(PIN_CLOCK);
//              gpio_set_dir(PIN_CLOCK, GPIO_OUT);
//        }

        gpio_init(PIN_CLOCK);
        gpio_set_dir(PIN_CLOCK, GPIO_OUT);
        gpio_put(PIN_CLOCK, 0);
        sleep_ms(1);
        if (!clocked_tasks.empty())
        {
            run_clocked_tasks(0);
        }
        gpio_put(PIN_CLOCK, 1);
        sleep_ms(1);
        if (!clocked_tasks.empty())
        {
            run_clocked_tasks(1);
        }
//        if (!(pins & RW_MASK)) // Write
//        {
//            run_clocked_tasks();
//        }
        return false;

    }
    bool cmd_pin_status(CommandInput input = CommandInput())
    {
        pin_status();
        return false;
    }

//    bool cmd_memory_operation(CommandInput input)
//    {
//        memory_operation();
//        return false;
//    }
//
//    void memory_operation(void)
//    {
//        set_databus_out(false);
//        uint64_t pins = gpioc_hilo_in_get();
//        uint16_t addr = pins & ADDR_MASK;
//        if (pins & RW_MASK)
//        {
//            set_databus_out(true);
//            uint8_t data = rom_ram::MEMORY[addr];
//            printf("Reading %02x from address %04x\r\n", data, addr); 
//            gpio_put_masked64(DATA_MASK, DATA_TO_MASK(data));
////            sio_hw->gpio_togl = (sio_hw->gpio_out ^ static_cast<uint32_t>(data) & DATA_MASK_LO);
////            sio_hw->gpio_hi_togl = (sio_hw->gpio_hi_out ^ static_cast<uint32_t>(data<<8) & DATA_MASK_HI);
//        }
//        else
//        {
//            set_databus_out(false);
//            uint8_t data = MASK_TO_DATA(pins);
//            printf("Writing %02x to address %04x\r\n", data, addr);
//            rom_ram::MEMORY[addr] = data;
//        }
//    }

    void pin_status(void)
    {
        char buffer[128];
        sprintf(buffer, "     Data          Addr                   Data     SRRni rC B                 AddrH    AddrL");
        log_queue.push_back(buffer);
        sprintf(buffer, ".... ..    .. .... .... ........ ........ ........ YYWIQ.SK e....... ........ FEDCBA98 76543210");
        log_queue.push_back(buffer);
        uint64_t pins = gpioc_hilo_in_get();
        uint64_t mask = (static_cast<uint64_t>(sio_hw->gpio_hi_oe) << 32) | static_cast<uint64_t>(sio_hw->gpio_oe);
        sprintf(buffer, "%04x %02x    %02x %04x %04x %s %s %s %s %s %s %s %s",
            static_cast<uint16_t>(pins >> 48), static_cast<uint8_t>(pins >> 40), static_cast<uint8_t>(pins >> 32),
            static_cast<uint16_t>(pins >> 16), static_cast<uint16_t>(pins), 
            std::bitset<8>(pins >> 56).to_string().c_str(), std::bitset<8>(pins >> 48).to_string().c_str(), 
            std::bitset<8>(pins >> 40).to_string().c_str(), std::bitset<8>(pins >> 32).to_string().c_str(), 
            std::bitset<8>(pins >> 24).to_string().c_str(), std::bitset<8>(pins >> 16).to_string().c_str(), 
            std::bitset<8>(pins >> 8).to_string().c_str(), std::bitset<8>(pins).to_string().c_str());
        log_queue.push_back(buffer);
        sprintf(buffer, "%04x %02x    %02x %04x %04x %s %s %s %s %s %s %s %s",
            static_cast<uint16_t>(mask >> 48), static_cast<uint8_t>(mask >> 40), static_cast<uint8_t>(mask >> 32),
            static_cast<uint16_t>(mask >> 16), static_cast<uint16_t>(mask), 
            std::bitset<8>(mask >> 56).to_string().c_str(), std::bitset<8>(mask >> 48).to_string().c_str(), 
            std::bitset<8>(mask >> 40).to_string().c_str(), std::bitset<8>(mask >> 32).to_string().c_str(), 
            std::bitset<8>(mask >> 24).to_string().c_str(), std::bitset<8>(mask >> 16).to_string().c_str(), 
            std::bitset<8>(mask >> 8).to_string().c_str(), std::bitset<8>(mask).to_string().c_str());
        log_queue.push_back(buffer);
    }

    bool cmd_pin_status_on_clock(CommandInput input = CommandInput())
    {
        clocked_tasks.push_back(std::pair("Pins", pin_status));
        return false;
    }

//    bool cmd_memory_operation_on_clock(std::string)
//    {
//        clocked_tasks.push_back(std::pair("Memory", memory_operation));
//        return false;
//    }

//    bool cmd_disable_memory(CommandInput = CommandInput())
//    {
//        clocked_tasks.erase("Memory");
//        return false;
//    }

    bool cmd_reset(CommandInput input = CommandInput())
    {
//        gpio_init(PIN_RESET);
//        gpio_set_dir(PIN_RESET, GPIO_OUT);
        gpio_put(PIN_RESET, 0);
        sleep_ms(100);
        gpio_put(PIN_RESET, 1);
        sleep_ms(100);
//        set_clock_frequency(0.0);
//        gpio_init(PIN_CLOCK);
//        gpio_set_dir(PIN_CLOCK, GPIO_OUT);
//        gpio_put(PIN_CLOCK, 1);
        cmd_init_buses(CommandInput());
//        gpio_put(PIN_BUS_ENABLE, BE_ACTIVE);
//
//        gpio_put(PIN_RESET, 0);
//        for (auto ii = 0; ii < 2; ii++)
//        {
//            gpio_put(PIN_CLOCK, 0);
//            sleep_ms(10);
//            gpio_put(PIN_CLOCK, 1);
//            sleep_ms(10);
//        }
//        gpio_put(PIN_RESET, 1);
//        for (auto ii = 0; ii < 8; ii++)
//        {
//            gpio_put(PIN_CLOCK, 0);
//            sleep_ms(10);
//            gpio_put(PIN_CLOCK, 1);
//            sleep_ms(10);
//        }
//        for (auto ii = 0; ii < 2; ii++)
//        {
//            gpio_put(PIN_CLOCK, 0);
//            sleep_ms(10);
//            gpio_put(PIN_CLOCK, 1);
//            sleep_ms(10);
////            add_alarm_in_ms(8+ii, [](alarm_id_t id, void *user_data) -> int64_t { gpio_put(PIN_CLOCK, 0); return 0; }, NULL, true);
////            add_alarm_in_ms(16+ii, [](alarm_id_t id, void *user_data) -> int64_t { gpio_put(PIN_CLOCK, 1); return 0; }, NULL, true);
//        }
//        add_alarm_in_ms(500, [](alarm_id_t id, void *user_data) -> int64_t { gpio_put(PIN_RESET, 1); return 0; }, NULL, true);
        return false;
    }

//    bool cmd_assert_databus(CommandInput input = CommandInput())
//    {
//        if (input.empty())
//        {
//            return true;
//        }
//        uint8_t data = std::stoi(input[0], nullptr, 16);
//        VERBOSE("Asserting data bus with %02x\r\n", data)
//        assert_databus(data);
//        return false;
//    }


    void set_address_bus_out(bool out)
    {
//        printf("set_address_bus_out %u\r\n", out);
//        std::cout << "set_address_bus_out " << out << " BEFORE " << std::bitset<32>(sio_hw->gpio_hi_oe) << std::bitset<32>(sio_hw->gpio_oe) << std::endl;

        if (out)
        {
            gpio_set_dir(PIN_RW, GPIO_OUT);
            gpio_put(PIN_RW, RW_READ);
            gpio_set_dir_masked64(ADDR_MASK, ADDR_MASK);
        }
        else
        {
            gpio_set_dir_masked64(ADDR_MASK, 0);
            gpio_set_dir(PIN_RW, GPIO_IN);
        }
//        std::cout << "set_address_bus_out " << out << " AFTER " << std::bitset<32>(sio_hw->gpio_hi_oe) << std::bitset<32>(sio_hw->gpio_oe) << std::endl;
    }

    bool cmd_io(CommandInput input = CommandInput())
    {
        if (input.empty())
        {
            return true;
        }
//        std::string io(value.substr(0,1));
//        std::string pin(value.substr(1,2));
//        std::string state(value.substr(3,1));
        uint8_t pin_num = std::stoi(input[2]);
        VERBOSE("io: %s, pin: %u\r\n", input[1].c_str(), pin_num)
        gpio_init(pin_num);
        gpio_set_dir(pin_num, "o" == input[1] ? GPIO_OUT : GPIO_IN);
        if (input.size() >= 2)
        {
            gpio_put(pin_num, "1" == input[3] ? true : false);
        }
        else
        {
            printf("Pin %u is %u\r\n", pin_num, gpio_get(pin_num));
        }
        
        return false;
    }

    bool cmd_clock_stop(CommandInput input = CommandInput())
    {
        set_clock_frequency(0.0);
        return false;
    }

//    bool cmd_clock_line_low(CommandInput input = CommandInput())
//    {
//        gpio_put(PIN_CLOCK, 0);
//        return false;
//    }
//    bool cmd_clock_line_high(CommandInput input = CommandInput())
//    {
//        gpio_put(PIN_CLOCK, 1);
//        return false;
//    }

    bool cmd_assert_address_bus(CommandInput input = CommandInput())
    {
        if (input.empty())
        {
            return true;
        }
        uint16_t address = std::stoi(input[0], nullptr, 16);
        assert_address_bus(address);
        return true;
    }

    void assert_address_bus(uint16_t addr)
    {
        if (verbose)
        {
            char buffer[64];
            sprintf(buffer, "Assert Address Bus %04x\r\n", addr);
            log_queue.push_back(buffer);
        }
        set_address_bus_out(true);
        gpio_put_masked64(ADDR_MASK, static_cast<uint64_t>(addr));
    }

//    bool cmd_we_lo(CommandInput input = CommandInput())
//    {
//        gpio_set_dir(PIN_RW, GPIO_OUT);
//        gpio_put(PIN_RW, 0);
//        return false;
//    }
//
//    bool cmd_we_hi(CommandInput input = CommandInput())
//    {
//        gpio_set_dir(PIN_RW, GPIO_OUT);
//        gpio_put(PIN_RW, 1);
//        return false;
//    }
    bool cmd_bus_active(CommandInput input = CommandInput()) 
    {
        gpio_put(PIN_BUS_ENABLE, BE_ACTIVE);
        return false;
    }

    bool cmd_bus_inactive(CommandInput input = CommandInput())
    {
        gpio_put(PIN_BUS_ENABLE, BE_INACTIVE);
        return false;
    }

    bool cmd_dump_memory(CommandInput input = CommandInput())
    {
        if (input.empty())
        {
            return true;
        }
        memdump_addr = std::stoi(input[1], nullptr, 16);
        memdump_length = std::stoi(input[2], nullptr, 16);
        log_queue.push_back(rom_ram::dump_memory(memdump_addr, memdump_length));
        return false;
    }

    bool cmd_set_memory_dump(CommandInput input = CommandInput())
    {
        if (input.empty())
        {
            printf("No input\r\n");
            return true;
        }
        memdump_addr = std::stoi(input[1], nullptr, 16);
        memdump_length = std::stoi(input[2], nullptr, 16);
        printf("Set %04x/%04x\r\n", memdump_addr, memdump_length);
        return false;
    }

    bool cmd_dump_memory_on_clock(CommandInput input = CommandInput())
    {
        clocked_tasks.push_back(std::pair("Memory", [](){ VERBOSE("lambda"); log_queue.push_back(rom_ram::dump_memory(memdump_addr, memdump_length)); }));
        VERBOSE("Dumping %04x/%04x on clock", memdump_addr, memdump_length)
        return false;
    }

    void clocked_tasks_remove(std::string jobname)
    {
        for (auto iter = clocked_tasks.begin(); iter != clocked_tasks.end(); iter++)
        {
            if (iter->first == jobname)
            {
                clocked_tasks.erase(iter);
                break;
            }
        }
    }

    void breakpoint_check(void)
    {
//        char buffer[64];
        uint16_t addr = static_cast<uint16_t>(gpioc_hilo_in_get() & ADDR_MASK);
        for (auto iter = breakpoints.begin(); iter != breakpoints.end(); iter++)
        {
            VERBOSE("Checking addr %04x against %04x", addr, *iter);
//            log_queue.push_back(buffer);
            if (addr == *iter)
            {
                set_clock_frequency(0.0);
                VERBOSE("Break @%04x", addr)
                break;
            }
        }
    }

    bool cmd_set_breakpoint(CommandInput input = CommandInput())
    {
        if (input.empty())
        {
            return true;
        }
        breakpoints.push_back(std::stoi(input[1], nullptr, 16));
        bool found = false;
        for (auto iter = clocked_tasks.begin(); iter != clocked_tasks.end(); iter++)
        {
            if (iter->first == std::string("Breakpoint"))
            {
                found = true;
                break;
            }
        }
        if (false == found)
        {
            clocked_tasks.push_back(std::pair("Breakpoint", breakpoint_check));
        }
        return false;
    }

    bool cmd_clear_breakpoint(CommandInput input = CommandInput())
    {
        if (input.empty())
        {
            return true;
        }
        uint8_t index = std::stoi(input[1]);
        if (index < breakpoints.size())
        {
            auto iter = breakpoints.begin();
            std::advance(iter, index);
            breakpoints.erase(iter);
        }
        if (breakpoints.empty())
        {
            clocked_tasks_remove("Breakpoint");
        }
        return false;
    }

    bool cmd_list_breakpoints(CommandInput input = CommandInput())
    {
        uint8_t index = 0;
        for (auto iter = breakpoints.begin(); iter != breakpoints.end(); iter++)
        {
            printf("%02u: %04x\r\n", index++, *iter);
        }
        return false;
    }

    bool cmd_clear_clocked_tasks(CommandInput input = CommandInput())
    {
        clocked_tasks.clear();
        return false;
    }

    void cmd_verbose_logging(bool set)
    {
        verbose = set;
    }
} // namespace cmd_io
