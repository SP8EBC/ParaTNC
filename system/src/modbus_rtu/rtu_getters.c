/*
 * rtu_getters.c
 *
 *  Created on: 29.09.2020
 *      Author: mateusz
 */

#include <stdio.h>

#include "station_config.h"

#include "modbus_rtu/rtu_getters.h"
#include "modbus_rtu/rtu_return_values.h"
#include "modbus_rtu/rtu_register_data_t.h"

#include "modbus_rtu/rtu_configuration.h"

#include "rte_wx.h"
#include "rte_rtu.h"
#include "main.h"

int32_t rtu_get_temperature(float* out) {

	int32_t retval = MODBUS_RET_UNINITIALIZED;

	rtu_register_data_t* source = 0;

	uint16_t raw_register_value = 0;

	uint16_t scaling_a, scaling_b, scaling_c, scaling_d;

	// the timestamp of last update of the register value
	uint32_t last_update_timestam = 0;

#ifdef _MODBUS_RTU
#ifdef _RTU_SLAVE_TEMPERATURE_SOURCE
	#if (_RTU_SLAVE_TEMPERATURE_SOURCE == 1)
		source = &RTU_GETTERS_F1_NAME;
		scaling_a = _RTU_SLAVE_SCALING_A_1;
		scaling_b = _RTU_SLAVE_SCALING_B_1;
		scaling_c = _RTU_SLAVE_SCALING_C_1;
		scaling_d = _RTU_SLAVE_SCALING_D_1;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[0];
	#elif (_RTU_SLAVE_TEMPERATURE_SOURCE == 2)
		source = &RTU_GETTERS_F2_NAME;
		scaling_a = _RTU_SLAVE_SCALING_A_2;
		scaling_b = _RTU_SLAVE_SCALING_B_2;
		scaling_c = _RTU_SLAVE_SCALING_C_2;
		scaling_d = _RTU_SLAVE_SCALING_D_2;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[1];
	#elif (_RTU_SLAVE_TEMPERATURE_SOURCE == 3)
		source = &RTU_GETTERS_F3_NAME;
		scaling_a = _RTU_SLAVE_SCALING_A_3;
		scaling_b = _RTU_SLAVE_SCALING_B_3;
		scaling_c = _RTU_SLAVE_SCALING_C_3;
		scaling_d = _RTU_SLAVE_SCALING_D_3;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[2];
	#elif (_RTU_SLAVE_TEMPERATURE_SOURCE == 4)
		source = &RTU_GETTERS_F4_NAME;
		scaling_a = _RTU_SLAVE_SCALING_A_4;
		scaling_b = _RTU_SLAVE_SCALING_B_4;
		scaling_c = _RTU_SLAVE_SCALING_C_4;
		scaling_d = _RTU_SLAVE_SCALING_D_4;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[3];
	#elif (_RTU_SLAVE_TEMPERATURE_SOURCE == 5)
		source = &RTU_GETTERS_F5_NAME;
		scaling_a = _RTU_SLAVE_SCALING_A_5;
		scaling_b = _RTU_SLAVE_SCALING_B_5;
		scaling_c = _RTU_SLAVE_SCALING_C_5;
		scaling_d = _RTU_SLAVE_SCALING_D_5;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[4];
	#elif (_RTU_SLAVE_TEMPERATURE_SOURCE == 6)
		source = &RTU_GETTERS_F6_NAME;
		scaling_a = _RTU_SLAVE_SCALING_A_6;
		scaling_b = _RTU_SLAVE_SCALING_B_6;
		scaling_c = _RTU_SLAVE_SCALING_C_6;
		scaling_d = _RTU_SLAVE_SCALING_D_6;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[5];
	#else
		#error "Wrong Modbus Configuration"
	#endif
#else
	retval = MODBUS_RET_NOT_CONFIGURED;
#endif

	if (retval == MODBUS_RET_UNINITIALIZED && source != 0) {
		// copy the raw value from modbus register data
		raw_register_value = source->registers_values[0];

		// check when the value has been updated
		if (main_get_master_time() - last_update_timestam > RTU_MAXIMUM_VALUE_AGE) {
			retval = MODBUS_RET_NOT_AVALIABLE;
		}
		else {
			if (main_get_master_time() - rte_rtu_last_modbus_rx_error_timestamp < RTU_MAXIMUM_VALUE_AGE) {
				retval = MODBUS_RET_DEGRADED;
			}
			else {
				retval = MODBUS_RET_OK;
			}

			// rescale the raw value to target value in degrees C
			*out = 	(
					scaling_a * (float)raw_register_value * (float)raw_register_value +
					scaling_b * (float)raw_register_value +
					scaling_c
					) /
					scaling_d;
		}

	}
#endif

	return retval;
}

