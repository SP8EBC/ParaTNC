/*
 * parser.h
 *
 *  Created on: 10.03.2019
 *      Author: mateusz
 */

#ifndef VE_DIRECT_PROTOCOL_PARSER_H_
#define VE_DIRECT_PROTOCOL_PARSER_H_

#define VE_DIRECT_INVALID_INP_STR 				-1
#define VE_DIRECT_STRING_END_REACH_TO_EARLY 	-2

#include "raw_struct.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void ve_direct_parser_init(void);
void ve_direct_cut_to_checksum(uint8_t* input, uint16_t input_ln, uint16_t* target_ln);
void ve_direct_validate_checksum(uint8_t* input, uint16_t input_ln, uint8_t* valid);
int ve_direct_parse_to_raw_struct(uint8_t* input, uint16_t input_ln, ve_direct_raw_struct* out);
void ve_direct_add_to_average(ve_direct_raw_struct* in);
void ve_direct_get_averages(int16_t* battery_current, uint16_t* battery_voltage, uint16_t* pv_voltage, uint16_t* load_current);


#ifdef __cplusplus
}
#endif

#endif /* VE_DIRECT_PROTOCOL_PARSER_H_ */
