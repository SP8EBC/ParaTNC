/*
 * rtu_crc.h
 *
 *  Created on: 18.09.2020
 *      Author: mateusz
 */

#ifndef INCLUDE_MODBUS_RTU_RTU_CRC_H_
#define INCLUDE_MODBUS_RTU_RTU_CRC_H_

#include <stdint.h>

inline uint16_t rtu_crc_stream(uint16_t previous_crc, uint8_t current_data) {
	int i;

	previous_crc ^= (uint16_t)current_data;
	for (i = 0; i < 8; ++i) {
		if (previous_crc & 1) {
			previous_crc = (previous_crc >> 1);
			previous_crc = (previous_crc) ^ 0xA001;
		}
		else
			previous_crc = (previous_crc >> 1);
	}

	return previous_crc;
}

inline uint16_t rtu_crc_buffer(uint8_t* buffer_ptr, uint8_t buffer_ln) {
	uint16_t crc = 0xFFFF;

// 		https://ctlsys.com/support/how_to_compute_the_modbus_rtu_message_crc/

	for (int pos = 0; pos < buffer_ln; pos++) {
	crc ^= (uint16_t)buffer_ptr[pos];          // XOR byte into least sig. byte of crc

	for (int i = 8; i != 0; i--) {    // Loop over each bit
		if ((crc & 0x0001) != 0) {      // If the LSB is set
			crc >>= 1;                    // Shift right and XOR 0xA001
			crc ^= 0xA001;
		}
		else                            // Else LSB is not set
			crc >>= 1;                    // Just shift right
		}
	}
	// Note, this number has low and high bytes swapped, so use it accordingly (or swap bytes)
	return crc;
}

#endif /* INCLUDE_MODBUS_RTU_RTU_CRC_H_ */
