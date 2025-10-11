#include <bitset>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <deque>

#include "hardware/structs/sio.h"
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/pwm.h"
#include "cmd_io.h"
#include "rom_ram.h"
#include "pin_defs.h"

namespace cmd_io
{
    const uint64_t ADDR_MASK = 0x000000000000ffff;
    const uint32_t ADDR_MASK_HI = 0x00000000;
    const uint32_t ADDR_MASK_LO = 0x0000ffff;
    const uint64_t DATA_MASK = 0x0000ff0000000000;
    const uint32_t DATA_MASK_HI = 0x0000ff00;
    const uint32_t DATA_MASK_LO = 0x00000000;
    void set_databus_dir(bool out);
    bool cmd_pin_status(std::string value);
    void pin_status(void);
    void memory_operation(void);
    std::deque<void (*)(void)> task_queue;

    repeating_timer_t lfo_timer;
    bool pin_state = false;
    bool dump_pins_on_clock = false;
    bool run_memory_operation = false;

    void init(void)
    {
    }

    bool cmd_init_buses(std::string)
    {
        for (auto ii = 0; ii < 16; ii++)
        {
            gpio_init(ii);
//            gpio_set_dir(ii, GPIO_IN);
//            gpio_pull_up(ii);
        }
        for (auto ii = 40; ii < 48; ii++)
        {
            gpio_init(ii);
//            gpio_set_dir(ii, GPIO_IN);
//            gpio_pull_up(ii);
        }
        gpio_init(PIN_RW);
 //       gpio_set_dir(PIN_RW, GPIO_IN);
        ///gpio_pull_up(PIN_RW);
        return false;
    }

    void loop(void)
    {
        if (!task_queue.empty())
        {
            task_queue.front()();
            task_queue.pop_front();
        }
    }

    static bool lfo_timer_callback(repeating_timer_t *t)
    {
        pin_state = !pin_state;
        if (pin_state)
        {   
            if (run_memory_operation)
            {
                task_queue.push_back(memory_operation);
            }
            if (dump_pins_on_clock)
            {
                task_queue.push_back(pin_status);
            }
        }
        gpio_put(PIN_CLOCK, pin_state);
        return true;
    }

    void set_clock_frequency_low(uint32_t frequency_hz)
    {
        float period = 1.0/static_cast<float>(frequency_hz)*500.0;
        printf("set_clock_frequency_low called with %u, period is %f\r\n", frequency_hz, period);
        printf("Current pin function is %d\r\n", gpio_get_function(PIN_CLOCK));
        if (gpio_get_function(PIN_CLOCK) != GPIO_FUNC_SIO)
        {
            gpio_init(PIN_CLOCK);
            gpio_set_dir(PIN_CLOCK, GPIO_OUT);
        }
        pin_state = false;
        add_repeating_timer_ms(static_cast<uint32_t>(period), &lfo_timer_callback, NULL, &lfo_timer);
        printf("set_clock_frequency_returns\r\n");
    }

