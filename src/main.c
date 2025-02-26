#include <stdio.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include "pico/stdlib.h"
#include "hardware/adc.h"

#include "utils.h"
#include "adc_wrapper.h"
#include "buzzer.h"
#include "ssd1306.h"

#define BUZZER_PIN 21

#define JOYSTICK_X_PIN 27
#define JOYSTICK_X_INPUT 1
#define JOYSTICK_Y_PIN 26
#define JOYSTICK_Y_INPUT 0

// Configuração dos botões
#define BUTTON_A_PIN 5
#define BUTTON_B_PIN 6
#define BUTTON_J_PIN 22
#define DEBOUNCING_TIME_US 200000

// Configuração do display ssd1306
#define DISPLAY_SDA_PIN 14
#define DISPLAY_SCL_PIN 15
#define DISPLAY_I2C_PORT i2c1
#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64

#define MUS_NOTE_COUNT 96
#define MUS_BASE_OCTAVE 2 // C-2
#define MUS_BASE_FREQ (261.6f / 4.0f) // C-2

const float BASE_SPEED_HZ = 60.0f;
const float BASE_PERIOD_US = 1000000.0f / BASE_SPEED_HZ;

// Magnitude mínima do vetor do joystick para reconhecimento de uma nota
const float MIN_MAGNITUDE = 0.45f;

// Mapa de nota->frequência, onde a i-ésima casa é a nota i semitons acima de
// MUS_BASE_FREQ.
static float notes[MUS_NOTE_COUNT] = { 0.0f };

// Nota utilizada como base para as notas tocadas. Para subir uma oitava,
// adiciona-se 12, e para diminuir subtrai-se 12.
static volatile _Atomic uint16_t base_note = 24;

static void calc_joystick(uint16_t x_axis_raw, uint16_t y_axis_raw, float *angle, float *magnitude);
static void on_press(uint gpio, uint32_t events);
static const char *getNoteLetter(uint16_t note);
static uint8_t getNoteOctave(uint16_t note);

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

	// inicializar o display
	i2c_init(DISPLAY_I2C_PORT, 400000); // 400KHz
	gpio_set_function(DISPLAY_SDA_PIN, GPIO_FUNC_I2C);
	gpio_set_function(DISPLAY_SCL_PIN, GPIO_FUNC_I2C);
	gpio_pull_up(DISPLAY_SDA_PIN);
	gpio_pull_up(DISPLAY_SCL_PIN);

	// inicializar o display
	ssd1306_t display;
	if (!ssd1306_init(&display, DISPLAY_WIDTH, DISPLAY_HEIGHT, false, 0x3C, DISPLAY_I2C_PORT))
		utils_panicf("falha ao inicializar o display OLED");

	// limpar o display
	ssd1306_fill(&display, 0);
	ssd1306_send_data(&display);

	char lines[2][32] = { "-- EmbarcaTech --", {0} };

	while (true) {
		// verificar se o botão B está pressionado
		bool should_play = !gpio_get(BUTTON_B_PIN);

		// captura de dados do joystick e controle do buzzer
		if (should_play) {
			uint16_t x_axis_raw = adc_wrapper_read(joystick_x);
			uint16_t y_axis_raw = adc_wrapper_read(joystick_y);

			// calcular o ângulo e magnitude do vetor do joystick
			float angle, magnitude;
			calc_joystick(x_axis_raw, y_axis_raw, &angle, &magnitude);

			if (magnitude > MIN_MAGNITUDE) {
				uint16_t note = base_note + angle / (2.0f * M_PI) * 12.0f;

				// se a nota for alta demais, limitar
				if (note >= MUS_NOTE_COUNT)
					note = MUS_NOTE_COUNT - 1;

				float freq = notes[note];

				// mostrar informações de debug no serial
				printf("%.5f, %.5f pi rad (nota %u, freq %.2f Hz)\n", magnitude, angle / M_PI, note, freq);

				// gerar texto para a tela
				snprintf(lines[1], 32, "Nota: %s%u", getNoteLetter(note), getNoteOctave(note));

				buzzer_start(&bz, freq);
			} else {
				buzzer_stop(&bz);
			}
		} else {
			buzzer_stop(&bz);
		}

		// controle do display
		ssd1306_fill(&display, 1);
		ssd1306_rect(&display, 3, 3, 122, 58, 0, true);

		uint8_t x = 10, y = 10;
		ssd1306_draw_string(&display, lines[0], &x, &y);
		x = 10, y += 8;
		ssd1306_draw_string(&display, lines[1], &x, &y);

		ssd1306_send_data(&display);

		sleep_us(BASE_PERIOD_US);
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
		// Subir uma oitava quando o botão A é pressionado.
		base_note += 12;
	});

	// O estado do botão B não é verificado aqui porque ele se encaixa melhor no
	// loop principal.
	// DEBOUNCE_AND_DO(BUTTON_B_PIN, last_time_b, {});

	DEBOUNCE_AND_DO(BUTTON_J_PIN, last_time_j, {
		// Descer uma oitava quando o botão J é pressionado.
		if (base_note >= 12) base_note -= 12;
	});

#undef DEBOUNCE_AND_DO
}

static const char *getNoteLetter(uint16_t note) {
	const char *notes[12] = { "C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-" };
	return notes[note % 12];
}

static uint8_t getNoteOctave(uint16_t note) {
	return (note / 12) + MUS_BASE_OCTAVE;
}
