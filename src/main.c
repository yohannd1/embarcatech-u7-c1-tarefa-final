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

#define BUTTON_A_PIN 5
#define BUTTON_B_PIN 6
#define BUTTON_J_PIN 22
#define DEBOUNCING_TIME_US 22000

#define MUS_NOTE_COUNT 24
#define MUS_BASE_FREQ 261.6f // C-4

// Mapa de nota->frequência. A i-ésima casa é a nota a i semitons de
// MUS_BASE_FREQ.
static float notes[MUS_NOTE_COUNT] = { 0.0f };
static volatile int c1 = 0, c2 = 0, c3 = 0;

static void calc_joystick(uint16_t x_axis_raw, uint16_t y_axis_raw, float *angle, float *magnitude);
static void on_press(uint gpio, uint32_t events);

int main(void) {
	stdio_init_all();

	adc_init();

	// inicializar os wrappers para os joysticks
	adc_wrapper_t joystick_x, joystick_y;
	adc_wrapper_init(&joystick_x, JOYSTICK_X_INPUT, JOYSTICK_X_PIN);
	adc_wrapper_init(&joystick_y, JOYSTICK_Y_INPUT, JOYSTICK_Y_PIN);

	// inicializar as notas
	for (int i = 0; i < MUS_NOTE_COUNT; i++)
		notes[i] = MUS_BASE_FREQ * powf(2.0f, (float)i / 12.0f);

	// inicializar o buzzer
	buzzer_t bz;
	buzzer_init(&bz, BUZZER_PIN);

	// inicializar o botão A
	gpio_init(BUTTON_A_PIN);
	gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
	gpio_pull_up(BUTTON_A_PIN);

	// inicializar o botão B
	gpio_init(BUTTON_B_PIN);
	gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
	gpio_pull_up(BUTTON_B_PIN);

	// inicializar o botão J
	gpio_init(BUTTON_J_PIN);
	gpio_set_dir(BUTTON_J_PIN, GPIO_IN);
	gpio_pull_up(BUTTON_J_PIN);

	// ativar interrupt para os botões
	gpio_set_irq_enabled_with_callback(BUTTON_A_PIN, GPIO_IRQ_EDGE_FALL, true, &on_press);
	gpio_set_irq_enabled_with_callback(BUTTON_J_PIN, GPIO_IRQ_EDGE_FALL, true, &on_press);

	while (true) {
		uint16_t x_axis_raw = adc_wrapper_read(joystick_x);
		uint16_t y_axis_raw = adc_wrapper_read(joystick_y);

		float angle, magnitude;
		calc_joystick(x_axis_raw, y_axis_raw, &angle, &magnitude);

		printf("~ %d %d %d\n", c1, c2, c3);

		if (magnitude > 0.45f) {
			uint32_t note = angle / (2.0f * M_PI) * 12.0f;
			float freq = notes[note];
			printf("%.5f, %.5f pi rad (note %u, freq=%.2f)\n", magnitude, angle / M_PI, note, freq);

			buzzer_play(&bz, freq, 500);
		} else {
			sleep_ms(10);
		}
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

static void on_press(uint gpio, uint32_t events) {
	uint32_t current_time = to_us_since_boot(get_absolute_time());

#define DEBOUNCE_AND_DO(_pin, _lt_var, _block) \
	static volatile uint32_t _lt_var = 0; \
	if (gpio == _pin) { \
		if (current_time - _lt_var > DEBOUNCING_TIME_US) { \
			if (!gpio_get(_pin)) _block \
			_lt_var = current_time; \
		} \
	}

	DEBOUNCE_AND_DO(BUTTON_A_PIN, last_time_a, {
		c1++;
	});

	// O estado do botão B não é verificado aqui porque ele se encaixa melhor no
	// loop principal.
	// DEBOUNCE_AND_DO(BUTTON_B_PIN, last_time_b, {});

	DEBOUNCE_AND_DO(BUTTON_J_PIN, last_time_j, {
		c3++;
	});

#undef DEBOUNCE_AND_DO
}
