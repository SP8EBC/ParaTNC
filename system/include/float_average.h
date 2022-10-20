/*
 * float_average.h
 *
 *  Created on: Oct 17, 2022
 *      Author: mateusz
 */

#ifndef INCLUDE_FLOAT_AVERAGE_H_
#define INCLUDE_FLOAT_AVERAGE_H_

#define FLOAT_AVERAGE_LN 9
#define FLOAT_INIT_VALUE -128.0f


typedef struct float_average_t {
	float values[FLOAT_AVERAGE_LN];
	float *begin, *end, *current;
}float_average_t;

void float_average(float in, float_average_t* average);
float float_get_average(const float_average_t* average);
float float_get_min(const float_average_t* average);
float float_get_max(const float_average_t* average);

#endif /* INCLUDE_FLOAT_AVERAGE_H_ */
