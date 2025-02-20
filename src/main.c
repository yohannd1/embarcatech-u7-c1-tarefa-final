#include <stdio.h>
#include "pico/stdlib.h"

#include "utils.h"

int main(void) {
	stdio_init_all();

	utils_panicf("uh....");

	while (true)
		sleep_ms(10000);

	return 0;
}
