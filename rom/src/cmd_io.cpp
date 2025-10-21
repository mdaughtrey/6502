#include <bitset>
#include <cstdio>
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
#include "pin_defs.h"

namespace cmd_io
{
    void set_databus_out(bool out);
    void set_address_bus_out(bool out);
    bool cmd_pin_status(CommandInput input);
    void pin_status(void);
//    void memory_operation(void);
    void run_clocked_tasks(void);
    void assert_address_bus(uint16_t addr);
    void assert_databus(uint8_t data);
//    std::deque<void (*)(void)> task_queue;
    std::list<std::pair<std::string, void (*)(void)> > clocked_tasks;
    repeating_timer_t lfo_timer;
    bool clock_pin_state = true;

    void init(void)
    {
         cmd_init_buses(CommandInput());
    }

    bool cmd_init_buses(CommandInput input = CommandInput())
    {
        gpio_init(PIN_BUS_ENABLE);
        gpio_set_dir(PIN_BUS_ENABLE, GPIO_OUT);
        gpio_put(PIN_BUS_ENABLE, BE_INACTIVE);
        for (auto ii = 0; ii < 64; ii++)
        {
            if ((1 << ii) & (ADDR_MASK | DATA_MASK | CLOCK_MASK | RESET_MASK | RW_MASK))
            {
                gpio_init(ii);
            }
        }
        set_address_bus_out(false);
        gpio_set_dir(PIN_CLOCK, GPIO_OUT);
        gpio_put(PIN_CLOCK, 1);
        gpio_put(PIN_BUS_ENABLE, BE_ACTIVE);
        return false;
    }

    void loop(void)
    {
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
        if (!clock_pin_state)
        {   
            run_clocked_tasks();
        }
        gpio_put(PIN_CLOCK, clock_pin_state);
        return true;
    }

    void run_clocked_tasks(void)
    {
//        printf("Run_clocked_tasks: %u tasks\r\n", clocked_tasks.size());
        for (auto iter = clocked_tasks.begin(); iter != clocked_tasks.end(); iter++)
        {
            printf("%s\r\n", iter->first.c_str());
            iter->second();
        }
    }

    void set_clock_frequency_low(uint32_t frequency_hz)
    {
        float period = 1.0/static_cast<float>(frequency_hz)*500.0;
        printf("set_clock_frequency_low called with %u, period is %f\r\n", frequency_hz, period);
        if (gpio_get_function(PIN_CLOCK) != GPIO_FUNC_SIO)
        {
            gpio_init(PIN_CLOCK);
            gpio_set_dir(PIN_CLOCK, GPIO_OUT);
        }
        clock_pin_state = false;
        add_repeating_timer_ms(static_cast<uint32_t>(period), &lfo_timer_callback, NULL, &lfo_timer);
        printf("set_clock_frequency_returns\r\n");
    }

    bool cmd_set_clock_frequency(CommandInput input = CommandInput())
    {
        if (input.empty())
        {
            printf("Enter a frequency: ");
            return true;
        }

        cancel_repeating_timer(&lfo_timer);
        
        uint32_t sys_clk = clock_get_hz(clk_usb);
        printf("sys_clk %u\r\n", sys_clk);
        uint32_t frequency_hz = std::stof(input[1]);
        printf("Setting clock frequency to %u Hz\n", frequency_hz);
        if (frequency_hz < 1000)
        {
            set_clock_frequency_low(frequency_hz);
            return false;
        }
        float divider = sys_clk / frequency_hz * 1.0;  // Max wrap is 65535
       // if (divider < 1.0) divider = 1.0;
        printf("divider %f\r\n", divider);
        uint32_t idivider = static_cast<int>(divider);
        printf("idivider %u\r\n", idivider);

        uint16_t wrap = static_cast<int>((divider - idivider) * 65536) -1 ;
        printf("wrap %u\r\n", wrap);
        // clock_gpio_init_int_frac16(PIN_CLOCK, CLOCKS_CLK_GPOUT0_CTRL_AUXSRC_VALUE_CLK_SYS, idivider, wrap);
        clock_gpio_init_int_frac16(PIN_CLOCK, CLOCKS_CLK_GPOUT0_CTRL_AUXSRC_VALUE_CLK_USB, idivider, wrap);
        
        return false;
    }

