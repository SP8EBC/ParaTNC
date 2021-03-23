/*
 * parser.c
 *
 *  Created on: 10.03.2019
 *      Author: mateusz
 */

#include "./ve_direct_protocol/parser.h"
#include "./ve_direct_protocol/average_struct.h"

#include <string.h>
#include <stdlib.h>

#define CHECKSUM_NAME_FIELD_LN_TO_DATA 10
#define LOWEST_PRINTABLE_CHARACTER 33
#define HIGHEST_PRINTABLE_CHARACTER 126

#define VE_DIRECT_MESSAGES_TO_SKIP 2

#define is_non_printable_character() (*(input + i) < LOWEST_PRINTABLE_CHARACTER || *(input + i) > HIGHEST_PRINTABLE_CHARACTER)
#define is_printable_character() (*(input + i) >= LOWEST_PRINTABLE_CHARACTER && *(input + i) <= HIGHEST_PRINTABLE_CHARACTER)

uint8_t key[9];			// the static array to store a key fetched from input file
uint8_t value[12];

uint8_t skip_counter = 0;	// counter used to skip some of VE.Direct protocol messages to spread an average over longer
							// period of time

//ve_direct_average_struct ve_avg;

static int copy_till_non_printable_char(uint8_t* input, uint16_t* input_offset, uint16_t input_ln, uint8_t* output, uint16_t output_ln) {

	uint16_t j = 0;

	uint16_t i = 0;

	for (i = *input_offset; i < input_ln; i++) {

		// if we reach any non-printable character
		if (is_non_printable_character() || j > output_ln) {

			// stop copying and rewind to first printable character
			do {
				i++;
			} while (is_non_printable_character() && i < input_ln);

			// updating an offset to input buffer
			*input_offset = i;

			if (i > input_ln) {
				return VE_DIRECT_STRING_END_REACH_TO_EARLY;
			}
			else {

				// end exit from function
				return 0;
			}
		}

		output[j++] = input[i];
	}

	*input_offset = i;

	return 0;
}

static ve_direct_key_values get_key_value_from_str(uint8_t* input) {

	const char *str = (const char*) input;

	if (strcmp("PID", str) == 0) {
		return VE_PID;
	}
	if (strcmp("FW", str) == 0) {
		return VE_FW;
	}
	if (strcmp("SER#", str) == 0) {
		return VE_SERIAL;
	}
	if (strcmp("V", str) == 0) {
		return VE_V;
	}
	if (strcmp("I", str) == 0) {
		return VE_I;
	}
	if (strcmp("VPV", str) == 0) {
		return VE_VPV;
	}
	if (strcmp("PPV", str) == 0) {
		return VE_PPV;
	}
	if (strcmp("CS", str) == 0) {
		return VE_CS;
	}
	if (strcmp("ERR", str) == 0) {
		return VE_ERR;
	}
	if (strcmp("LOAD", str) == 0) {
		return VE_LOAD;
	}
	if (strcmp("IL", str) == 0) {
		return VE_IL;
	}
	if (strcmp("H19", str) == 0) {
		return VE_H19;
	}
	if (strcmp("H20", str) == 0) {
		return VE_H20;
	}
	if (strcmp("H21", str) == 0) {
		return VE_H21;
	}
	if (strcmp("H22", str) == 0) {
		return VE_H22;
	}
	if (strcmp("H23", str) == 0) {
		return VE_H23;
	}
	if (strcmp("HSDS", str) == 0) {
		return VE_HSDS;
	}
	if (strcmp("Checksum", str) == 0) {
		return VE_CHECKSUM;
	}

	return VE_UNKNOWN;
}

void ve_direct_parser_init(ve_direct_raw_struct* raw_struct, ve_direct_average_struct* avg_struct) {
	//uint16_t size = sizeof(ve_avg);

	memset(raw_struct, 0x00, sizeof(ve_direct_raw_struct));
	memset(avg_struct, 0x00, sizeof(ve_direct_average_struct));
}


void ve_direct_cut_to_checksum(uint8_t* input, uint16_t input_ln,
		uint16_t* target_ln) {

	uint16_t i = 0;
	uint16_t checksum_start = 0;

	for (; i < input_ln - 1; i++) {
		if (*(input + i) == 'C' && *(input + i + 1) == 'h') {
			checksum_start = i;
			break;
		}
	}

	if (i >= input_ln - 1) {
		*target_ln = 1;
		return;
	}

	*target_ln = checksum_start + CHECKSUM_NAME_FIELD_LN_TO_DATA;

}

