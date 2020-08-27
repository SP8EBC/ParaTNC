/*
 * rtu_parser.c
 *
 *  Created on: 27.08.2020
 *      Author: mateusz
 */

#include "./modbus_rtu/rtu_parser.h"

uint16_t rtu_parser_stream_crc(uint16_t previous_crc, uint8_t current_data) {
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
