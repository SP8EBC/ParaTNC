/*
 * kiss_read_memory.h
 *
 *  Created on: Aug 3, 2024
 *      Author: mateusz
 */

#ifndef KISS_COMMUNICATION_KISS_READ_MEMORY_H_
#define KISS_COMMUNICATION_KISS_READ_MEMORY_H_

#include <stdint.h>

uint8_t kiss_read_memory_response(uint32_t address, uint8_t size, uint8_t * output_buffer, uint16_t buffer_ln);



#endif /* KISS_COMMUNICATION_KISS_READ_MEMORY_H_ */
