/*
 * rtu_getters.c
 *
 *  Created on: 29.09.2020
 *      Author: mateusz
 */


#include "station_config.h"

#include "rtu_getters.h"
#include "rtu_return_values.h"

int32_t rtu_get_temperature(float* out) {

	int32_t retval = MODBUS_RET_UNINITIALIZED;

	rtu_register_data_t* source = 0;

	uint16_t raw_register_value = 0;

	uint16_t scaling_a, scaling_b, scaling_c, scaling_d;

#ifdef _RTU_SLAVE_TEMPERATURE_SOURCE
	#if (_RTU_SLAVE_TEMPERATURE_SOURCE == 1)
		source = &RTU_GETTERS_F1_NAME;
		scaling_a = _RTU_SLAVE_SCALING_A_1;
		scaling_b = _RTU_SLAVE_SCALING_B_1;
		scaling_c = _RTU_SLAVE_SCALING_C_1;
		scaling_d = _RTU_SLAVE_SCALING_D_1;
	#elif (_RTU_SLAVE_TEMPERATURE_SOURCE == 2)
		source = &RTU_GETTERS_F2_NAME;
		scaling_a = _RTU_SLAVE_SCALING_A_2;
		scaling_b = _RTU_SLAVE_SCALING_B_2;
		scaling_c = _RTU_SLAVE_SCALING_C_2;
		scaling_d = _RTU_SLAVE_SCALING_D_2;
	#elif (_RTU_SLAVE_TEMPERATURE_SOURCE == 3)
		source = &RTU_GETTERS_F3_NAME;
		scaling_a = _RTU_SLAVE_SCALING_A_3;
		scaling_b = _RTU_SLAVE_SCALING_B_3;
		scaling_c = _RTU_SLAVE_SCALING_C_3;
		scaling_d = _RTU_SLAVE_SCALING_D_3;
	#elif (_RTU_SLAVE_TEMPERATURE_SOURCE == 4)
		source = &RTU_GETTERS_F4_NAME;
		scaling_a = _RTU_SLAVE_SCALING_A_4;
		scaling_b = _RTU_SLAVE_SCALING_B_4;
		scaling_c = _RTU_SLAVE_SCALING_C_4;
		scaling_d = _RTU_SLAVE_SCALING_D_4;
	#else
		#error "Wrong Modbus Configuration"
	#endif
#else
	retval = MODBUS_RET_NOT_AVALIABLE;
#endif

	if (retval == MODBUS_RET_UNINITIALIZED && source != 0) {
		// copy the raw value from modbus register data
		raw_register_value = source->registers_values[0];

		// rescale the raw value to target value in degrees C
		*out = 	(
				scaling_a * (float)raw_register_value * (float)raw_register_value +
				scaling_b * (float)raw_register_value +
				scaling_c
				) /
				scaling_d;

		retval = MODBUS_RET_OK;
	}

	return retval;
}

int32_t rtu_get_pressure(float* out) {

	int32_t retval = MODBUS_RET_UNINITIALIZED;

	rtu_register_data_t* source = 0;

	uint16_t raw_register_value = 0;

	uint16_t scaling_a, scaling_b, scaling_c, scaling_d;

#ifdef _RTU_SLAVE_PRESSURE_SOURCE
	#if (_RTU_SLAVE_PRESSURE_SOURCE == 1)
		source = &RTU_GETTERS_F1_NAME;
		scaling_a = _RTU_SLAVE_SCALING_A_1;
		scaling_b = _RTU_SLAVE_SCALING_B_1;
		scaling_c = _RTU_SLAVE_SCALING_C_1;
		scaling_d = _RTU_SLAVE_SCALING_D_1;
	#elif (_RTU_SLAVE_PRESSURE_SOURCE == 2)
		source = &RTU_GETTERS_F2_NAME;
		scaling_a = _RTU_SLAVE_SCALING_A_2;
		scaling_b = _RTU_SLAVE_SCALING_B_2;
		scaling_c = _RTU_SLAVE_SCALING_C_2;
		scaling_d = _RTU_SLAVE_SCALING_D_2;
	#elif (_RTU_SLAVE_PRESSURE_SOURCE == 3)
		source = &RTU_GETTERS_F3_NAME;
		scaling_a = _RTU_SLAVE_SCALING_A_3;
		scaling_b = _RTU_SLAVE_SCALING_B_3;
		scaling_c = _RTU_SLAVE_SCALING_C_3;
		scaling_d = _RTU_SLAVE_SCALING_D_3;
	#elif (_RTU_SLAVE_PRESSURE_SOURCE == 4)
		source = &RTU_GETTERS_F4_NAME;
		scaling_a = _RTU_SLAVE_SCALING_A_4;
		scaling_b = _RTU_SLAVE_SCALING_B_4;
		scaling_c = _RTU_SLAVE_SCALING_C_4;
		scaling_d = _RTU_SLAVE_SCALING_D_4;
	#else
		#error "Wrong Modbus Configuration"
	#endif
#else
	retval = MODBUS_RET_NOT_AVALIABLE;
#endif

	if (retval == MODBUS_RET_UNINITIALIZED && source != 0) {
		// copy the raw value from modbus register data
		raw_register_value = source->registers_values[0];

		// rescale the raw value to target value in degrees C
		*out = 	(
				scaling_a * (float)raw_register_value * (float)raw_register_value +
				scaling_b * (float)raw_register_value +
				scaling_c
				) /
				scaling_d;

		retval = MODBUS_RET_OK;
	}

	return retval;
}

