/*
 * rtu_frame_t.h
 *
 *  Created on: 27.08.2020
 *      Author: mateusz
 */

#ifndef INCLUDE_MODBUS_RTU_RTU_FRAME_T_H_
#define INCLUDE_MODBUS_RTU_RTU_FRAME_T_H_

#include <stdint.h>

#define RTU_MAXIMUM_DATA_LN 64

typedef struct rtu_frame_t {
	uint8_t address;

	uint8_t function;

	uint8_t data[RTU_MAXIMUM_DATA_LN];

	uint16_t crc;
}rtu_frame_t;

#endif /* INCLUDE_MODBUS_RTU_RTU_FRAME_T_H_ */