void ve_direct_validate_checksum(uint8_t* input, uint16_t input_ln, uint8_t* valid) {
	uint8_t sum = 0;

	int i = 0;

	if (input_ln < 10) {
		*valid = 0;
		return;
	}

	for (; i < input_ln; i++) {

		// workaround of the problem with loosing \n or \r characters
		// don't ask why and what this crap is all about. i had no time
		// to debug the connection between the controller and pv charging regulator

		sum += *(input + i);

		if (i > 0) {

			// if \n is the current character check if the previous one was \r
			if (*(input + i) == '\n' && *(input + i - 1) != '\r') {
				// if not assume that it was received and add it to the checksum
				sum += '\r';

				// proceed to next character
				continue;
			}

			// if \r is the current character but the next one isn't \n
			if (*(input + i) == '\r' && *(input + i + 1) != '\n') {
				// if not assume that it was received and add it to the checksum
				sum += '\n';

				// proceed to next character
				continue;
			}

		}
		else {

			// the next part of the stupid workaround
			if ((*input) == '\n') {
				sum += '\r';
			}
		}

	}

	sum %= 0xFF;

	if (sum == 0)
		*valid = 1;
	else
		*valid = 0;

	return;
}



int ve_direct_parse_to_raw_struct(uint8_t* input, uint16_t input_ln, ve_direct_raw_struct* out) {

	// local var to iterate throught
	uint16_t i = 0;

	// value used to store return value from
	int32_t ret_val = 0;

	// local variable for parsing a key value to something easly processed
	ve_direct_key_values key_enum;

	// the same but for values
	char* pointer_val = (char*) value;

	// rewind to first printable chcaracter
	while (is_non_printable_character()) {
		i++;

		// if we reach an end of the string but no printable character has been spotted
		if (i >= input_ln)
			return VE_DIRECT_INVALID_INP_STR;
	}

	// loop from the first printable character till the end of input buffer
	for (; i < input_ln;) {

		memset(key, 0x00, 9);
		memset(value, 0x00, 12);

		// start copying a key of this entry
		ret_val = copy_till_non_printable_char(input, &i, input_ln, key, sizeof(key));

		// check if data were copied correctly
 		if (ret_val == VE_DIRECT_STRING_END_REACH_TO_EARLY)
			return VE_DIRECT_INVALID_INP_STR;

		key_enum = get_key_value_from_str(key);


		if (key_enum == VE_CHECKSUM) {
			// the checksum need to be treated separately, because it consist non-printable chracers
			out->checksum  = *(input + i - 1);
		}
		else {
			// start copying a value of this entry
			ret_val = copy_till_non_printable_char(input, &i, input_ln, value, sizeof(value));

			// in case than value wasn't copiedo correctly
			if (ret_val == VE_DIRECT_STRING_END_REACH_TO_EARLY)
				return VE_DIRECT_INVALID_INP_STR;
		}

		switch (key_enum) {
			case VE_CHECKSUM:
				// the checksum is a little bit different because it can consist non-printable characters
				break;
			case VE_CS: {
				switch (value[0]) {
				case '0' : out->system_state = STATE_OFF; break;
				case '1' : out->system_state = STATE_LOW_PWR; break;
				case '2' : out->system_state = STATE_FAULT; break;
				case '3' : out->system_state = STATE_BULK; break;
				case '4' : out->system_state = STATE_ABSORPTION; break;
				case '5' : out->system_state = STATE_FLOAT; break;
				default: out->system_state = STATE_UNINITIALIZED; break;
				}
				break;
			}
			case VE_ERR:
				if		(strcmp(pointer_val, "0") == 0) 	out->error_reason = ERR_OK;
				else if (strcmp(pointer_val, "2") == 0)		out->error_reason = ERR_EXCESIVE_BAT_VOLTAGE;
				else if (strcmp(pointer_val, "17") == 0) 	out->error_reason = ERR_CHGR_TEMP_TOO_HIGH;
				else if (strcmp(pointer_val, "18") == 0)	out->error_reason = ERR_CHGR_EXCESIVE_CURR;
				else if (strcmp(pointer_val, "19") == 0)	out->error_reason = ERR_CHGR_CURR_REVERSED;
				else if (strcmp(pointer_val, "20") == 0)	out->error_reason = ERR_BULK_TIME_EXCEED;
				else if (strcmp(pointer_val, "21") == 0)	out->error_reason = ERR_CURRENT_SENSE_FAIL;
				else if (strcmp(pointer_val, "26") == 0)	out->error_reason = ERR_EXCESIVE_TERMINAL_TEMP;
				else if (strcmp(pointer_val, "33") == 0)	out->error_reason = ERR_EXCESIVE_PV_VOLTAGE;
				else if (strcmp(pointer_val, "34") == 0)	out->error_reason = ERR_EXCESIVE_PV_CURRENT;
				else if (strcmp(pointer_val, "38") == 0)	out->error_reason = ERR_INPUT_SHUTDOWN;
				else if (strcmp(pointer_val, "116") == 0)	out->error_reason = ERR_TUNES_LOST;
				else out->error_reason = ERR_UNINITIALIZED;
				break;
			case VE_FW:
				strcpy(out->fw_version, pointer_val);
				break;
			case VE_H19:
				out->energy_gathered_total = strtol(pointer_val, NULL, 10);
				break;
			case VE_H20:
				out->energy_gathered_today = strtol(pointer_val, NULL, 10);
				break;
			case VE_H21:
				out->maximum_power_today = strtol(pointer_val, NULL, 10);
				break;
			case VE_H22:
				out->energy_gathered_yesterday = strtol(pointer_val, NULL, 10);
				break;
			case VE_H23:
				out->maximum_power_yesterday = strtol(pointer_val, NULL, 10);
				break;
			case VE_HSDS:
				out->day_seq_number = strtol(pointer_val, NULL, 10);
				break;
			case VE_I:
				out->battery_current = strtol(pointer_val, NULL, 10);
				break;
			case VE_IL:
				out->load_current = strtol(pointer_val, NULL, 10);
				break;
			case VE_LOAD:
				if (strcmp(pointer_val, "ON") == 0) out->is_load_on = 1;
				else out->is_load_on = 0;
				break;
			case VE_PID:
				out->pid = strtol(pointer_val, NULL, 16);
				break;
			case VE_PPV:
				out->pv_power = strtol(pointer_val, NULL, 10);
				break;
			case VE_SERIAL:
				break;
			case VE_V:
				out->battery_voltage = strtol(pointer_val, NULL, 10);
				break;
			case VE_VPV:
				out->pv_voltage = strtol(pointer_val, NULL, 10);
				break;
			default:
				break;
		}

	}

	return 0;

}

