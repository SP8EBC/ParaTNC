/*
 * float_to_string.c
 *
 *  Created on: May 12, 2021
 *      Author: mateusz
 */

#include "float_to_string.h"

#include <string.h>
#include <stdio.h>
#include <math.h>

void float_to_string(float in, char * out, uint8_t ln, int8_t precision, int8_t integer_ln) {

	int32_t integer_part = 0, decimal_part = 0;

	float decimal_in_float = 0.0f;

	integer_part = (int32_t)in;

	// decimal part stored as float
	decimal_in_float = in - integer_part;

	// calcaulating decimal precision multiplier
	precision = pow(10, precision);

	// moving decimal point according to precision required by the caller
	decimal_in_float *= precision;

	// converting decimal part into integer variable
	decimal_part = (int32_t) decimal_in_float;

	// clearing output
	memset (out, 0x00, ln);

	if (integer_ln == 3) {
		snprintf(out, ln, "%03ld.%02ld", integer_part, decimal_part);
	}
	else if (integer_ln == 4) {
		snprintf(out, ln, "%04ld.%02ld", integer_part, decimal_part);
	}
	else if (integer_ln == 5) {
		snprintf(out, ln, "%05ld.%02ld", integer_part, decimal_part);
	}
	else {
		snprintf(out, ln, "%ld.%02ld", integer_part, decimal_part);
	}


}
