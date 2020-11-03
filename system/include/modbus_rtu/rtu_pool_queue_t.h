/*
 * rtu_pool_queue_t.h
 *
 *  Created on: 18.09.2020
 *      Author: mateusz
 */

#ifndef INCLUDE_MODBUS_RTU_RTU_POOL_QUEUE_T_H_
#define INCLUDE_MODBUS_RTU_RTU_POOL_QUEUE_T_H_

#include <stdint.h>

#include "modbus_rtu/rtu_configuration.h"

/**
 * This type stores the queue of RTU functions to be queued
 * with theirs parameters (if there is any)
 */
typedef struct rtu_pool_queue {

	int8_t it;

	uint8_t function_id[RTU_POOL_QUEUE_LENGHT];

	void* function_parameter[RTU_POOL_QUEUE_LENGHT];

	uint32_t last_successfull_call_to_function[RTU_POOL_QUEUE_LENGHT];

	uint16_t number_of_errors[RTU_POOL_QUEUE_LENGHT];

} rtu_pool_queue_t;

#endif /* INCLUDE_MODBUS_RTU_RTU_POOL_QUEUE_T_H_ */
