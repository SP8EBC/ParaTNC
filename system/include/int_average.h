/*
 * int_average.h
 *
 *  Created on: Nov 2, 2022
 *      Author: mateusz
 */

#ifndef INCLUDE_INT_AVERAGE_H_
#define INCLUDE_INT_AVERAGE_H_

#include <stdint.h>

#define INT_AVERAGE_LN 4
#define INT_INIT_VALUE (INT32_MIN + 1)


typedef struct int_average_t {
	int32_t values[INT_AVERAGE_LN + 1];
	int32_t *begin, *end, *current;
}int_average_t;

void int_average_init(int_average_t* average);
void int_average(int32_t in, int_average_t* average);
int32_t int_get_average(const int_average_t* average);
int32_t int_get_min(const int_average_t* average);
int32_t int_get_max(const int_average_t* average);


#endif /* INCLUDE_INT_AVERAGE_H_ */