int32_t rtu_get_pressure(float* out) {

	int32_t retval = MODBUS_RET_UNINITIALIZED;

	rtu_register_data_t* source = 0;

	uint16_t raw_register_value = 0;

	uint16_t scaling_a, scaling_b, scaling_c, scaling_d;

	// the timestamp of last update of the register value
	uint32_t last_update_timestam = 0;

#ifdef _MODBUS_RTU
#ifdef _RTU_SLAVE_PRESSURE_SOURCE
	#if (_RTU_SLAVE_PRESSURE_SOURCE == 1)
		source = &RTU_GETTERS_F1_NAME;
		scaling_a = _RTU_SLAVE_SCALING_A_1;
		scaling_b = _RTU_SLAVE_SCALING_B_1;
		scaling_c = _RTU_SLAVE_SCALING_C_1;
		scaling_d = _RTU_SLAVE_SCALING_D_1;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[0];
	#elif (_RTU_SLAVE_PRESSURE_SOURCE == 2)
		source = &RTU_GETTERS_F2_NAME;
		scaling_a = _RTU_SLAVE_SCALING_A_2;
		scaling_b = _RTU_SLAVE_SCALING_B_2;
		scaling_c = _RTU_SLAVE_SCALING_C_2;
		scaling_d = _RTU_SLAVE_SCALING_D_2;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[1];
	#elif (_RTU_SLAVE_PRESSURE_SOURCE == 3)
		source = &RTU_GETTERS_F3_NAME;
		scaling_a = _RTU_SLAVE_SCALING_A_3;
		scaling_b = _RTU_SLAVE_SCALING_B_3;
		scaling_c = _RTU_SLAVE_SCALING_C_3;
		scaling_d = _RTU_SLAVE_SCALING_D_3;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[2];
	#elif (_RTU_SLAVE_PRESSURE_SOURCE == 4)
		source = &RTU_GETTERS_F4_NAME;
		scaling_a = _RTU_SLAVE_SCALING_A_4;
		scaling_b = _RTU_SLAVE_SCALING_B_4;
		scaling_c = _RTU_SLAVE_SCALING_C_4;
		scaling_d = _RTU_SLAVE_SCALING_D_4;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[3];
	#elif (_RTU_SLAVE_PRESSURE_SOURCE == 5)
		source = &RTU_GETTERS_F5_NAME;
		scaling_a = _RTU_SLAVE_SCALING_A_5;
		scaling_b = _RTU_SLAVE_SCALING_B_5;
		scaling_c = _RTU_SLAVE_SCALING_C_5;
		scaling_d = _RTU_SLAVE_SCALING_D_5;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[4];
	#elif (_RTU_SLAVE_PRESSURE_SOURCE == 6)
		source = &RTU_GETTERS_F6_NAME;
		scaling_a = _RTU_SLAVE_SCALING_A_6;
		scaling_b = _RTU_SLAVE_SCALING_B_6;
		scaling_c = _RTU_SLAVE_SCALING_C_6;
		scaling_d = _RTU_SLAVE_SCALING_D_6;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[5];
	#else
		#error "Wrong Modbus Configuration"
	#endif
#else
	retval = MODBUS_RET_NOT_CONFIGURED;
#endif

	if (retval == MODBUS_RET_UNINITIALIZED && source != 0) {
		// copy the raw value from modbus register data
		raw_register_value = source->registers_values[0];

		// check when the value has been updated
		if (main_get_master_time() - last_update_timestam > RTU_MAXIMUM_VALUE_AGE) {
			retval = MODBUS_RET_NOT_AVALIABLE;
		}
		else {
			if (main_get_master_time() - rte_rtu_last_modbus_rx_error_timestamp < RTU_MAXIMUM_VALUE_AGE) {
				retval = MODBUS_RET_DEGRADED;
			}
			else {
				retval = MODBUS_RET_OK;
			}

			// rescale the raw value to target value in degrees C
			*out = 	(
					scaling_a * (float)raw_register_value * (float)raw_register_value +
					scaling_b * (float)raw_register_value +
					scaling_c
					) /
					scaling_d;
		}
	}
#endif

	return retval;
}

