/*
 * rtu_parser.h
 *
 *  Created on: 27.08.2020
 *      Author: mateusz
 */

#ifndef INCLUDE_MODBUS_RTU_RTU_PARSER_H_
#define INCLUDE_MODBUS_RTU_RTU_PARSER_H_

#include <stdint.h>

uint16_t rtu_parser_stream_crc(uint16_t previous_crc, uint8_t current_data);

#endif /* INCLUDE_MODBUS_RTU_RTU_PARSER_H_ */