void ve_direct_add_to_average(ve_direct_raw_struct* in, ve_direct_average_struct* avg_struct) {

	if (skip_counter++ < VE_DIRECT_MESSAGES_TO_SKIP) {
		return;
	}
	else {
		skip_counter = 0;
	}

	uint16_t it = avg_struct->current_pointer;

	avg_struct->battery_current[it] = in->battery_current;
	avg_struct->battery_voltage[it] = in->battery_voltage;
	avg_struct->load_current[it] = in->load_current;
	avg_struct->pv_voltage[it] = in->pv_voltage;

	it++;

	if (it > VE_DIRECT_AVERAGE_LEN - 1) {
		it = 0;
		avg_struct->full_buffer = 1;
	}
	else {
		;
	}

	avg_struct->current_pointer = it;

	if (in->battery_current < avg_struct->min_battery_current) {
		avg_struct->min_battery_current = in->battery_current;
	}

	if (in->battery_current > avg_struct->max_battery_current) {
		avg_struct->max_battery_current = in->battery_current;
	}

	return;

}

void ve_direct_get_averages(ve_direct_average_struct* avg_struct, int16_t* battery_current, uint16_t* battery_voltage,
		uint16_t* pv_voltage, uint16_t* load_current) {

	if (avg_struct->full_buffer != 1)
		return;

	int32_t battery_current_avg = 0;
	uint32_t battery_voltage_avg = 0;
	uint32_t pv_voltage_avg = 0;
	uint32_t load_current_avg = 0;

	for (int i = 0; i < VE_DIRECT_AVERAGE_LEN; i++) {
		battery_current_avg += avg_struct->battery_current[i];
		battery_voltage_avg += avg_struct->battery_voltage[i];
		pv_voltage_avg += avg_struct->pv_voltage[i];
		load_current_avg += avg_struct->load_current[i];
	}

	*battery_current = battery_current_avg / VE_DIRECT_AVERAGE_LEN;
	*battery_voltage = battery_voltage_avg / VE_DIRECT_AVERAGE_LEN;
	*pv_voltage = pv_voltage_avg / VE_DIRECT_AVERAGE_LEN;
	*load_current = load_current_avg / VE_DIRECT_AVERAGE_LEN;
}

