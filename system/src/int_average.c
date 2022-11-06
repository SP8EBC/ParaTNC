/*
 * int_average.c
 *
 *  Created on: Nov 2, 2022
 *      Author: mateusz
 */


#include "int_average.h"
#include <stdint.h>

void int_average_init(int_average_t* average) {

	average->begin = average->values;
	average->end = average->values + INT_AVERAGE_LN;

	average->current = average->begin;

	for (int i = 0; i <= INT_AVERAGE_LN; i++) {
		average->values[i] = INT_INIT_VALUE;
	}
}

void int_average(int32_t in, int_average_t* average) {
	*average->current = in;

	if (average->current == average->end) {
		average->current = average->begin;
	}
	else {
		average->current++;
	}

}

int32_t int_get_average(const int_average_t* average) {
	int32_t out = 0;
	uint8_t j = 0;

	for (int i = 0; i < INT_AVERAGE_LN; i++) {
		if (average->values[i] == INT_INIT_VALUE) {
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
		return INT_INIT_VALUE;
	}
}

int32_t int_get_min(const int_average_t* average) {
	int32_t out = INT32_MAX;

	for (int i = 0; i < INT_AVERAGE_LN; i++) {
		if (average->values[i] == INT_INIT_VALUE) {
			continue;
		}

		if (average->values[i] < out)
			out = average->values[i];
	}

	return out;
}

int32_t int_get_max(const int_average_t* average) {
	int32_t out = INT32_MIN;

	for (int i = 0; i < INT_AVERAGE_LN; i++) {
		if (average->values[i] == INT_INIT_VALUE) {
			continue;
		}

		if (average->values[i] > out)
			out = average->values[i];
	}

	return out;
}

int32_t int_get_last(const int_average_t* average) {

	int32_t out = 0;

	if (average->current == average->begin) {
		out = *average->end;
	}
	else {
		out = *(average->current - 1);
	}

	return out;
}
