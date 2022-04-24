/*
 * rtu_coil_data_t.h
 *
 *  Created on: 28.08.2020
 *      Author: mateusz
 */

#ifndef INCLUDE_MODBUS_RTU_RTU_REGISTER_DATA_T_H_
#define INCLUDE_MODBUS_RTU_RTU_REGISTER_DATA_T_H_

#include <stdint.h>

#include "../include/etc/rtu_configuration.h"

typedef struct rtu_register_data {

	uint16_t slave_address;

	uint8_t number_of_registers;

	uint16_t base_address;

	uint16_t registers_values[MODBUS_RTU_MAX_REGISTERS_AT_ONCE];

	//uint32_t last_update_timestamp;

}rtu_register_data_t;

#endif /* INCLUDE_MODBUS_RTU_RTU_REGISTER_DATA_T_H_ */
