/*
 * rtu_exception_t.h
 *
 *  Created on: 28.09.2020
 *      Author: mateusz
 */

#ifndef INCLUDE_MODBUS_RTU_RTU_EXCEPTION_T_H_
#define INCLUDE_MODBUS_RTU_RTU_EXCEPTION_T_H_

#include <stdint.h>

typedef enum rtu_exception {

	RTU_EXCEPTION_OK,

	RTU_EXCEPTION_ILLEGAL_FUNCTION,
	RTU_EXCETPION_ILLEGAL_ADDRESS,
	RTU_EXCEPTION_ILLEGAL_VALUE,
	RTU_EXCEPTION_SLAVE_FAIL,
	RTU_EXCEPTION_ACK,
	RTU_EXCEPTION_BUSY,
	RTU_EXCEPTION_NACK,
	RTU_EXCEPTION_PARITY_ERR,
	RTU_EXCEPTION_GW_NAVAIBLE,
	RTU_EXCEPTION_GW_TARGET_NAVAIBLE


} rtu_exception_t;

inline rtu_exception_t rtu_exception_from_frame_data(uint8_t in) {

	rtu_exception_t out = RTU_EXCEPTION_OK;

	if (in == 0x01) {
		out = RTU_EXCEPTION_ILLEGAL_FUNCTION;
	}
	else if (in == 0x02) {
		out = RTU_EXCETPION_ILLEGAL_ADDRESS;
	}
	else if (in == 0x03) {
		out = RTU_EXCEPTION_ILLEGAL_VALUE;
	}
	else if (in == 0x04) {
		out = RTU_EXCEPTION_SLAVE_FAIL;
	}
	else if (in == 0x05) {
		out = RTU_EXCEPTION_ACK;
	}
	else if (in == 0x06) {
		out = RTU_EXCEPTION_BUSY;
	}
	else if (in == 0x07) {
		out = RTU_EXCEPTION_NACK;
	}
	else if (in == 0x08) {
		out = RTU_EXCEPTION_PARITY_ERR;
	}
	else if (in == 0x0A) {
		out = RTU_EXCEPTION_GW_NAVAIBLE;
	}
	else if (in == 0x0B) {
		out = RTU_EXCEPTION_GW_TARGET_NAVAIBLE;
	}
	else {
		;
	}

	return out;
}

#endif /* INCLUDE_MODBUS_RTU_RTU_EXCEPTION_T_H_ */
