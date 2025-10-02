#include <cstdio>
#include <iomanip>
#include <iostream>
#include <bitset>
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/pwm.h"
#include "cmd_io.h"
#include "pin_defs.h"

namespace cmd_io
{
    repeating_timer_t lfo_timer;
    bool pin_state = false;

    void init(void)
    {
        for (auto ii = 0; ii < 16; ii++)
        {
            gpio_init(ii);
            gpio_set_dir(ii, GPIO_IN);
        }
        for (auto ii = 40; ii < 48; ii++)
        {
            gpio_init(ii);
            gpio_set_dir(ii, GPIO_IN);
        }
    }

    static bool lfo_timer_callback(repeating_timer_t *t)
    {
        pin_state = !pin_state;
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
    bool cmd_pin_status(std::string value)
    {
        uint64_t pins = gpio_get_all64();
        std::cout << "Data          AddH AddL" << std::endl \
            << std::hex << std::setw(2) << std::setfill('0') << (pins >> 56) \
            << "    " << static_cast<uint8_t>(pins >> 48) << std::setw(4) << " " << static_cast<uint16_t>(pins >> 32) << " " \
            << " " << std::setw(4) << static_cast<uint16_t>(pins >> 16) \
            << " " << std::setw(4) << static_cast<uint16_t>(pins) << std::endl \
            << "Data                                                  AddressH AddressL" << std::endl \
            << "76543210                                              FEDCBA98 76543210" << std::endl \
            << std::bitset<8>(pins >> 56) << " " << std::bitset<8>(pins >> 48)  \
            << " " << std::bitset<8>(pins >> 40) << " " << std::bitset<8>(pins >> 32) \
            << " " << std::bitset<8>(pins >> 24) << " " << std::bitset<8>(pins >> 16) \
            << " " << std::bitset<8>(pins >> 8) << " " << std::bitset<8>(pins) << std::endl;
        return false;
    }

    bool cmd_reset(std::string value)
    {
        printf("Current pin function is %d\r\n", gpio_get_function(PIN_RESET));
        if (gpio_get_function(PIN_RESET) != GPIO_FUNC_SIO)
        {
              gpio_init(PIN_RESET);
              gpio_set_dir(PIN_RESET, GPIO_OUT);
        }
        gpio_put(PIN_RESET, 0);
        add_repeating_timer_ms(1, [](repeating_timer_t *){ gpio_put(PIN_RESET, 1); return false; }, NULL, &lfo_timer);
        gpio_put(PIN_RESET, 1);
        return false;
    }

} // namespace cmd_io
