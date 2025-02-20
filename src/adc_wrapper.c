#include "adc_wrapper.h"
#include "hardware/adc.h"

void adc_wrapper_init(adc_wrapper_t *aw, uint input, uint gpio) {
	aw->input = input;
	aw->gpio = gpio;
	adc_gpio_init(aw->input);
}

uint16_t adc_wrapper_read(adc_wrapper_t aw) {
	adc_select_input(aw.input);
	return adc_read();
}
