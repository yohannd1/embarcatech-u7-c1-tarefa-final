#include <stdio.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include "pico/stdlib.h"
#include "hardware/adc.h"

#include "utils.h"
#include "adc_wrapper.h"
#include "buzzer.h"

#define BUZZER_PIN 21
#define JOYSTICK_X_PIN 27
#define JOYSTICK_X_INPUT 1
#define JOYSTICK_Y_PIN 26
#define JOYSTICK_Y_INPUT 0

static float notes[24] = { 0.0f };

static void calc_joystick(uint16_t x_axis_raw, uint16_t y_axis_raw, float *angle, float *magnitude);

int main(void) {
	stdio_init_all();

	adc_init();

	// inicializar os wrappers para os joysticks
	adc_wrapper_t joystick_x, joystick_y;
	adc_wrapper_init(&joystick_x, JOYSTICK_X_INPUT, JOYSTICK_X_PIN);
	adc_wrapper_init(&joystick_y, JOYSTICK_Y_INPUT, JOYSTICK_Y_PIN);

	// inicializar o buzzer
	buzzer_t bz;
	buzzer_init(&bz, BUZZER_PIN);

	// buzzer_play(&bz, 440.0f, 500);

	while (true) {
		uint16_t x_axis_raw = adc_wrapper_read(joystick_x);
		uint16_t y_axis_raw = adc_wrapper_read(joystick_y);

		float angle, magnitude;
		calc_joystick(x_axis_raw, y_axis_raw, &angle, &magnitude);

		if (magnitude > 0.45f) {
			float note = angle / (2.0f * M_PI) * 12.0f;

			printf("%.5f, %.5f pi rad (note %.2f)\n", magnitude, angle / M_PI, note);
		}

		sleep_ms(500);
	}

	return 0;
}

static void calc_joystick(uint16_t x_axis_raw, uint16_t y_axis_raw, float *angle, float *magnitude) {
	float centered[2] = { (float)x_axis_raw - 2048.0f, (float)y_axis_raw - 2048.0f };
	float normalized[2] = { centered[0] / 2048.0f, centered[1] / 2048.0f };

	*magnitude = sqrtf(normalized[0] * normalized[0] + normalized[1] * normalized[1]);

	*angle = atan2f(normalized[1], normalized[0]);
	if (*angle < 0.0f)
		*angle += 2 * M_PI;
}
