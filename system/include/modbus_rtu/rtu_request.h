/*
 * rtu_request.h
 *
 *  Created on: 18.09.2020
 *      Author: mateusz
 */

#ifndef INCLUDE_MODBUS_RTU_RTU_REQUEST_H_
#define INCLUDE_MODBUS_RTU_RTU_REQUEST_H_

#include <stdint.h>

int32_t rtu_request_03_04_registers(int8_t input_or_holding, uint8_t* output, uint8_t output_ln, uint8_t* output_ln_used, uint8_t slave_address, uint16_t base_register, uint8_t number_of_registers);

#endif /* INCLUDE_MODBUS_RTU_RTU_REQUEST_H_ */
