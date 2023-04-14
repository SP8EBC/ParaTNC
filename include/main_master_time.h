/*
 * main_master_time.h
 *
 *  Created on: Apr 10, 2023
 *      Author: mateusz
 */

#ifndef MAIN_MASTER_TIME_H_
#define MAIN_MASTER_TIME_H_

#include <stdint.h>

extern volatile uint32_t master_time;

inline uint32_t main_get_master_time(void) {
	return master_time;
}

#endif /* MAIN_MASTER_TIME_H_ */
