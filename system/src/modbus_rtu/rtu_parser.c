/*
 * rtu_parser.c
 *
 *  Created on: 27.08.2020
 *      Author: mateusz
 */

#include "./modbus_rtu/rtu_parser.h"
#include "./modbus_rtu/rtu_return_values.h"

#define MODBUS_RTU_MIN_03_04_RESP_LN 	7	// one register to read

int32_t rtu_parser_03_04_registers(uint8_t* input, uint16_t input_ln, rtu_register_data_t* output) {
	uint32_t retval = MODBUS_RET_UNINITIALIZED;

	uint16_t data = 0;

	// iterator through input table and registers table
	int i = 0, j = 3;

	// rewind the input buffer if the first byte is not valid modbus rtu slave address
	if (*input < 1 || *input > 0xF7) {
		// don't loop here and assume that the modbus response is shifted only
		// by one byte
		input++;
	}

	// 7 bytes is the shortest meaningful Modbus RTU frame
	// with a value of single register
	if (input_ln < MODBUS_RTU_MIN_03_04_RESP_LN) {
		retval = MODBUS_RET_TOO_SHORT;
	}
	else {
		// fetch slave address
		data = *input << 8 | *(input + 1);

		// store slave address
		output->slave_address = data;

		// fetch function code
		data = *(input + 1);

		// check if the function code is correct or not
		if (data == 0x03 || data == 0x04) {
			// fetch the function result lenght in bytes
			data = *(input + 2);

			// store amount of registers in this response
			output->number_of_registers = data / 2;

			// get all registers values
			for (int i = 0; i < output->number_of_registers && i < MODBUS_RTU_MAX_REGISTERS_AT_ONCE; i++) {
				output->registers_values[i] = *(input + j) << 8 | *(input + j + 1);

				// moving to next 16bit word with next register
				j += 2;
			}

			retval = MODBUS_RET_OK;

		}
		else {
			// if not exit with an error as this isn't
			// correct parser for this modbus function
			retval = MODBUS_RET_WRONG_FUNCTION;
		}
	}

	return retval;
}

