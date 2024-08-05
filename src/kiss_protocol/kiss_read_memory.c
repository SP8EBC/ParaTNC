/*
 * kiss_read_memory.c
 *
 *  Created on: Aug 3, 2024
 *      Author: mateusz
 */

#include "kiss_communication/kiss_read_memory.h"

uint8_t kiss_read_memory_response(uint32_t address, uint8_t size, uint8_t * output_buffer, uint16_t buffer_ln) {

	uint8_t out;

	const uint8_t* pointer = (const uint8_t*)address;
	uint16_t output_buffer_iterator = 0;

	for (int i = 0; i < size; i++) {
		output_buffer[output_buffer_iterator++] = pointer[i];
	}

	out = size;

	return out;
}