    bool set_clock_frequency0(std::string frequency)
    {
        printf("set_clock_frequency called with %s\r\n", frequency.c_str());
        if (frequency.empty())
        {
            printf("Enter a frequency: ");
            return true;
        }
        uint32_t frequency_hz = std::stoi(frequency);
        printf("Setting clock frequency to %u Hz\n", frequency_hz);

        if (gpio_get_function(PIN_CLOCK) != GPIO_FUNC_PWM)
        {
            gpio_set_function(PIN_CLOCK, GPIO_FUNC_PWM);
        }

        // Determine PWM slice and channel for the GPIO
        uint slice_num = pwm_gpio_to_slice_num(PIN_CLOCK);
        uint chan = pwm_gpio_to_channel(PIN_CLOCK);
        printf("slice %u channel %u\r\n", slice_num, chan);

        // Get system clock frequency
        uint32_t sys_clk = clock_get_hz(clk_usb);
        printf("sys_clk %u\r\n", sys_clk);

        // Calculate divider and wrap values
        float divider = sys_clk / (frequency_hz * 65536.0);  // Max wrap is 65535
        if (divider < 1.0) divider = 1.0;
        printf("divider %f\r\n", divider);
        uint32_t idivider = static_cast<int>(divider);
        printf("idivider %u\r\n", idivider);

        uint32_t wrap = static_cast<int>((divider - idivider) * 65536) - 1;
        printf("wrap %u\r\n", wrap);

        pwm_set_clkdiv(slice_num, divider);
        pwm_set_wrap(slice_num, wrap);
        pwm_set_chan_level(slice_num, chan, wrap/2);
        pwm_set_enabled(slice_num, true);
        return false;
    }

