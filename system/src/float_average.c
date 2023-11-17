/*
 * float_average.c
 *
 *  Created on: Oct 17, 2022
 *      Author: mateusz
 */

#include "float_average.h"
#include <stdint.h>

void float_average(float in, float_average_t* average) {
	*average->current = in;

	if (average->current == average->end) {
		average->current = average->begin;
	}
	else {
		average->current++;
	}

}

float float_get_average(const float_average_t* average) {
	float out = 0.0f;
	uint8_t j = 0;

	for (int i = 0; i < FLOAT_AVERAGE_LN; i++) {
		if (average->values[i] == FLOAT_INIT_VALUE) {
			continue;
		}

		out += average->values[i];
		j++;
	}
	if (j > 0) {
		out /= j;
		return out;
	}
	else {
		return FLOAT_INIT_VALUE;
	}
}

float float_get_min(const float_average_t* average) {
	float out = 128.0f;

	for (int i = 0; i < FLOAT_AVERAGE_LN; i++) {
		if (average->values[i] == FLOAT_INIT_VALUE) {
			continue;
		}

		if (average->values[i] < out)
			out = average->values[i];
	}

	return out;
}

float dallas_get_max(const float_average_t* average) {
	float out = -128.0f;

	for (int i = 0; i < FLOAT_AVERAGE_LN; i++) {
		if (average->values[i] == FLOAT_INIT_VALUE) {
			continue;
		}

		if (average->values[i] > out)
			out = average->values[i];
	}

	return out;
}

char float_get_nonfull(const float_average_t* average) {
	for (int i = 0; i < FLOAT_AVERAGE_LN; i++) {
		if (average->values[i] == FLOAT_INIT_VALUE) {
			return 1;
		}
	}

	return 0;
}

