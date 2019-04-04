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

#define VE_DIRECT_MAX_FRAME_LN	186

#include "raw_struct.h"
#include "average_struct.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void ve_direct_parser_init(ve_direct_raw_struct* raw_struct, ve_direct_average_struct* avg_struct);
void ve_direct_cut_to_checksum(uint8_t* input, uint16_t input_ln, uint16_t* target_ln);
void ve_direct_validate_checksum(uint8_t* input, uint16_t input_ln, uint8_t* valid);
int ve_direct_parse_to_raw_struct(uint8_t* input, uint16_t input_ln, ve_direct_raw_struct* out);
void ve_direct_add_to_average(ve_direct_raw_struct* in, ve_direct_average_struct* avg_struct);
void ve_direct_get_averages(ve_direct_average_struct* avg_struct, int16_t* battery_current, uint16_t* battery_voltage, uint16_t* pv_voltage, uint16_t* load_current);
void ve_direct_set_sys_voltage(ve_direct_raw_struct* in, uint8_t* sys_voltage);
void ve_direct_store_errors(ve_direct_raw_struct* input, ve_direct_error_reason* err_reason);
void ve_direct_error_to_string(ve_direct_error_reason input, char* output, int8_t output_ln);
void ve_direct_state_to_string(ve_direct_system_state input, char* output, int8_t output_ln);

#ifdef __cplusplus
}
#endif

#endif /* VE_DIRECT_PROTOCOL_PARSER_H_ */