    bool cmd_step_clock(CommandInput input = CommandInput())
    {
        if (gpio_get_function(PIN_CLOCK) != GPIO_FUNC_SIO)
        {
              gpio_init(PIN_CLOCK);
              gpio_set_dir(PIN_CLOCK, GPIO_OUT);
        }
        set_databus_out(false);
        gpio_put(PIN_CLOCK, 0);
        sleep_us(1);
        uint64_t pins = gpioc_hilo_in_get();
        if (pins & RW_MASK) // Read
        {
            run_clocked_tasks();
        }
        gpio_put(PIN_CLOCK, 1);
        sleep_us(1);
        if (!(pins & RW_MASK)) // Write
        {
            run_clocked_tasks();
        }
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
        printf("     Data          Addr                   Data               b R R     C      AddrH    AddrL\r\n");
        printf(".... ..    .. .... .... ........ ........ ........ ........ .E.w.S.. ..K..... FEDCBA98 76543210\r\n");
        uint64_t pins = gpioc_hilo_in_get();
        uint64_t mask = (static_cast<uint64_t>(sio_hw->gpio_hi_oe) << 32) | static_cast<uint64_t>(sio_hw->gpio_oe);
        printf("%04x %02x    %02x %04x %04x %s %s %s %s %s %s %s %s\r\n",
            static_cast<uint16_t>(pins >> 48), static_cast<uint8_t>(pins >> 40), static_cast<uint8_t>(pins >> 32),
            static_cast<uint16_t>(pins >> 16), static_cast<uint16_t>(pins), 
            std::bitset<8>(pins >> 56).to_string().c_str(), std::bitset<8>(pins >> 48).to_string().c_str(), 
            std::bitset<8>(pins >> 40).to_string().c_str(), std::bitset<8>(pins >> 32).to_string().c_str(), 
            std::bitset<8>(pins >> 24).to_string().c_str(), std::bitset<8>(pins >> 16).to_string().c_str(), 
            std::bitset<8>(pins >> 8).to_string().c_str(), std::bitset<8>(pins).to_string().c_str());
        printf("%04x %02x    %02x %04x %04x %s %s %s %s %s %s %s %s\r\n",
            static_cast<uint16_t>(mask >> 48), static_cast<uint8_t>(mask >> 40), static_cast<uint8_t>(mask >> 32),
            static_cast<uint16_t>(mask >> 16), static_cast<uint16_t>(mask), 
            std::bitset<8>(mask >> 56).to_string().c_str(), std::bitset<8>(mask >> 48).to_string().c_str(), 
            std::bitset<8>(mask >> 40).to_string().c_str(), std::bitset<8>(mask >> 32).to_string().c_str(), 
            std::bitset<8>(mask >> 24).to_string().c_str(), std::bitset<8>(mask >> 16).to_string().c_str(), 
            std::bitset<8>(mask >> 8).to_string().c_str(), std::bitset<8>(mask).to_string().c_str());
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

    bool cmd_disable_memory(CommandInput = CommandInput())
    {
//        clocked_tasks.erase("Memory");
        return false;
    }

    bool cmd_reset(CommandInput input = CommandInput())
    {
        cancel_repeating_timer(&lfo_timer);
        if (gpio_get_function(PIN_RESET) != GPIO_FUNC_SIO)
        {
              gpio_init(PIN_RESET);
              gpio_set_dir(PIN_RESET, GPIO_OUT);
        }
        gpio_put(PIN_RESET, 0);
        clocked_tasks.clear();
        add_alarm_in_ms(10, [](alarm_id_t id, void *user_data) -> int64_t { gpio_put(PIN_CLOCK, 0); return 0; }, NULL, true);
        add_alarm_in_ms(20, [](alarm_id_t id, void *user_data) -> int64_t { gpio_put(PIN_CLOCK, 1); return 0; }, NULL, true);
        add_alarm_in_ms(30, [](alarm_id_t id, void *user_data) -> int64_t { gpio_put(PIN_CLOCK, 0); return 0; }, NULL, true);
        add_alarm_in_ms(40, [](alarm_id_t id, void *user_data) -> int64_t { gpio_put(PIN_CLOCK, 1); return 0; }, NULL, true);
        add_alarm_in_ms(50, [](alarm_id_t id, void *user_data) -> int64_t { gpio_put(PIN_RESET, 1); return 0; }, NULL, true);
        return false;
    }

    bool cmd_assert_databus(CommandInput input = CommandInput())
    {
        if (input.empty())
        {
            printf("Enter hex value: ");
            return true;
        }
        std::cout << std::endl;
        uint8_t data = std::stoi(input[0], nullptr, 16);
        printf("Asserting data bus with %02x\r\n", data);
        assert_databus(data);
        return false;
    }

    void assert_databus(uint8_t data)
    {
        for (auto ii = 40; ii < 48; ii++)
        {
            gpio_init(ii);
        }
        set_databus_out(true);
        std::cout << "Data " << std::bitset<64>(static_cast<uint64_t>(data) << 40) << std::endl << "Mask " << std::bitset<64>(DATA_MASK) << std::endl;
        for (auto ii = 0; ii < 8; ii++)
        {
            if (data & (1 << ii))
            {
                gpio_put(ii + 40, 1);
            }
            else
            {
                gpio_put(ii + 40, 0);
            }
                    
        }
//        gpio_put_masked64(DATA_MASK, static_cast<uint64_t>(data) << 40);
    }

    bool cmd_deassert_databus(CommandInput input = CommandInput())
    {
        set_databus_out(false);
        return false;
    }

    inline void set_databus_out(bool out)
    {
//        std::cout << "set_databus_out " << out << " BEFORE " << std::bitset<32>(sio_hw->gpio_hi_oe) << std::bitset<32>(sio_hw->gpio_oe) << std::endl;
//        for (auto ii = 40; ii < 48; ii++)
//        {
//            gpio_set_dir(ii, GPIO_OUT);
//        }
        if (out)
            gpio_set_dir_masked64(DATA_MASK, DATA_MASK);
        else
          gpio_set_dir_masked64(DATA_MASK, 0);
//        std::cout << "set_databus_out " << out << " AFTER " << std::bitset<32>(sio_hw->gpio_hi_oe) << std::bitset<32>(sio_hw->gpio_oe) << std::endl;
    }

    void set_address_bus_out(bool out)
    {
//        printf("set_address_bus_out %u\r\n", out);
//        std::cout << "set_address_bus_out " << out << " BEFORE " << std::bitset<32>(sio_hw->gpio_hi_oe) << std::bitset<32>(sio_hw->gpio_oe) << std::endl;
        if (out)
        {
            gpio_put(PIN_BUS_ENABLE, BE_INACTIVE);
            gpio_set_dir(PIN_RW, GPIO_OUT);
            gpio_put(PIN_RW, RW_READ);
            gpio_set_dir_masked64(ADDR_MASK, ADDR_MASK);
        }
        else
        {
            gpio_set_dir_masked64(ADDR_MASK, 0);
            gpio_set_dir(PIN_RW, GPIO_IN);
            gpio_put(PIN_BUS_ENABLE, BE_ACTIVE);
        }
//        std::cout << "set_address_bus_out " << out << " AFTER " << std::bitset<32>(sio_hw->gpio_hi_oe) << std::bitset<32>(sio_hw->gpio_oe) << std::endl;
    }

    bool cmd_io(CommandInput input = CommandInput())
    {
        if (input.empty())
        {
            printf("[io]Pin[01] (i|o)NN(1|0)");
            return true;
        }
//        std::string io(value.substr(0,1));
//        std::string pin(value.substr(1,2));
//        std::string state(value.substr(3,1));
        uint8_t pin_num = std::stoi(input[2]);
        printf("io: %s, pin: %u\r\n", input[1].c_str(), pin_num);
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

    bool cmd_clock_line_low(CommandInput input = CommandInput())
    {
        gpio_put(PIN_CLOCK, 0);
        return false;
    }
    bool cmd_clock_line_high(CommandInput input = CommandInput())
    {
        gpio_put(PIN_CLOCK, 1);
        return false;
    }

    bool cmd_assert_address_bus(CommandInput input = CommandInput())
    {
        if (input.empty())
        {
            printf("Enter addr XXXX: ");
            return true;
        }
        std::cout << std::endl;
        uint16_t address = std::stoi(input[0], nullptr, 16);
        assert_address_bus(address);
        return true;
    }

    void assert_address_bus(uint16_t addr)
    {
//        printf("Assert Address Bus %04x\r\n", addr);
        set_address_bus_out(true);
        gpio_put_masked64(ADDR_MASK, static_cast<uint64_t>(addr));
    }

    bool cmd_we_lo(CommandInput input = CommandInput())
    {
        gpio_set_dir(PIN_RW, GPIO_OUT);
        gpio_put(PIN_RW, 0);
        return false;
    }

    bool cmd_we_hi(CommandInput input = CommandInput())
    {
        gpio_set_dir(PIN_RW, GPIO_OUT);
        gpio_put(PIN_RW, 1);
        return false;
    }
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

} // namespace cmd_io