int32_t rtu_get_wind_direction(uint16_t* out) {

	int32_t retval = MODBUS_RET_UNINITIALIZED;

	rtu_register_data_t* source = 0;

	uint16_t raw_register_value = 0;

	uint16_t scaling_a, scaling_b, scaling_c, scaling_d;

#ifdef _RTU_SLAVE_WIND_DIRECTION_SORUCE
	#if (_RTU_SLAVE_WIND_DIRECTION_SORUCE == 1)
		source = &RTU_GETTERS_F1_NAME;
		scaling_a = _RTU_SLAVE_SCALING_A_1;
		scaling_b = _RTU_SLAVE_SCALING_B_1;
		scaling_c = _RTU_SLAVE_SCALING_C_1;
		scaling_d = _RTU_SLAVE_SCALING_D_1;
	#elif (_RTU_SLAVE_WIND_DIRECTION_SORUCE == 2)
		source = &RTU_GETTERS_F2_NAME;
		scaling_a = _RTU_SLAVE_SCALING_A_2;
		scaling_b = _RTU_SLAVE_SCALING_B_2;
		scaling_c = _RTU_SLAVE_SCALING_C_2;
		scaling_d = _RTU_SLAVE_SCALING_D_2;
	#elif (_RTU_SLAVE_WIND_DIRECTION_SORUCE == 3)
		source = &RTU_GETTERS_F3_NAME;
		scaling_a = _RTU_SLAVE_SCALING_A_3;
		scaling_b = _RTU_SLAVE_SCALING_B_3;
		scaling_c = _RTU_SLAVE_SCALING_C_3;
		scaling_d = _RTU_SLAVE_SCALING_D_3;
	#elif (_RTU_SLAVE_WIND_DIRECTION_SORUCE == 4)
		source = &RTU_GETTERS_F4_NAME;
		scaling_a = _RTU_SLAVE_SCALING_A_4;
		scaling_b = _RTU_SLAVE_SCALING_B_4;
		scaling_c = _RTU_SLAVE_SCALING_C_4;
		scaling_d = _RTU_SLAVE_SCALING_D_4;
	#else
		#error "Wrong Modbus Configuration"
	#endif
#else
	retval = MODBUS_RET_NOT_AVALIABLE;
#endif

	if (retval == MODBUS_RET_UNINITIALIZED && source != 0) {
		// copy the raw value from modbus register data
		raw_register_value = source->registers_values[0];

		// rescale the raw value to target value in degrees C
		*out = 	(
				scaling_a * (float)raw_register_value * (float)raw_register_value +
				scaling_b * (float)raw_register_value +
				scaling_c
				) /
				scaling_d;

		retval = MODBUS_RET_OK;
	}

	return retval;
}

