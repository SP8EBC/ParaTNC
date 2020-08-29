/*
 * rtu_parser.h
 *
 *  Created on: 27.08.2020
 *      Author: mateusz
 */

#ifndef INCLUDE_MODBUS_RTU_RTU_PARSER_H_
#define INCLUDE_MODBUS_RTU_RTU_PARSER_H_

#include <stdint.h>

inline uint16_t rtu_parser_stream_crc(uint16_t previous_crc, uint8_t current_data) {
	int i;

	previous_crc ^= (uint16_t)current_data;
	for (i = 0; i < 8; ++i) {
		if (previous_crc & 1)
			previous_crc = (previous_crc) ^ 0xA001;
		else
			previous_crc = (previous_crc >> 1);
	}

	return previous_crc;
}

#endif /* INCLUDE_MODBUS_RTU_RTU_PARSER_H_ */