int32_t rtu_get_wind_direction(uint16_t* out) {

	int32_t retval = MODBUS_RET_UNINITIALIZED;

	rtu_register_data_t* source = 0;

	uint16_t raw_register_value = 0;

	uint16_t scaling_a, scaling_b, scaling_c, scaling_d;

	// the timestamp of last update of the register value
	uint32_t last_update_timestam = 0;

#ifdef _MODBUS_RTU
#ifdef _RTU_SLAVE_WIND_DIRECTION_SORUCE
	#if (_RTU_SLAVE_WIND_DIRECTION_SORUCE == 1)
		source = &RTU_GETTERS_F1_NAME;
		scaling_a = _RTU_SLAVE_SCALING_A_1;
		scaling_b = _RTU_SLAVE_SCALING_B_1;
		scaling_c = _RTU_SLAVE_SCALING_C_1;
		scaling_d = _RTU_SLAVE_SCALING_D_1;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[0];
	#elif (_RTU_SLAVE_WIND_DIRECTION_SORUCE == 2)
		source = &RTU_GETTERS_F2_NAME;
		scaling_a = _RTU_SLAVE_SCALING_A_2;
		scaling_b = _RTU_SLAVE_SCALING_B_2;
		scaling_c = _RTU_SLAVE_SCALING_C_2;
		scaling_d = _RTU_SLAVE_SCALING_D_2;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[1];
	#elif (_RTU_SLAVE_WIND_DIRECTION_SORUCE == 3)
		source = &RTU_GETTERS_F3_NAME;
		scaling_a = _RTU_SLAVE_SCALING_A_3;
		scaling_b = _RTU_SLAVE_SCALING_B_3;
		scaling_c = _RTU_SLAVE_SCALING_C_3;
		scaling_d = _RTU_SLAVE_SCALING_D_3;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[2];
	#elif (_RTU_SLAVE_WIND_DIRECTION_SORUCE == 4)
		source = &RTU_GETTERS_F4_NAME;
		scaling_a = _RTU_SLAVE_SCALING_A_4;
		scaling_b = _RTU_SLAVE_SCALING_B_4;
		scaling_c = _RTU_SLAVE_SCALING_C_4;
		scaling_d = _RTU_SLAVE_SCALING_D_4;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[3];
	#elif (_RTU_SLAVE_WIND_DIRECTION_SORUCE == 5)
		source = &RTU_GETTERS_F5_NAME;
		scaling_a = _RTU_SLAVE_SCALING_A_5;
		scaling_b = _RTU_SLAVE_SCALING_B_5;
		scaling_c = _RTU_SLAVE_SCALING_C_5;
		scaling_d = _RTU_SLAVE_SCALING_D_5;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[4];
	#elif (_RTU_SLAVE_WIND_DIRECTION_SORUCE == 6)
		source = &RTU_GETTERS_F6_NAME;
		scaling_a = _RTU_SLAVE_SCALING_A_6;
		scaling_b = _RTU_SLAVE_SCALING_B_6;
		scaling_c = _RTU_SLAVE_SCALING_C_6;
		scaling_d = _RTU_SLAVE_SCALING_D_6;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[5];
	#else
		#error "Wrong Modbus Configuration"
	#endif
#else
	retval = MODBUS_RET_NOT_CONFIGURED;
#endif

	if (retval == MODBUS_RET_UNINITIALIZED && source != 0) {
		// copy the raw value from modbus register data
		raw_register_value = source->registers_values[0];

		// check when the value has been updated
		if (main_get_master_time() - last_update_timestam > RTU_MAXIMUM_VALUE_AGE) {
			retval = MODBUS_RET_NOT_AVALIABLE;
		}
		else {
			if (main_get_master_time() - rte_rtu_last_modbus_rx_error_timestamp < RTU_MAXIMUM_VALUE_AGE) {
				retval = MODBUS_RET_DEGRADED;
			}
			else {
				retval = MODBUS_RET_OK;
			}

			// rescale the raw value to target value in degrees C
			*out = 	(uint16_t)(
								scaling_a * (uint16_t)raw_register_value * (uint16_t)raw_register_value +
								scaling_b * (uint16_t)raw_register_value +
								scaling_c
								) /
								scaling_d;
		}
	}
#endif

	return retval;
}

