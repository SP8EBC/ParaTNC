/*
 * rtu_parser.h
 *
 *  Created on: 27.08.2020
 *      Author: mateusz
 */

#ifndef INCLUDE_MODBUS_RTU_RTU_PARSER_H_
#define INCLUDE_MODBUS_RTU_RTU_PARSER_H_

#include <modbus_rtu/rtu_register_data_t.h>
#include <modbus_rtu/rtu_exception_t.h>
#include <stdint.h>


int32_t rtu_parser_03_04_registers(uint8_t* input, uint16_t input_ln, rtu_register_data_t* output, rtu_exception_t* exception);

#endif /* INCLUDE_MODBUS_RTU_RTU_PARSER_H_ */
