#include <cstdio>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "cmd_io.h"

const int PIN_CLOCK = 20;
namespace cmd_io
{

    void set_clock_frequency(uint32_t frequency_hz)
    {
        printf("Setting clock frequency to %d Hz\n", frequency_hz);

        int gpio = PIN_CLOCK;

        // Determine PWM slice and channel for the GPIO
        uint slice_num = pwm_gpio_to_slice_num(gpio);
        uint chan = pwm_gpio_to_channel(gpio);

        // Get system clock frequency
        uint32_t sys_clk = clock_get_hz(clk_sys);

        // Calculate divider and wrap values
        uint32_t divider = sys_clk / (frequency_hz * 65536);  // Max wrap is 65535
        if (divider < 1) divider = 1;

        uint32_t wrap = (sys_clk / (frequency_hz * divider)) - 1;

        // Configure PWM
        pwm_config config = pwm_get_default_config();
        pwm_config_set_clkdiv(&config, divider);
        pwm_config_set_wrap(&config, wrap);
        pwm_config_set_chan_level(&config, chan, wrap / 2);  // 50% duty cycle

        pwm_init(slice_num, &config, true);
        pwm_set_gpio_level(gpio, wrap / 2);
        gpio_set_function(gpio, GPIO_FUNC_PWM);
    }
}

} // namespace cmd_io
