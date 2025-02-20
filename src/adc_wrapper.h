#ifndef _ADC_WRAPPER_H
#define _ADC_WRAPPER_H

#include "pico/stdlib.h"

typedef struct adc_wrapper_t {
	uint input;
	uint gpio;
} adc_wrapper_t;

/**
 * Inicializa um pino para o modo ADC.
 *
 * É necessário já ter chamado `adc_init()`.
 */
void adc_wrapper_init(adc_wrapper_t *aw, uint input, uint gpio);

uint16_t adc_wrapper_read(adc_wrapper_t aw);

#endif