int32_t rtu_get_wind_speed(uint16_t* out) {

	int32_t retval = MODBUS_RET_UNINITIALIZED;

	rtu_register_data_t* source = 0;

	uint16_t raw_register_value = 0;

	uint16_t scaling_a, scaling_b, scaling_c, scaling_d;

#ifdef _RTU_SLAVE_WIND_SPEED_SOURCE
	#if (_RTU_SLAVE_WIND_SPEED_SOURCE == 1)
		source = &RTU_GETTERS_F1_NAME;
		scaling_a = _RTU_SLAVE_SCALING_A_1;
		scaling_b = _RTU_SLAVE_SCALING_B_1;
		scaling_c = _RTU_SLAVE_SCALING_C_1;
		scaling_d = _RTU_SLAVE_SCALING_D_1;
	#elif (_RTU_SLAVE_WIND_SPEED_SOURCE == 2)
		source = &RTU_GETTERS_F2_NAME;
		scaling_a = _RTU_SLAVE_SCALING_A_2;
		scaling_b = _RTU_SLAVE_SCALING_B_2;
		scaling_c = _RTU_SLAVE_SCALING_C_2;
		scaling_d = _RTU_SLAVE_SCALING_D_2;
	#elif (_RTU_SLAVE_WIND_SPEED_SOURCE == 3)
		source = &RTU_GETTERS_F3_NAME;
		scaling_a = _RTU_SLAVE_SCALING_A_3;
		scaling_b = _RTU_SLAVE_SCALING_B_3;
		scaling_c = _RTU_SLAVE_SCALING_C_3;
		scaling_d = _RTU_SLAVE_SCALING_D_3;
	#elif (_RTU_SLAVE_WIND_SPEED_SOURCE == 4)
		source = &RTU_GETTERS_F4_NAME;
		scaling_a = _RTU_SLAVE_SCALING_A_4;
		scaling_b = _RTU_SLAVE_SCALING_B_4;
		scaling_c = _RTU_SLAVE_SCALING_C_4;
		scaling_d = _RTU_SLAVE_SCALING_D_4;
	#else
		#error "Wrong Modbus Configuration"
	#endif
#else
	retval = MODBUS_RET_NOT_AVALIABLE;
#endif

	if (retval == MODBUS_RET_UNINITIALIZED && source != 0) {
		// copy the raw value from modbus register data
		raw_register_value = source->registers_values[0];

		// rescale the raw value to target value in .1m/s incremenets
		*out = 	10 * (
				scaling_a * (uint16_t)raw_register_value * (uint16_t)raw_register_value +
				scaling_b * (uint16_t)raw_register_value +
				scaling_c
				) /
				scaling_d;

		retval = MODBUS_RET_OK;
	}

	return retval;
}

int32_t rtu_get_humidity(int8_t* out) {

	int32_t retval = MODBUS_RET_UNINITIALIZED;

	rtu_register_data_t* source = 0;

	uint16_t raw_register_value = 0;

	uint16_t scaling_a, scaling_b, scaling_c, scaling_d;

#ifdef _RTU_SLAVE_HUMIDITY_SOURCE
	#if (_RTU_SLAVE_HUMIDITY_SOURCE == 1)
		source = &RTU_GETTERS_F1_NAME;
		scaling_a = _RTU_SLAVE_SCALING_A_1;
		scaling_b = _RTU_SLAVE_SCALING_B_1;
		scaling_c = _RTU_SLAVE_SCALING_C_1;
		scaling_d = _RTU_SLAVE_SCALING_D_1;
	#elif (_RTU_SLAVE_HUMIDITY_SOURCE == 2)
		source = &RTU_GETTERS_F2_NAME;
		scaling_a = _RTU_SLAVE_SCALING_A_2;
		scaling_b = _RTU_SLAVE_SCALING_B_2;
		scaling_c = _RTU_SLAVE_SCALING_C_2;
		scaling_d = _RTU_SLAVE_SCALING_D_2;
	#elif (_RTU_SLAVE_HUMIDITY_SOURCE == 3)
		source = &RTU_GETTERS_F3_NAME;
		scaling_a = _RTU_SLAVE_SCALING_A_3;
		scaling_b = _RTU_SLAVE_SCALING_B_3;
		scaling_c = _RTU_SLAVE_SCALING_C_3;
		scaling_d = _RTU_SLAVE_SCALING_D_3;
	#elif (_RTU_SLAVE_HUMIDITY_SOURCE == 4)
		source = &RTU_GETTERS_F4_NAME;
		scaling_a = _RTU_SLAVE_SCALING_A_4;
		scaling_b = _RTU_SLAVE_SCALING_B_4;
		scaling_c = _RTU_SLAVE_SCALING_C_4;
		scaling_d = _RTU_SLAVE_SCALING_D_4;
	#else
		#error "Wrong Modbus Configuration"
	#endif
#else
	retval = MODBUS_RET_NOT_AVALIABLE;
#endif

	if (retval == MODBUS_RET_UNINITIALIZED && source != 0) {
		// copy the raw value from modbus register data
		raw_register_value = source->registers_values[0];

		// rescale the raw value to target value in percents
		*out = 	10 * (
				scaling_a * (uint16_t)raw_register_value * (uint16_t)raw_register_value +
				scaling_b * (uint16_t)raw_register_value +
				scaling_c
				) /
				scaling_d;

		retval = MODBUS_RET_OK;
	}

	return retval;
}

