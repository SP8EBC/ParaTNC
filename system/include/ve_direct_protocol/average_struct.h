/*
 * average_struct.h
 *
 *  Created on: 17.03.2019
 *      Author: mateusz
 */

#ifndef VE_DIRECT_PROTOCOL_AVERAGE_STRUCT_H_
#define VE_DIRECT_PROTOCOL_AVERAGE_STRUCT_H_

typedef __attribute__ ((aligned(1))) struct ve_direct_average_struct {

	int16_t battery_current[32];	// 16 bytes

	uint16_t battery_voltage[32];	// 16 bytes

	uint16_t pv_voltage[32];			// 16 bytes

	uint16_t load_current[32];			// 16 bytes

	uint16_t current_pointer;

} ve_direct_average_struct;




#endif /* VE_DIRECT_PROTOCOL_AVERAGE_STRUCT_H_ */
