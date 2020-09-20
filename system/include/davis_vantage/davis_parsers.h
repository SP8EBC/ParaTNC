/*
 * davis_parsers.h
 *
 *  Created on: 10.08.2020
 *      Author: mateusz
 */

#ifndef INCLUDE_DAVIS_VANTAGE_DAVIS_PARSERS_H_
#define INCLUDE_DAVIS_VANTAGE_DAVIS_PARSERS_H_

#include <stdint.h>

#include "davis_loop_t.h"

#define DAVIS_PARSERS_OK				0
#define DAVIS_PARSERS_TOO_SHORT_FRAME 	-1
#define DAVIS_PARSERS_CORRUPTED_CRC		-2
#define DAVIS_PARSERS_WRONG_CONTENT		-3

inline static uint8_t is_digit(char c) {
	if (c >= 0x30 && c <= 0x39)
		return 1;
	else
		return 0;
}

uint32_t davis_parsers_loop(uint8_t* input, uint16_t input_ln, davis_loop_t* output);
uint32_t davis_parsers_loop2(uint8_t* input, uint16_t input_ln, davis_loop_t* output);
uint32_t davis_parsers_check_crc(uint8_t* input, uint16_t input_ln);
uint32_t davis_parsers_rxcheck(		uint8_t* input,
									uint16_t input_ln,
									uint16_t* total_packet_received,
									uint16_t* total_packet_missed,
									uint16_t* resynchronizations,
									uint16_t* packets_in_the_row,
									uint16_t* crc_errors);

/*
 *
 * uint16_t davis_base_total_packet_received = 0;

uint16_t davis_base_total_packet_missed = 0;

uint16_t davis_base_resynchronizations = 0;

uint16_t davis_base_packets_in_the_row = 0;

uint16_t davis_base_crc_errors = 0;
 *
 */

#endif /* INCLUDE_DAVIS_VANTAGE_DAVIS_PARSERS_H_ */
