#include <stdio.h>
#include "pico/stdlib.h"

#include "utils.h"

void utils_panicf(const char *fmt, ...) {
	while (true) {
		printf("Fatal error: ");

		va_list args;
		va_start(args, fmt);
		vprintf(fmt, args);
		va_end(args);

		printf("\n");
		sleep_ms(2000);
	}
}
