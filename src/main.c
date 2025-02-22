#include <stdio.h>

#define _USE_MATH_DEFINES
#include <math.h>

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

		float centered[2] = { (float)x_axis - 2048.0f, (float)y_axis - 2048.0f };
		float normalized[2] = { centered[0] / 2048.0f, centered[1] / 2048.0f };
		float mag = sqrtf(normalized[0] * normalized[0] + normalized[1] * normalized[1]);

		if (mag > 0.45f) {
			float angle = atan2f(normalized[1], normalized[0]);
			if (angle < 0.0f)
				angle += 2 * M_PI;
			printf("(%.8f %.8f) ~ %.8f, ang %.8f pi rad\n", normalized[0], normalized[1], mag, angle / M_PI);
		}

		sleep_ms(500);
	}

	return 0;
}