int32_t rtu_get_wind_speed(uint16_t* out) {

	int32_t retval = MODBUS_RET_UNINITIALIZED;

	rtu_register_data_t* source = 0;

	uint16_t raw_register_value = 0;

	uint16_t scaling_a, scaling_b, scaling_c, scaling_d;

	// the timestamp of last update of the register value
	uint32_t last_update_timestam = 0;

#ifdef _MODBUS_RTU
#ifdef _RTU_SLAVE_WIND_SPEED_SOURCE
	#if (_RTU_SLAVE_WIND_SPEED_SOURCE == 1)
		source = &RTU_GETTERS_F1_NAME;
		scaling_a = _RTU_SLAVE_SCALING_A_1;
		scaling_b = _RTU_SLAVE_SCALING_B_1;
		scaling_c = _RTU_SLAVE_SCALING_C_1;
		scaling_d = _RTU_SLAVE_SCALING_D_1;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[0];
	#elif (_RTU_SLAVE_WIND_SPEED_SOURCE == 2)
		source = &RTU_GETTERS_F2_NAME;
		scaling_a = _RTU_SLAVE_SCALING_A_2;
		scaling_b = _RTU_SLAVE_SCALING_B_2;
		scaling_c = _RTU_SLAVE_SCALING_C_2;
		scaling_d = _RTU_SLAVE_SCALING_D_2;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[1];
	#elif (_RTU_SLAVE_WIND_SPEED_SOURCE == 3)
		source = &RTU_GETTERS_F3_NAME;
		scaling_a = _RTU_SLAVE_SCALING_A_3;
		scaling_b = _RTU_SLAVE_SCALING_B_3;
		scaling_c = _RTU_SLAVE_SCALING_C_3;
		scaling_d = _RTU_SLAVE_SCALING_D_3;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[2];
	#elif (_RTU_SLAVE_WIND_SPEED_SOURCE == 4)
		source = &RTU_GETTERS_F4_NAME;
		scaling_a = _RTU_SLAVE_SCALING_A_4;
		scaling_b = _RTU_SLAVE_SCALING_B_4;
		scaling_c = _RTU_SLAVE_SCALING_C_4;
		scaling_d = _RTU_SLAVE_SCALING_D_4;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[3];
	#elif (_RTU_SLAVE_WIND_SPEED_SOURCE == 5)
		source = &RTU_GETTERS_F5_NAME;
		scaling_a = _RTU_SLAVE_SCALING_A_5;
		scaling_b = _RTU_SLAVE_SCALING_B_5;
		scaling_c = _RTU_SLAVE_SCALING_C_5;
		scaling_d = _RTU_SLAVE_SCALING_D_5;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[4];
	#elif (_RTU_SLAVE_WIND_SPEED_SOURCE == 6)
		source = &RTU_GETTERS_F6_NAME;
		scaling_a = _RTU_SLAVE_SCALING_A_6;
		scaling_b = _RTU_SLAVE_SCALING_B_6;
		scaling_c = _RTU_SLAVE_SCALING_C_6;
		scaling_d = _RTU_SLAVE_SCALING_D_6;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[5];
	#else
		#error "Wrong Modbus Configuration"
	#endif
#else
	retval = MODBUS_RET_NOT_CONFIGURED;
#endif

	if (retval == MODBUS_RET_UNINITIALIZED && source != 0) {
		// copy the raw value from modbus register data
		raw_register_value = source->registers_values[0];

		// check when the value has been updated
		if (main_get_master_time() - last_update_timestam > RTU_MAXIMUM_VALUE_AGE) {
			retval = MODBUS_RET_NOT_AVALIABLE;
		}
		else {
			if (main_get_master_time() - rte_rtu_last_modbus_rx_error_timestamp < RTU_MAXIMUM_VALUE_AGE) {
				retval = MODBUS_RET_DEGRADED;
			}
			else {
				retval = MODBUS_RET_OK;
			}

			// rescale the raw value to target value in .1m/s incremenets
			*out = 	(uint16_t) (10 * (
								scaling_a * (uint16_t)raw_register_value * (uint16_t)raw_register_value +
								scaling_b * (uint16_t)raw_register_value +
								scaling_c
								) /
								scaling_d);
		}
	}
#endif

	return retval;
}

