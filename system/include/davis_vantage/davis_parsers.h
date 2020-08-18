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

#define DAVIS_PARSERS_TOO_SHORT_FRAME 	-1
#define DAVIS_PARSERS_CORRUPTED_CRC		-2

uint32_t davis_parsers_loop(uint8_t* input, uint16_t input_ln, davis_loop_t* output);
uint32_t davis_parsers_check_crc(uint8_t* input, uint16_t input_ln);

#endif /* INCLUDE_DAVIS_VANTAGE_DAVIS_PARSERS_H_ */
