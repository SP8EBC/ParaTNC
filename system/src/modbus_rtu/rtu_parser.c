/*
 * rtu_parser.c
 *
 *  Created on: 27.08.2020
 *      Author: mateusz
 */

#include "./modbus_rtu/rtu_parser.h"
#include "./modbus_rtu/rtu_return_values.h"

#define MODBUS_RTU_MIN_03_04_RESP_LN 	7	// one register to read

/**
* 2021-03-21 03:06:56 CET: SR6WXP>AKLPRZ,WIDE2-1,qAS,SQ6NDL:=5038.00N101747.00E# DigiW1 + WX ~ Lotnisko Polska Nowa Wies ~ https://www.facebook.com/aeroklub.opolski
* 2021-03-21 03:06:58 CET: SR6WXP>AKLPRZ,WIDE2-1,qAR,SR9NSK:T#237,023,004,001,000,122,10000110
* 2021-03-21 03:07:58 CET: SR6WXP>AKLPRZ,WIDE2-1,qAO,SR6NKB:>F1V FFED, F2V 47, F3V 0, F4V 0, F5V 0, F6V 0
* 2021-03-21 03:08:59 CET: SR6WXP>AKLPRZ,WIDE2-1,qAR,SR9NSK:!5038.00N/01747.00E_344/006g011t025r...p...P...b09952h71
* 2021-03-21 03:11:59 CET: SR6WXP>AKLPRZ,WIDE2-1,qAO,SR6NKB:>F1V FFEE, F2V 47, F3V 0, F4V 0, F5V 0, F6V 0
* 2021-03-21 03:13:00 CET: SR6WXP>AKLPRZ,WIDE2-1,qAO,SR6NKB:!5038.00N/01747.00E_352/005g009t025r...p...P...b09951h71
* 2021-03-21 03:16:00 CET: SR6WXP>AKLPRZ,WIDE2-1,qAO,SR6NKB:>F1V FFEE, F2V 47, F3V 0, F4V 0, F5V 0, F6V 0
* 2021-03-21 03:17:01 CET: SR6WXP>AKLPRZ,WIDE2-1,qAR,SR9NSK:!5038.00N/01747.00E_348/006g008t025r...p...P...b09950h71
* 
* 2021-03-21 05:36:27 CET: SR6WXP>AKLPRZ,WIDE2-1,qAS,SQ6NDL:>F1V FFFA, F2V 46, F3V 0, F4V 0, F5V 0, F6V 0
* 2021-03-21 05:37:28 CET: SR6WXP>AKLPRZ,WIDE2-1,qAS,SQ6NDL:!5038.00N/01747.00E_339/003g007t027r...p...P...b09923h70
* 2021-03-21 05:37:30 CET: SR6WXP>AKLPRZ,WIDE2-1,qAR,SR9NSK:T#252,026,004,001,000,122,10000110
* 2021-03-21 05:40:30 CET: SR6WXP>AKLPRZ,WIDE2-1,qAS,SQ6NDL:>F1V FFFA, F2V 46, F3V 0, F4V 0, F5V 0, F6V 0
*/

int32_t rtu_parser_03_04_registers(uint8_t* input, uint16_t input_ln, rtu_register_data_t* output, rtu_exception_t* exception) {
	uint32_t retval = MODBUS_RET_UNINITIALIZED;

	uint16_t data = 0;

	uint8_t slave_address_from_frame = 0;

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
		data = *input;

		// TODO: store slave address
		slave_address_from_frame = data;

		// fetch function code
		data = *(input + 1);

		// if the exception flag is set
		if ((data & 0x80) > 0) {
			// parse the exception value
			*exception = rtu_exception_from_frame_data(data);

			// and set the return value
			retval = MODBUS_RET_GOT_EXCEPTION;
		}

		// check if the function code is correct or not
		else if (data == 0x03 || data == 0x04) {

			// check if this is an answer from the slave we expect
			if (slave_address_from_frame == output->slave_address) {
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
				retval = MODBUS_RET_UNEXP_SLAVE_ADR;
			}

		}
		else {
			// if not exit with an error as this isn't
			// correct parser for this modbus function
			retval = MODBUS_RET_WRONG_FUNCTION;
		}
	}

	return retval;
}