int32_t rtu_get_wind_gusts(uint16_t* out) {
	int32_t retval = MODBUS_RET_UNINITIALIZED;

	rtu_register_data_t* source = 0;

	uint16_t raw_register_value = 0;

	uint16_t scaling_a, scaling_b, scaling_c, scaling_d;

	// the timestamp of last update of the register value
	uint32_t last_update_timestam = 0;

#ifdef _MODBUS_RTU
#ifdef _RTU_SLAVE_WIND_GUSTS_SOURCE
	#if (_RTU_SLAVE_WIND_GUSTS_SOURCE == 1)
		source = &RTU_GETTERS_F1_NAME;
		scaling_a = _RTU_SLAVE_SCALING_A_1;
		scaling_b = _RTU_SLAVE_SCALING_B_1;
		scaling_c = _RTU_SLAVE_SCALING_C_1;
		scaling_d = _RTU_SLAVE_SCALING_D_1;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[0];
	#elif (_RTU_SLAVE_WIND_GUSTS_SOURCE == 2)
		source = &RTU_GETTERS_F2_NAME;
		scaling_a = _RTU_SLAVE_SCALING_A_2;
		scaling_b = _RTU_SLAVE_SCALING_B_2;
		scaling_c = _RTU_SLAVE_SCALING_C_2;
		scaling_d = _RTU_SLAVE_SCALING_D_2;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[1];
	#elif (_RTU_SLAVE_WIND_GUSTS_SOURCE == 3)
		source = &RTU_GETTERS_F3_NAME;
		scaling_a = _RTU_SLAVE_SCALING_A_3;
		scaling_b = _RTU_SLAVE_SCALING_B_3;
		scaling_c = _RTU_SLAVE_SCALING_C_3;
		scaling_d = _RTU_SLAVE_SCALING_D_3;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[2];
	#elif (_RTU_SLAVE_WIND_GUSTS_SOURCE == 4)
		source = &RTU_GETTERS_F4_NAME;
		scaling_a = _RTU_SLAVE_SCALING_A_4;
		scaling_b = _RTU_SLAVE_SCALING_B_4;
		scaling_c = _RTU_SLAVE_SCALING_C_4;
		scaling_d = _RTU_SLAVE_SCALING_D_4;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[3];
	#elif (_RTU_SLAVE_WIND_GUSTS_SOURCE == 5)
		source = &RTU_GETTERS_F5_NAME;
		scaling_a = _RTU_SLAVE_SCALING_A_5;
		scaling_b = _RTU_SLAVE_SCALING_B_5;
		scaling_c = _RTU_SLAVE_SCALING_C_5;
		scaling_d = _RTU_SLAVE_SCALING_D_5;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[4];
	#elif (_RTU_SLAVE_WIND_GUSTS_SOURCE == 6)
		source = &RTU_GETTERS_F6_NAME;
		scaling_a = _RTU_SLAVE_SCALING_A_6;
		scaling_b = _RTU_SLAVE_SCALING_B_6;
		scaling_c = _RTU_SLAVE_SCALING_C_6;
		scaling_d = _RTU_SLAVE_SCALING_D_6;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[5];
	#else
		#error "Wrong Modbus Configuration"
	#endif
#else
	retval = MODBUS_RET_NOT_CONFIGURED;
#endif

	if (retval == MODBUS_RET_UNINITIALIZED && source != 0) {
		// copy the raw value from modbus register data
		raw_register_value = source->registers_values[0];

		// check when the value has been updated
		if (main_get_master_time() - last_update_timestam > RTU_MAXIMUM_VALUE_AGE) {
			retval = MODBUS_RET_NOT_AVALIABLE;
		}
		else {
			if (main_get_master_time() - rte_rtu_last_modbus_rx_error_timestamp < RTU_MAXIMUM_VALUE_AGE) {
				retval = MODBUS_RET_DEGRADED;
			}
			else {
				retval = MODBUS_RET_OK;
			}

			// rescale the raw value to target value in .1m/s incremenets
			*out = 	(uint16_t) (10 * (
								scaling_a * (uint16_t)raw_register_value * (uint16_t)raw_register_value +
								scaling_b * (uint16_t)raw_register_value +
								scaling_c
								) /
								scaling_d);
		}
	}
#endif

	return retval;
}