    bool cmd_set_clock_frequency(std::string frequency)
    {
        printf("set_clock_frequency called with %s\r\n", frequency.c_str());
        if (frequency.empty())
        {
            printf("Enter a frequency: ");
            return true;
        }

        cancel_repeating_timer(&lfo_timer);
        
        uint32_t sys_clk = clock_get_hz(clk_usb);
        printf("sys_clk %u\r\n", sys_clk);
        uint32_t frequency_hz = std::stof(frequency);
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

        printf("Current pin function is %d\r\n", gpio_get_function(PIN_CLOCK));
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

    bool cmd_step_clock(std::string value)
    {
        printf("Current pin function is %d\r\n", gpio_get_function(PIN_CLOCK));
        if (gpio_get_function(PIN_CLOCK) != GPIO_FUNC_SIO)
        {
              gpio_init(PIN_CLOCK);
              gpio_set_dir(PIN_CLOCK, GPIO_OUT);
        }
        gpio_put(PIN_CLOCK, 0);
        gpio_put(PIN_CLOCK, 1);
        return false;

    }
    void pin_status(void)
    {
        cmd_pin_status("");
    }

    void memory_operation(void)
    {
        uint64_t pins =  gpioc_hilo_in_get();
        uint16_t addr = pins & ADDR_MASK_LO;
        if (pins & PIN_RW) // READ
        {
            set_databus_dir(true);
            uint8_t data = addr >= rom_ram::RR_ROM_BASE ? rom_ram::ROM[addr - rom_ram::RR_ROM_BASE] : rom_ram::RAM[addr];
            sio_hw->gpio_togl = (sio_hw->gpio_out ^ static_cast<uint32_t>(data) & DATA_MASK_LO);
            sio_hw->gpio_hi_togl = (sio_hw->gpio_hi_out ^ static_cast<uint32_t>(data<<8) & DATA_MASK_HI);
        }
        else
        {
            set_databus_dir(false);
            if (addr < rom_ram::RR_ROM_BASE)
            {
                rom_ram::RAM[addr] = (pins & DATA_MASK) >> 40;
            }
        }
    }

    bool cmd_pin_status(std::string value)
    {
//        uint64_t pins = gpio_get_all64();

        printf("     Data          Addr                   Data                 R R     C      AddrH    AddrL\r\n");
        printf(".... ..    .. .... .... ........ ........ ........ ........ ...w.S.. ..K..... FEDCBA98 76543210\r\n");
//        printf("Via gpio_get_all64()\r\n");
//        printf("%04x %02x    %02x %04x %04x %s %s %s %s %s %s %s %s\r\n",
//            static_cast<uint16_t>(pins >> 48), static_cast<uint8_t>(pins >> 40), static_cast<uint8_t>(pins >> 32),
//            static_cast<uint16_t>(pins >> 16), static_cast<uint16_t>(pins), 
//            std::bitset<8>(pins >> 56).to_string().c_str(), std::bitset<8>(pins >> 48).to_string().c_str(), 
//            std::bitset<8>(pins >> 40).to_string().c_str(), std::bitset<8>(pins >> 32).to_string().c_str(), 
//            std::bitset<8>(pins >> 24).to_string().c_str(), std::bitset<8>(pins >> 16).to_string().c_str(), 
//            std::bitset<8>(pins >> 8).to_string().c_str(), std::bitset<8>(pins).to_string().c_str());
//
//        pins = 0;
//        for (auto ii = 0; ii < 64; ii++)
//        {
//            pins |= gpio_get(ii) << ii;
//        }
//        printf("Via gpio_get individual\r\n");
//        printf("%04x %02x    %02x %04x %04x %s %s %s %s %s %s %s %s\r\n",
//            static_cast<uint16_t>(pins >> 48), static_cast<uint8_t>(pins >> 40), static_cast<uint8_t>(pins >> 32),
//            static_cast<uint16_t>(pins >> 16), static_cast<uint16_t>(pins), 
//            std::bitset<8>(pins >> 56).to_string().c_str(), std::bitset<8>(pins >> 48).to_string().c_str(), 
//            std::bitset<8>(pins >> 40).to_string().c_str(), std::bitset<8>(pins >> 32).to_string().c_str(), 
//            std::bitset<8>(pins >> 24).to_string().c_str(), std::bitset<8>(pins >> 16).to_string().c_str(), 
//            std::bitset<8>(pins >> 8).to_string().c_str(), std::bitset<8>(pins).to_string().c_str());

        uint64_t pins =  gpioc_hilo_in_get();
//        printf("Via gpio_hilo_in_get()\r\n");
        printf("%04x %02x    %02x %04x %04x %s %s %s %s %s %s %s %s\r\n",
            static_cast<uint16_t>(pins >> 48), static_cast<uint8_t>(pins >> 40), static_cast<uint8_t>(pins >> 32),
            static_cast<uint16_t>(pins >> 16), static_cast<uint16_t>(pins), 
            std::bitset<8>(pins >> 56).to_string().c_str(), std::bitset<8>(pins >> 48).to_string().c_str(), 
            std::bitset<8>(pins >> 40).to_string().c_str(), std::bitset<8>(pins >> 32).to_string().c_str(), 
            std::bitset<8>(pins >> 24).to_string().c_str(), std::bitset<8>(pins >> 16).to_string().c_str(), 
            std::bitset<8>(pins >> 8).to_string().c_str(), std::bitset<8>(pins).to_string().c_str());


        return false;
    }

    bool cmd_pin_status_on_clock(std::string)
    {
        dump_pins_on_clock = true;
        return false;
    }

    bool cmd_enable_memory(std::string)
    {
        run_memory_operation = true;
        return false;
    }

    bool cmd_disable_memory(std::string)
    {
        run_memory_operation = false;
        return false;
    }

    bool cmd_reset(std::string value)
    {
        printf("Current pin function is %d\r\n", gpio_get_function(PIN_RESET));
        dump_pins_on_clock = false;
        if (gpio_get_function(PIN_RESET) != GPIO_FUNC_SIO)
        {
              gpio_init(PIN_RESET);
              gpio_set_dir(PIN_RESET, GPIO_OUT);
        }
        gpio_put(PIN_RESET, 0);
        add_repeating_timer_ms(1, [](repeating_timer_t *){ gpio_put(PIN_RESET, 1); return false; }, NULL, &lfo_timer);
//        gpio_put(PIN_RESET, 1);
        return false;
    }

    bool cmd_assert_databus(std::string value)
    {
        if (value.empty())
        {
            printf("Enter hex value: ");
            return true;
        }
        uint8_t data = std::stoi(value, nullptr, 16);
        printf("\r\nAsserting data bus with %02x\r\n", data);
        set_databus_dir(true);
//        uint64_t mask = 0x0000ff0000000000;
//        std::cout << "Mask is " << std::bitset<64>(mask) << ", data is " << std::bitset<64>(static_cast<uint64_t>(data) << 40) << std::endl;
//        gAfterpio_put_masked64(mask, static_cast<uint64_t>(data) << 40);
//        std::cout << "Before:" << std::endl;
//        std::cout << "sio_hw->gpio_out " << std::bitset<32>(sio_hw->gpio_out) << std::endl;
//        std::cout << "sio_hw->gpio_hi_out " << std::bitset<32>(sio_hw->gpio_hi_out) << std::endl;
//
        sio_hw->gpio_togl = (sio_hw->gpio_out ^ static_cast<uint32_t>(data) & DATA_MASK_LO);
        sio_hw->gpio_hi_togl = (sio_hw->gpio_hi_out ^ static_cast<uint32_t>(data<<8) & DATA_MASK_HI);
//
//        std::cout << "After:" << std::endl;
//        std::cout << "sio_hw->gpio_out " << std::bitset<32>(sio_hw->gpio_out) << std::endl;
//        std::cout << "sio_hw->gpio_hi_out " << std::bitset<32>(sio_hw->gpio_hi_out) << std::endl;
       
//        = (sio_hw->gpio_out ^ (uint32_t)value) & (uint32_t)mask;
//    sio_hw->gpio_hi_togl = (sio_hw->gpio_hi_out ^ (uint32_t)(value>>32u)) & (uint32_t)(mask>>32u);
        return false;
    }

    bool cmd_assert_databus_via_pullups(std::string value)
    {
        if (value.empty())
        {
            printf("Enter hex value: ");
            return true;
        }
        uint8_t data = std::stoi(value, nullptr, 16);
        printf("\r\nAsserting data bus via pullups with %02x\r\n", data);
        set_databus_dir(false);
        for (auto ii = 0; ii < 7; ii++)
        {
            gpio_set_pulls(ii, data & (2 << ii), false);
        }
//        uint64_t mask = 0x0000ff0000000000;
//        gpio_put_masked(mask, static_cast<uint64_t>(data) << 40);
        return false;
    }

    void set_databus_dir(bool out)
    {
        uint64_t mask = 0x0000ff0000000000;
        uint64_t value;
        if (out)
        {
            value = 0x0000ff0000000000;
        }
        else
        {
            value = 0x0000000000000000;
        }
        std::cout << "set_databus_dir mask " << std::bitset<64>(mask) << std::endl;
//        gpio_set_dir_masked64(mask, dir);
        std::cout << "Before:" << std::endl;
        std::cout << "gpio_oe " << std::bitset<32>(sio_hw->gpio_oe) << std::endl;
        std::cout << "gpio_hi_oe " << std::bitset<32>(sio_hw->gpio_hi_oe) << std::endl;
        sio_hw->gpio_oe_togl = (sio_hw->gpio_oe ^ (uint32_t)value) & (uint32_t)mask;
        sio_hw->gpio_hi_oe_togl = (sio_hw->gpio_hi_oe ^ (uint32_t)(value >> 32u)) & (uint32_t)(mask >> 32u);
        std::cout << "After:" << std::endl;
        std::cout << "gpio_oe " << std::bitset<32>(sio_hw->gpio_oe) << std::endl;
        std::cout << "gpio_hi_oe " << std::bitset<32>(sio_hw->gpio_hi_oe) << std::endl;
    }

    bool cmd_io(std::string value)
    {
        if (value.empty())
        {
            printf("[Dir/Pin[/Value] (i|o)NN(1|0)");
            return true;
        }
        std::string io(value.substr(0,1));
        std::string pin(value.substr(1,2));
        std::string state(value.substr(3,1));
        uint8_t pin_num = std::stoi(pin);
        printf("io: %s, pin: %u, state: %s\r\n", io.c_str(), pin_num, state.c_str());
        gpio_init(pin_num);
        gpio_set_dir(pin_num, "o" == io ? GPIO_OUT : GPIO_IN);
        if (!state.empty())
        {
            gpio_put(pin_num, "1" == state ? true : false);
        }
        else
        {
            printf("Pin %u is %u\r\n", pin_num, gpio_get(pin_num));
        }
        
        return false;
    }

} // namespace cmd_io
