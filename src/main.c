#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

#include "utils.h"
#include "adc_wrapper.h"

#define JOYSTICK_X_PIN 27
#define JOYSTICK_X_INPUT 1
#define JOYSTICK_Y_PIN 26
#define JOYSTICK_Y_INPUT 0

int main(void) {
	stdio_init_all();

	adc_init();

	adc_wrapper_t joystick_x;
	adc_wrapper_init(&joystick_x, JOYSTICK_X_INPUT, JOYSTICK_X_PIN);

	adc_wrapper_t joystick_y;
	adc_wrapper_init(&joystick_y, JOYSTICK_Y_INPUT, JOYSTICK_Y_PIN);

	while (true) {
		uint16_t x_axis = adc_wrapper_read(joystick_x);
		uint16_t y_axis = adc_wrapper_read(joystick_y);
		printf("X=%u, Y=%u\n", x_axis, y_axis);

		sleep_ms(500);
	}

	return 0;
}
