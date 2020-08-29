/*
 * rtu_serial_io.c
 *
 *  Created on: 27.08.2020
 *      Author: mateusz
 */

#include "modbus_rtu/rtu_serial_io.h"
#include "modbus_rtu/rtu_parser.h"

#include "drivers/serial.h"


/**
 * CRC value after the last call to rtu_serial_callback
 */
uint16_t rtu_serial_previous_crc = 0;

/**
 * A callback for stream CRC calculation
 */
uint8_t rtu_serial_callback(uint8_t current_data, const uint8_t * const rx_buffer, uint16_t rx_bytes_counter) {

	uint8_t retval = 0;

	uint16_t new_crc = 0;

	// calculate new crc
	new_crc = rtu_parser_stream_crc(rtu_serial_previous_crc, current_data);

	// if the new CRC value equals 0x0000 it means that this was MSB
	// of CRC from correctly received Modbus-RTU frame
	if (new_crc == 0) {
		// return '1' to terminate the transmission
		retval = 1;
	}
	else {
		rtu_serial_previous_crc = new_crc;
	}

	return retval;
}


