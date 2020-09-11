/*
 * davis_global_state_t.h
 *
 *  Created on: 11.09.2020
 *      Author: mateusz
 */

#ifndef INCLUDE_DAVIS_VANTAGE_DAVIS_GLOBAL_STATE_T_H_
#define INCLUDE_DAVIS_VANTAGE_DAVIS_GLOBAL_STATE_T_H_

typedef enum dallas_global_state {
	DAVIS_GLOBAL_IDLE_OR_BLOCKING,		// set if station is idle or busy on blocking I/O comm
	DAVIS_GLOBAL_LOOP,
	DAVIS_GLOBAL_RXCHECK,
	DAVIS_GLOBAL_WAKE_UP
} dallas_global_state_t;

#endif /* INCLUDE_DAVIS_VANTAGE_DAVIS_GLOBAL_STATE_T_H_ */
