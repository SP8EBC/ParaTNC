/*
 * rtu_request.c
 *
 *  Created on: 18.09.2020
 *      Author: mateusz
 */

#include <stdarg.h>
#include <string.h>

#include "./modbus_rtu/rtu_request.h"
#include "./modbus_rtu/rtu_return_values.h"
#include "./modbus_rtu/rtu_crc.h"

int32_t rtu_request_03_04_registers(int8_t input_or_holding, uint8_t* output, uint8_t output_ln, uint8_t* output_ln_used, uint8_t slave_address, uint16_t base_register, uint8_t number_of_registers) {

	int32_t retval = MODBUS_RET_UNINITIALIZED;

	uint16_t crc = 0;

	// check if there is a room for a RTU frame
	if (output == 0x00 || output_ln < 8) {
		retval = MODBUS_RET_TOO_SHORT;
	}
	else {
		// initialize the output buffer for RTU binary frame
		memset(output, 0x00, output_ln);

		// put the slave address
		*output = slave_address;

		// put the function code
		if (input_or_holding == 0) {
			*(output + 1) = 0x04;
		}
		else {
			*(output + 1) = 0x03;
		}

		// put the base address to be read from the slave
		*(output + 2) = (base_register & 0xFF00) >> 8;
		*(output + 3) = base_register & 0xFF;

		// put the numbers of register to be read from slave
		*(output + 4) = 0x00;	// all in all modbus RTU can transfer no more than 125 at once
		*(output + 5) = number_of_registers;

		// calculate the CRC from the content
		crc = rtu_crc_buffer(output, 6);

		// append the crc value
		*(output + 6) = (crc & 0xFF00) >> 8;
		*(output + 7) = crc & 0xFF;

		retval = MODBUS_RET_OK;
	}

	return retval;
}