int32_t rtu_get_humidity(int8_t* out) {

	int32_t retval = MODBUS_RET_UNINITIALIZED;

	rtu_register_data_t* source = 0;

	volatile uint16_t raw_register_value = 0;

	uint16_t scaling_a, scaling_b, scaling_c, scaling_d;

	// the timestamp of last update of the register value
	uint32_t last_update_timestam = 0;

#ifdef _MODBUS_RTU
#ifdef _RTU_SLAVE_HUMIDITY_SOURCE
	#if (_RTU_SLAVE_HUMIDITY_SOURCE == 1)
		source = &RTU_GETTERS_F1_NAME;
		scaling_a = _RTU_SLAVE_SCALING_A_1;
		scaling_b = _RTU_SLAVE_SCALING_B_1;
		scaling_c = _RTU_SLAVE_SCALING_C_1;
		scaling_d = _RTU_SLAVE_SCALING_D_1;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[0];
	#elif (_RTU_SLAVE_HUMIDITY_SOURCE == 2)
		source = &RTU_GETTERS_F2_NAME;
		scaling_a = _RTU_SLAVE_SCALING_A_2;
		scaling_b = _RTU_SLAVE_SCALING_B_2;
		scaling_c = _RTU_SLAVE_SCALING_C_2;
		scaling_d = _RTU_SLAVE_SCALING_D_2;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[1];
	#elif (_RTU_SLAVE_HUMIDITY_SOURCE == 3)
		source = &RTU_GETTERS_F3_NAME;
		scaling_a = _RTU_SLAVE_SCALING_A_3;
		scaling_b = _RTU_SLAVE_SCALING_B_3;
		scaling_c = _RTU_SLAVE_SCALING_C_3;
		scaling_d = _RTU_SLAVE_SCALING_D_3;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[2];
	#elif (_RTU_SLAVE_HUMIDITY_SOURCE == 4)
		source = &RTU_GETTERS_F4_NAME;
		scaling_a = _RTU_SLAVE_SCALING_A_4;
		scaling_b = _RTU_SLAVE_SCALING_B_4;
		scaling_c = _RTU_SLAVE_SCALING_C_4;
		scaling_d = _RTU_SLAVE_SCALING_D_4;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[3];
	#else
		#error "Wrong Modbus Configuration"
	#endif
#else
	retval = MODBUS_RET_NOT_CONFIGURED;
#endif

	if (retval == MODBUS_RET_UNINITIALIZED && source != 0) {
		// copy the raw value from modbus register data
		raw_register_value = source->registers_values[0];

		// check when the value has been updated
		if (main_get_master_time() - last_update_timestam > RTU_MAXIMUM_VALUE_AGE) {
			retval = MODBUS_RET_NOT_AVALIABLE;
		}
		else {
			if (main_get_master_time() - rte_rtu_last_modbus_rx_error_timestamp < RTU_MAXIMUM_VALUE_AGE) {
				retval = MODBUS_RET_DEGRADED;
			}
			else {
				retval = MODBUS_RET_OK;
			}

			// rescale the raw value to target value in percents
			*out = 	(int8_t) (
								scaling_a * (uint16_t)raw_register_value * (uint16_t)raw_register_value +
								scaling_b * (uint16_t)raw_register_value +
								scaling_c
								/
								scaling_d);
		}
	}
#endif

	return retval;
}

void rtu_get_raw_values_string(char* out, uint16_t out_buffer_ln, uint8_t* generated_string_ln) {

	uint16_t f1_value = 0;
	uint16_t f2_value = 0;
	uint16_t f3_value = 0;
	uint16_t f4_value = 0;
	uint16_t f5_value = 0;
	uint16_t f6_value = 0;

	int string_ln = 0;

#if defined(_RTU_SLAVE_ID_1)
	f1_value = RTU_GETTERS_F1_NAME.registers_values[0];
#endif

#if defined(_RTU_SLAVE_ID_2)
	f2_value = RTU_GETTERS_F2_NAME.registers_values[0];
#endif

#if defined(_RTU_SLAVE_ID_3)
	f3_value = RTU_GETTERS_F3_NAME.registers_values[0];
#endif

#if defined(_RTU_SLAVE_ID_4)
	f4_value = RTU_GETTERS_F4_NAME.registers_values[0];
#endif

#if defined(_RTU_SLAVE_ID_5)
	f5_value = RTU_GETTERS_F5_NAME.registers_values[0];
#endif

#if defined(_RTU_SLAVE_ID_6)
	f6_value = RTU_GETTERS_F6_NAME.registers_values[0];
#endif

	string_ln = snprintf(out, out_buffer_ln, ">F1V %X, F2V %X, F3V %X, F4V %X, F5V %X, F6V %X",
												(int) f1_value,
												(int) f2_value,
												(int) f3_value,
												(int) f4_value,
												(int) f5_value,
												(int) f6_value);

	*generated_string_ln = (uint8_t)string_ln;
}

