/*
 * average_struct.h
 *
 *  Created on: 17.03.2019
 *      Author: mateusz
 */

#ifndef VE_DIRECT_PROTOCOL_AVERAGE_STRUCT_H_
#define VE_DIRECT_PROTOCOL_AVERAGE_STRUCT_H_

#define VE_DIRECT_AVERAGE_LEN	12

typedef __attribute__ ((aligned(1))) struct ve_direct_average_struct {

	int16_t battery_current[VE_DIRECT_AVERAGE_LEN];	// 16 bytes

	uint16_t battery_voltage[VE_DIRECT_AVERAGE_LEN];	// 16 bytes

	uint16_t pv_voltage[VE_DIRECT_AVERAGE_LEN];			// 16 bytes

	uint16_t load_current[VE_DIRECT_AVERAGE_LEN];			// 16 bytes

	uint16_t current_pointer;

	uint8_t full_buffer;

	int16_t min_battery_current;

	int16_t max_battery_current;

} ve_direct_average_struct;




#endif /* VE_DIRECT_PROTOCOL_AVERAGE_STRUCT_H_ */
