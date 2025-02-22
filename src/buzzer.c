#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/pwm.h"

#include "buzzer.h"

// current strategy: when the clock is at 125MHz, a 250 divisor makes it so the
// frequency is 500KHz/(wrap+1), and since the max wrap+1 value is 65536, our
// range is 7Hz to 500KHz.
const float clock_div = 250.0f;

void buzzer_init(buzzer_t *bz, uint pin) {
	bz->pin = pin;

	gpio_set_function(bz->pin, GPIO_FUNC_PWM);
	bz->slice = pwm_gpio_to_slice_num(bz->pin);

	pwm_config config = pwm_get_default_config();
	pwm_config_set_clkdiv(&config, clock_div);
	pwm_init(bz->slice, &config, true);

	pwm_set_gpio_level(bz->pin, 0); // initially turn off PWM
}

void buzzer_deinit(buzzer_t *bz) {
	bz->pin = 0;
	bz->slice = 0;
}

void buzzer_play(buzzer_t *bz, float frequency, uint duration_ms) {
	uint32_t clock_freq = clock_get_hz(clk_sys);
	uint32_t wrap = (uint32_t)(clock_freq / (clock_div * frequency) - 1.0f);

	pwm_set_wrap(bz->slice, wrap);
	pwm_set_gpio_level(bz->pin, wrap * 0.5f);

	sleep_ms(duration_ms);

	pwm_set_gpio_level(bz->pin, 0);
}