void ve_direct_set_sys_voltage(ve_direct_raw_struct* in, uint8_t* sys_voltage) {

}

void ve_direct_store_errors(ve_direct_raw_struct* input, ve_direct_error_reason* err_reason) {
	if (input->error_reason != ERR_OK) {
		*err_reason = input->error_reason;
	}
	else {
		if (*err_reason == ERR_UNINITIALIZED || *err_reason == ERR_OK) {
			*err_reason = ERR_OK;
		}
	}

}

void ve_direct_error_to_string(ve_direct_error_reason input, char* output, int8_t output_ln) {
	memset(output, 0x00, output_ln);

	switch(input) {
		case ERR_UNINITIALIZED: snprintf(output, output_ln, "ERR_UNINITIALIZED"); break;
		case ERR_OK: snprintf(output, output_ln, "ERR_OK"); break;
		case ERR_EXCESIVE_BAT_VOLTAGE: snprintf(output, output_ln, "ERR_EXCESIVE_BAT_VOLTAGE"); break;
		case ERR_CHGR_TEMP_TOO_HIGH: snprintf(output, output_ln, "ERR_CHGR_TEMP_TOO_HIGH"); break;
		case ERR_CHGR_EXCESIVE_CURR: snprintf(output, output_ln, "ERR_CHGR_EXCESIVE_CURR"); break;
		case ERR_CHGR_CURR_REVERSED: snprintf(output, output_ln, "ERR_CHGR_CURR_REVERSED"); break;
		case ERR_BULK_TIME_EXCEED: snprintf(output, output_ln, "ERR_BULK_TIME_EXCEED"); break;
		case ERR_CURRENT_SENSE_FAIL: snprintf(output, output_ln, "ERR_CURRENT_SENSE_FAIL"); break;
		case ERR_EXCESIVE_TERMINAL_TEMP:  snprintf(output, output_ln, "ERR_EXCESIVE_TERMINAL_TEMP"); break;
		case ERR_EXCESIVE_PV_VOLTAGE: snprintf(output, output_ln, "ERR_EXCESIVE_PV_VOLTAGE"); break;
		case ERR_EXCESIVE_PV_CURRENT: snprintf(output, output_ln, "ERR_EXCESIVE_PV_CURRENT"); break;
		case ERR_INPUT_SHUTDOWN: snprintf(output, output_ln, "ERR_INPUT_SHUTDOWN"); break;
		case ERR_TUNES_LOST: snprintf(output, output_ln, "ERR_TUNES_LOST"); break;
	}
}

void ve_direct_state_to_string(ve_direct_system_state input, char* output, int8_t output_ln) {
	memset(output, 0x00, output_ln);

	switch(input) {
		case STATE_UNINITIALIZED: snprintf(output, output_ln, "STATE_UNINITIALIZED"); break;
		case STATE_OFF: snprintf(output, output_ln, "STATE_OFF"); break;
		case STATE_LOW_PWR: snprintf(output, output_ln, "STATE_LOW_PWR"); break;
		case STATE_FAULT: snprintf(output, output_ln, "STATE_FAULT"); break;
		case STATE_BULK: snprintf(output, output_ln, "STATE_BULK"); break;
		case STATE_ABSORPTION: snprintf(output, output_ln, "STATE_ABSORPTION"); break;
		case STATE_FLOAT: snprintf(output, output_ln, "STATE_FLOAT"); break;
		case STATE_INVERTING:  snprintf(output, output_ln, "STATE_INVERTING"); break;
	}
}


