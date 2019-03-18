/*
 * raw_struct.h
 *
 *  Created on: 10.03.2019
 *      Author: mateusz
 */

#include <stdint.h>

#ifndef VE_DIRECT_PROTOCOL_RAW_STRUCT_H_
#define VE_DIRECT_PROTOCOL_RAW_STRUCT_H_

typedef enum ve_direct_system_state {
	STATE_UNINITIALIZED	= -1,
	STATE_OFF 			= 0,
	STATE_LOW_PWR 		= 1,
	STATE_FAULT			= 2,
	STATE_BULK			= 3,
	STATE_ABSORPTION	= 4,
	STATE_FLOAT			= 5,
	STATE_INVERTING		= 9
}ve_direct_system_state;

typedef enum ve_direct_error_reason {
	ERR_UNINITIALIZED			= -1,
	ERR_OK						= 0,
	ERR_EXCESIVE_BAT_VOLTAGE	= 2,
	ERR_CHGR_TEMP_TOO_HIGH		= 17,
	ERR_CHGR_EXCESIVE_CURR		= 18,
	ERR_CHGR_CURR_REVERSED		= 19,
	ERR_BULK_TIME_EXCEED		= 20,
	ERR_CURRENT_SENSE_FAIL		= 21,
	ERR_EXCESIVE_TERMINAL_TEMP	= 26,
	ERR_EXCESIVE_PV_VOLTAGE		= 33,
	ERR_EXCESIVE_PV_CURRENT		= 34,
	ERR_INPUT_SHUTDOWN			= 38,
	ERR_TUNES_LOST				= 116
}ve_direct_error_reason;

typedef enum ve_direct_key_values {
	VE_UNKNOWN,
	VE_PID,
	VE_FW,
	VE_SERIAL,
	VE_V,
	VE_I,
	VE_VPV,
	VE_PPV,
	VE_CS,
	VE_ERR,
	VE_LOAD,
	VE_IL,
	VE_H19,
	VE_H20,
	VE_H21,
	VE_H22,
	VE_H23,
	VE_HSDS,
	VE_CHECKSUM
}ve_direct_key_values;

typedef struct ve_direct_raw_struct{

	uint16_t pid;

	char fw_version[8];

	uint16_t battery_voltage;	// in milivolts

	int16_t battery_current;	// in miliamperes. Positive means charging, negative means supplying

	uint16_t pv_voltage;

	uint8_t pv_power;		// in watts

	ve_direct_system_state system_state;

	ve_direct_error_reason error_reason;

	uint8_t is_load_on;

	uint16_t load_current;

	uint16_t energy_gathered_total;	// in 0.01kWh increments

	uint16_t energy_gathered_today;	// in 0.01kWh increments

	uint16_t maximum_power_today;

	uint16_t energy_gathered_yesterday;

	uint16_t maximum_power_yesterday;

	uint16_t day_seq_number;

	uint16_t checksum;
} ve_direct_raw_struct;

#endif /* VE_DIRECT_PROTOCOL_RAW_STRUCT_H_ */