/**
 *
 *
 *
 *"\r\nPID\t0xA042\r\nFW\t127\r\nSER#\tHQ1723BQTPC\nV\t13780\nI\t190\r\nVPV\t17770\r\nPPV\t3\r\nCS\t5\r\nERR\t0\r\nLOAD\tON\nIL\t0\nH19\t593\r\nH20\t1\r\nH21\t12\r\nH22\t7\r\nH23\t93\r\nHSDS\t167\nChecksum\t8"
 *
 *
 *\r\nPID\t0xA042
 *\r\nFW\t127
 *\r\nSER#\tHQ1723BQTPC
 *\nV\t13780
 *\nI\t190
 *\r\nVPV\t17770
 *\r\nPPV\t3
 *\r\nCS\t5
 *\r\nERR\t0
 *\r\nLOAD\tON
 *\nIL\t0
 *\nH19\t593
 *\r\nH20\t1
 *\r\nH21\t12
 *\r\nH22\t7
 *\r\nH23\t93
 *\r\nHSDS\t167
 *\nChecksum\t8"
 *
 *   Z:\home\mateusz\Documents\___STM32\VE_DIRECT_3.bin (3/22/2021 7:08:36 PM)
 *   StartOffset(h): 00000000, EndOffset(h): 0000009B, Length(h): 0000009C
 *
 *unsigned char rawData[156] = {
 *	0x0D, 0x0A, 0x50, 0x49, 0x44, 0x09, 0x30, 0x78, 0x41, 0x30, 0x34, 0x32,
 *	0x0D, 0x0A, 0x46, 0x57, 0x09, 0x31, 0x32, 0x37, 0x0D, 0x0A, 0x53, 0x45,
 *	0x52, 0x23, 0x09, 0x48, 0x51, 0x31, 0x37, 0x32, 0x33, 0x42, 0x51, 0x54,
 *	0x50, 0x43, 0x0A, 0x56, 0x09, 0x31, 0x33, 0x37, 0x38, 0x30, 0x0A, 0x49,
 *	0x09, 0x31, 0x39, 0x30, 0x0D, 0x0A, 0x56, 0x50, 0x56, 0x09, 0x31, 0x37,
 *	0x37, 0x37, 0x30, 0x0D, 0x0A, 0x50, 0x50, 0x56, 0x09, 0x33, 0x0D, 0x0A,
 *	0x43, 0x53, 0x09, 0x35, 0x0D, 0x0A, 0x45, 0x52, 0x52, 0x09, 0x30, 0x0D,
 *	0x0A, 0x4C, 0x4F, 0x41, 0x44, 0x09, 0x4F, 0x4E, 0x0A, 0x49, 0x4C, 0x09,
 *	0x30, 0x0A, 0x48, 0x31, 0x39, 0x09, 0x35, 0x39, 0x33, 0x0D, 0x0A, 0x48,
 *	0x32, 0x30, 0x09, 0x31, 0x0D, 0x0A, 0x48, 0x32, 0x31, 0x09, 0x31, 0x32,
 *	0x0D, 0x0A, 0x48, 0x32, 0x32, 0x09, 0x37, 0x0D, 0x0A, 0x48, 0x32, 0x33,
 *	0x09, 0x39, 0x33, 0x0D, 0x0A, 0x48, 0x53, 0x44, 0x53, 0x09, 0x31, 0x36,
 *	0x37, 0x0A, 0x43, 0x68, 0x65, 0x63, 0x6B, 0x73, 0x75, 0x6D, 0x09, 0x38
 *};
 *
 * 0D 0A 50 49 44 09 30 78 41 30 34 32 0D 0A 46 57 09 31 32 37 0D 0A 53 45 52 23 09 48 51 31 37 32 33 42 51 54 50 43 0A 56 09 31 33 37 38 30 0A 49 09 31 39 30 0D 0A 56 50 56 09 31 37 37 37 30 0D 0A 50 50 56 09 33 0D 0A 43 53 09 35 0D 0A 45 52 52 09 30 0D 0A 4C 4F 41 44 09 4F 4E 0A 49 4C 09 30 0A 48 31 39 09 35 39 33 0D 0A 48 32 30 09 31 0D 0A 48 32 31 09 31 32 0D 0A 48 32 32 09 37 0D 0A 48 32 33 09 39 33 0D 0A 48 53 44 53 09 31 36 37 0A 43 68 65 63 6B 73 75 6D 09 38
 *
 *
 *
 *
 *
 *
 *
 *
 */
