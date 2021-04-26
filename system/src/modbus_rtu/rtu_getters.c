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

int32_t rtu_get_temperature(float* out, const config_data_rtu_t * const config) {

	int32_t retval = MODBUS_RET_UNINITIALIZED;

	rtu_register_data_t* source = 0;

	uint16_t raw_register_value = 0;

	float physical_register_value = 0.0f;

	uint16_t scaling_a, scaling_b, scaling_c, scaling_d;

	// the timestamp of last update of the register value
	uint32_t last_update_timestam = 0;

	switch (config->temperature_source) {
	case 1:
		source = &rte_wx_modbus_rtu_f1;
		scaling_a = config->slave_1_scaling_a;
		scaling_b = config->slave_1_scaling_b;
		scaling_c = config->slave_1_scaling_c;
		scaling_d = config->slave_1_scaling_d;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[0];
		break;
	case 2:
		source = &rte_wx_modbus_rtu_f2;
		scaling_a = config->slave_2_scaling_a;
		scaling_b = config->slave_2_scaling_b;
		scaling_c = config->slave_2_scaling_c;
		scaling_d = config->slave_2_scaling_d;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[1];
		source = &rte_wx_modbus_rtu_f2;
		break;
	case 3:
		source = &rte_wx_modbus_rtu_f3;
		scaling_a = config->slave_3_scaling_a;
		scaling_b = config->slave_3_scaling_b;
		scaling_c = config->slave_3_scaling_c;
		scaling_d = config->slave_3_scaling_d;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[2];
		source = &rte_wx_modbus_rtu_f3;
		break;
	case 4:
		source = &rte_wx_modbus_rtu_f4;
		scaling_a = config->slave_4_scaling_a;
		scaling_b = config->slave_4_scaling_b;
		scaling_c = config->slave_4_scaling_c;
		scaling_d = config->slave_4_scaling_d;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[3];
		source = &rte_wx_modbus_rtu_f4;
		break;
	case 5:
		source = &rte_wx_modbus_rtu_f5;
		scaling_a = config->slave_5_scaling_a;
		scaling_b = config->slave_5_scaling_b;
		scaling_c = config->slave_5_scaling_c;
		scaling_d = config->slave_5_scaling_d;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[4];
		source = &rte_wx_modbus_rtu_f5;
		break;
	case 6:
		source = &rte_wx_modbus_rtu_f6;
		scaling_a = config->slave_6_scaling_a;
		scaling_b = config->slave_6_scaling_b;
		scaling_c = config->slave_6_scaling_c;
		scaling_d = config->slave_6_scaling_d;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[5];
		source = &rte_wx_modbus_rtu_f6;
		break;
	default:
		retval = MODBUS_RET_NOT_CONFIGURED;
	}

	if (retval == MODBUS_RET_UNINITIALIZED && source != 0) {
		// copy the raw value from modbus register data
		raw_register_value = source->registers_values[0];

		// rescale the raw value to target value in degrees C
		physical_register_value = 	(
				scaling_a * (float)raw_register_value * (float)raw_register_value +
				scaling_b * (float)raw_register_value +
				scaling_c
				) /
				scaling_d;

		// check when the value has been updated
		if (main_get_master_time() - last_update_timestam > RTU_MAXIMUM_VALUE_AGE ||
				physical_register_value < RTU_MIN_VALID_TEMPERATURE ||
				physical_register_value > RTU_MAX_VALID_TEMPERATURE) {
			retval = MODBUS_RET_NOT_AVALIABLE;
		}
		else {
			if (main_get_master_time() - rte_rtu_last_modbus_rx_error_timestamp < RTU_MAXIMUM_VALUE_AGE) {
				retval = MODBUS_RET_DEGRADED;
			}
			else {
				retval = MODBUS_RET_OK;
			}

			*out = physical_register_value;
		}

	}

	return retval;
}

int32_t rtu_get_pressure(float* out, const config_data_rtu_t * const config) {

	int32_t retval = MODBUS_RET_UNINITIALIZED;

	rtu_register_data_t* source = 0;

	uint16_t raw_register_value = 0;

	float physical_register_value = 0.0f;

	uint16_t scaling_a, scaling_b, scaling_c, scaling_d;

	// the timestamp of last update of the register value
	uint32_t last_update_timestam = 0;

	switch (config->pressure_source) {
	case 1:
		source = &rte_wx_modbus_rtu_f1;
		scaling_a = config->slave_1_scaling_a;
		scaling_b = config->slave_1_scaling_b;
		scaling_c = config->slave_1_scaling_c;
		scaling_d = config->slave_1_scaling_d;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[0];
		break;
	case 2:
		source = &rte_wx_modbus_rtu_f2;
		scaling_a = config->slave_2_scaling_a;
		scaling_b = config->slave_2_scaling_b;
		scaling_c = config->slave_2_scaling_c;
		scaling_d = config->slave_2_scaling_d;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[1];
		source = &rte_wx_modbus_rtu_f2;
		break;
	case 3:
		source = &rte_wx_modbus_rtu_f3;
		scaling_a = config->slave_3_scaling_a;
		scaling_b = config->slave_3_scaling_b;
		scaling_c = config->slave_3_scaling_c;
		scaling_d = config->slave_3_scaling_d;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[2];
		source = &rte_wx_modbus_rtu_f3;
		break;
	case 4:
		source = &rte_wx_modbus_rtu_f4;
		scaling_a = config->slave_4_scaling_a;
		scaling_b = config->slave_4_scaling_b;
		scaling_c = config->slave_4_scaling_c;
		scaling_d = config->slave_4_scaling_d;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[3];
		source = &rte_wx_modbus_rtu_f4;
		break;
	case 5:
		source = &rte_wx_modbus_rtu_f5;
		scaling_a = config->slave_5_scaling_a;
		scaling_b = config->slave_5_scaling_b;
		scaling_c = config->slave_5_scaling_c;
		scaling_d = config->slave_5_scaling_d;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[4];
		source = &rte_wx_modbus_rtu_f5;
		break;
	case 6:
		source = &rte_wx_modbus_rtu_f6;
		scaling_a = config->slave_6_scaling_a;
		scaling_b = config->slave_6_scaling_b;
		scaling_c = config->slave_6_scaling_c;
		scaling_d = config->slave_6_scaling_d;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[5];
		source = &rte_wx_modbus_rtu_f6;
		break;
	default:
		retval = MODBUS_RET_NOT_CONFIGURED;
	}

	if (retval == MODBUS_RET_UNINITIALIZED && source != 0) {
		// copy the raw value from modbus register data
		raw_register_value = source->registers_values[0];

		// rescale the raw value to target value in degrees C
		physical_register_value = 	(
				scaling_a * (float)raw_register_value * (float)raw_register_value +
				scaling_b * (float)raw_register_value +
				scaling_c
				) /
				scaling_d;

		// check when the value has been updated
		if (main_get_master_time() - last_update_timestam > RTU_MAXIMUM_VALUE_AGE ||
				physical_register_value < RTU_MIN_VALID_PRESSURE ||
				physical_register_value > RTU_MAX_VALID_PRESSURE) {
			retval = MODBUS_RET_NOT_AVALIABLE;
		}
		else {
			if (main_get_master_time() - rte_rtu_last_modbus_rx_error_timestamp < RTU_MAXIMUM_VALUE_AGE) {
				retval = MODBUS_RET_DEGRADED;
			}
			else {
				retval = MODBUS_RET_OK;
			}

			*out = physical_register_value;
		}
	}

	return retval;
}

int32_t rtu_get_wind_direction(uint16_t* out, const config_data_rtu_t * const config) {

	int32_t retval = MODBUS_RET_UNINITIALIZED;

	rtu_register_data_t* source = 0;

	uint16_t raw_register_value = 0, physical_register_value = 0;

	uint16_t scaling_a, scaling_b, scaling_c, scaling_d;

	// the timestamp of last update of the register value
	uint32_t last_update_timestam = 0;

	switch (config->wind_direction_source) {
	case 1:
		source = &rte_wx_modbus_rtu_f1;
		scaling_a = config->slave_1_scaling_a;
		scaling_b = config->slave_1_scaling_b;
		scaling_c = config->slave_1_scaling_c;
		scaling_d = config->slave_1_scaling_d;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[0];
		break;
	case 2:
		source = &rte_wx_modbus_rtu_f2;
		scaling_a = config->slave_2_scaling_a;
		scaling_b = config->slave_2_scaling_b;
		scaling_c = config->slave_2_scaling_c;
		scaling_d = config->slave_2_scaling_d;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[1];
		source = &rte_wx_modbus_rtu_f2;
		break;
	case 3:
		source = &rte_wx_modbus_rtu_f3;
		scaling_a = config->slave_3_scaling_a;
		scaling_b = config->slave_3_scaling_b;
		scaling_c = config->slave_3_scaling_c;
		scaling_d = config->slave_3_scaling_d;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[2];
		source = &rte_wx_modbus_rtu_f3;
		break;
	case 4:
		source = &rte_wx_modbus_rtu_f4;
		scaling_a = config->slave_4_scaling_a;
		scaling_b = config->slave_4_scaling_b;
		scaling_c = config->slave_4_scaling_c;
		scaling_d = config->slave_4_scaling_d;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[3];
		source = &rte_wx_modbus_rtu_f4;
		break;
	case 5:
		source = &rte_wx_modbus_rtu_f5;
		scaling_a = config->slave_5_scaling_a;
		scaling_b = config->slave_5_scaling_b;
		scaling_c = config->slave_5_scaling_c;
		scaling_d = config->slave_5_scaling_d;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[4];
		source = &rte_wx_modbus_rtu_f5;
		break;
	case 6:
		source = &rte_wx_modbus_rtu_f6;
		scaling_a = config->slave_6_scaling_a;
		scaling_b = config->slave_6_scaling_b;
		scaling_c = config->slave_6_scaling_c;
		scaling_d = config->slave_6_scaling_d;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[5];
		source = &rte_wx_modbus_rtu_f6;
		break;
	default:
		retval = MODBUS_RET_NOT_CONFIGURED;
	}

	if (retval == MODBUS_RET_UNINITIALIZED && source != 0) {
		// copy the raw value from modbus register data
		raw_register_value = source->registers_values[0];

		// rescale the raw value to target value
		physical_register_value = 	(uint16_t)(
							scaling_a * (uint16_t)raw_register_value * (uint16_t)raw_register_value +
							scaling_b * (uint16_t)raw_register_value +
							scaling_c
							) /
							scaling_d;

		// check when the value has been updated
		if (main_get_master_time() - last_update_timestam > RTU_MAXIMUM_VALUE_AGE ||
			physical_register_value > 360) {
			retval = MODBUS_RET_NOT_AVALIABLE;
		}
		else {
			if (main_get_master_time() - rte_rtu_last_modbus_rx_error_timestamp < RTU_MAXIMUM_VALUE_AGE) {
				retval = MODBUS_RET_DEGRADED;
			}
			else {
				retval = MODBUS_RET_OK;
			}

			*out = physical_register_value;
		}
	}

	return retval;
}

int32_t rtu_get_wind_speed(uint16_t* out, const config_data_rtu_t * const config) {

	int32_t retval = MODBUS_RET_UNINITIALIZED;

	rtu_register_data_t* source = 0;

	uint16_t raw_register_value = 0, physical_register_value = 0;

	uint16_t scaling_a, scaling_b, scaling_c, scaling_d;

	// the timestamp of last update of the register value
	uint32_t last_update_timestam = 0;

	switch (config->wind_speed_source) {
	case 1:
		source = &rte_wx_modbus_rtu_f1;
		scaling_a = config->slave_1_scaling_a;
		scaling_b = config->slave_1_scaling_b;
		scaling_c = config->slave_1_scaling_c;
		scaling_d = config->slave_1_scaling_d;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[0];
		break;
	case 2:
		source = &rte_wx_modbus_rtu_f2;
		scaling_a = config->slave_2_scaling_a;
		scaling_b = config->slave_2_scaling_b;
		scaling_c = config->slave_2_scaling_c;
		scaling_d = config->slave_2_scaling_d;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[1];
		source = &rte_wx_modbus_rtu_f2;
		break;
	case 3:
		source = &rte_wx_modbus_rtu_f3;
		scaling_a = config->slave_3_scaling_a;
		scaling_b = config->slave_3_scaling_b;
		scaling_c = config->slave_3_scaling_c;
		scaling_d = config->slave_3_scaling_d;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[2];
		source = &rte_wx_modbus_rtu_f3;
		break;
	case 4:
		source = &rte_wx_modbus_rtu_f4;
		scaling_a = config->slave_4_scaling_a;
		scaling_b = config->slave_4_scaling_b;
		scaling_c = config->slave_4_scaling_c;
		scaling_d = config->slave_4_scaling_d;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[3];
		source = &rte_wx_modbus_rtu_f4;
		break;
	case 5:
		source = &rte_wx_modbus_rtu_f5;
		scaling_a = config->slave_5_scaling_a;
		scaling_b = config->slave_5_scaling_b;
		scaling_c = config->slave_5_scaling_c;
		scaling_d = config->slave_5_scaling_d;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[4];
		source = &rte_wx_modbus_rtu_f5;
		break;
	case 6:
		source = &rte_wx_modbus_rtu_f6;
		scaling_a = config->slave_6_scaling_a;
		scaling_b = config->slave_6_scaling_b;
		scaling_c = config->slave_6_scaling_c;
		scaling_d = config->slave_6_scaling_d;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[5];
		source = &rte_wx_modbus_rtu_f6;
		break;
	default:
		retval = MODBUS_RET_NOT_CONFIGURED;
	}

	if (retval == MODBUS_RET_UNINITIALIZED && source != 0) {
		// copy the raw value from modbus register data
		raw_register_value = source->registers_values[0];

		// rescale the raw value to target value in .1m/s incremenets
		physical_register_value = 	(uint16_t) (10 * (
							scaling_a * (uint16_t)raw_register_value * (uint16_t)raw_register_value +
							scaling_b * (uint16_t)raw_register_value +
							scaling_c
							) /
							scaling_d);

		// check when the value has been updated
		if (main_get_master_time() - last_update_timestam > RTU_MAXIMUM_VALUE_AGE ||
			physical_register_value > RTU_MAX_VALID_WINDSPEED) {
			retval = MODBUS_RET_NOT_AVALIABLE;
		}
		else {
			if (main_get_master_time() - rte_rtu_last_modbus_rx_error_timestamp < RTU_MAXIMUM_VALUE_AGE) {
				retval = MODBUS_RET_DEGRADED;
			}
			else {
				retval = MODBUS_RET_OK;
			}

			*out = physical_register_value;
		}
	}

	return retval;
}

int32_t rtu_get_wind_gusts(uint16_t* out, const config_data_rtu_t * const config) {
	int32_t retval = MODBUS_RET_UNINITIALIZED;

	rtu_register_data_t* source = 0;

	uint16_t raw_register_value = 0, physical_register_value = 0;

	uint16_t scaling_a, scaling_b, scaling_c, scaling_d;

	// the timestamp of last update of the register value
	uint32_t last_update_timestam = 0;

	switch (config->wind_gusts_source) {
	case 1:
		source = &rte_wx_modbus_rtu_f1;
		scaling_a = config->slave_1_scaling_a;
		scaling_b = config->slave_1_scaling_b;
		scaling_c = config->slave_1_scaling_c;
		scaling_d = config->slave_1_scaling_d;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[0];
		break;
	case 2:
		source = &rte_wx_modbus_rtu_f2;
		scaling_a = config->slave_2_scaling_a;
		scaling_b = config->slave_2_scaling_b;
		scaling_c = config->slave_2_scaling_c;
		scaling_d = config->slave_2_scaling_d;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[1];
		source = &rte_wx_modbus_rtu_f2;
		break;
	case 3:
		source = &rte_wx_modbus_rtu_f3;
		scaling_a = config->slave_3_scaling_a;
		scaling_b = config->slave_3_scaling_b;
		scaling_c = config->slave_3_scaling_c;
		scaling_d = config->slave_3_scaling_d;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[2];
		source = &rte_wx_modbus_rtu_f3;
		break;
	case 4:
		source = &rte_wx_modbus_rtu_f4;
		scaling_a = config->slave_4_scaling_a;
		scaling_b = config->slave_4_scaling_b;
		scaling_c = config->slave_4_scaling_c;
		scaling_d = config->slave_4_scaling_d;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[3];
		source = &rte_wx_modbus_rtu_f4;
		break;
	case 5:
		source = &rte_wx_modbus_rtu_f5;
		scaling_a = config->slave_5_scaling_a;
		scaling_b = config->slave_5_scaling_b;
		scaling_c = config->slave_5_scaling_c;
		scaling_d = config->slave_5_scaling_d;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[4];
		source = &rte_wx_modbus_rtu_f5;
		break;
	case 6:
		source = &rte_wx_modbus_rtu_f6;
		scaling_a = config->slave_6_scaling_a;
		scaling_b = config->slave_6_scaling_b;
		scaling_c = config->slave_6_scaling_c;
		scaling_d = config->slave_6_scaling_d;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[5];
		source = &rte_wx_modbus_rtu_f6;
		break;
	default:
		retval = MODBUS_RET_NOT_CONFIGURED;
	}

	if (retval == MODBUS_RET_UNINITIALIZED && source != 0) {
		// copy the raw value from modbus register data
		raw_register_value = source->registers_values[0];

		// rescale the raw value to target value in .1m/s incremenets
		physical_register_value = 	(uint16_t) (10 * (
							scaling_a * (uint16_t)raw_register_value * (uint16_t)raw_register_value +
							scaling_b * (uint16_t)raw_register_value +
							scaling_c
							) /
							scaling_d);

		// check when the value has been updated
		if (main_get_master_time() - last_update_timestam > RTU_MAXIMUM_VALUE_AGE ||
			physical_register_value > RTU_MAX_VALID_WINDSPEED) {
			retval = MODBUS_RET_NOT_AVALIABLE;
		}
		else {
			if (main_get_master_time() - rte_rtu_last_modbus_rx_error_timestamp < RTU_MAXIMUM_VALUE_AGE) {
				retval = MODBUS_RET_DEGRADED;
			}
			else {
				retval = MODBUS_RET_OK;
			}

			*out = physical_register_value;

		}
	}

	return retval;
}

int32_t rtu_get_humidity(int8_t* out, const config_data_rtu_t * const config) {

	int32_t retval = MODBUS_RET_UNINITIALIZED;

	rtu_register_data_t* source = 0;

	volatile uint16_t raw_register_value = 0;
	int8_t physical_register_value = 0;

	uint16_t scaling_a, scaling_b, scaling_c, scaling_d;

	// the timestamp of last update of the register value
	uint32_t last_update_timestam = 0;

	switch (config->humidity_source) {
	case 1:
		source = &rte_wx_modbus_rtu_f1;
		scaling_a = config->slave_1_scaling_a;
		scaling_b = config->slave_1_scaling_b;
		scaling_c = config->slave_1_scaling_c;
		scaling_d = config->slave_1_scaling_d;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[0];
		break;
	case 2:
		source = &rte_wx_modbus_rtu_f2;
		scaling_a = config->slave_2_scaling_a;
		scaling_b = config->slave_2_scaling_b;
		scaling_c = config->slave_2_scaling_c;
		scaling_d = config->slave_2_scaling_d;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[1];
		source = &rte_wx_modbus_rtu_f2;
		break;
	case 3:
		source = &rte_wx_modbus_rtu_f3;
		scaling_a = config->slave_3_scaling_a;
		scaling_b = config->slave_3_scaling_b;
		scaling_c = config->slave_3_scaling_c;
		scaling_d = config->slave_3_scaling_d;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[2];
		source = &rte_wx_modbus_rtu_f3;
		break;
	case 4:
		source = &rte_wx_modbus_rtu_f4;
		scaling_a = config->slave_4_scaling_a;
		scaling_b = config->slave_4_scaling_b;
		scaling_c = config->slave_4_scaling_c;
		scaling_d = config->slave_4_scaling_d;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[3];
		source = &rte_wx_modbus_rtu_f4;
		break;
	case 5:
		source = &rte_wx_modbus_rtu_f5;
		scaling_a = config->slave_5_scaling_a;
		scaling_b = config->slave_5_scaling_b;
		scaling_c = config->slave_5_scaling_c;
		scaling_d = config->slave_5_scaling_d;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[4];
		source = &rte_wx_modbus_rtu_f5;
		break;
	case 6:
		source = &rte_wx_modbus_rtu_f6;
		scaling_a = config->slave_6_scaling_a;
		scaling_b = config->slave_6_scaling_b;
		scaling_c = config->slave_6_scaling_c;
		scaling_d = config->slave_6_scaling_d;
		last_update_timestam = rte_rtu_pool_queue.last_successfull_call_to_function[5];
		source = &rte_wx_modbus_rtu_f6;
		break;
	default:
		retval = MODBUS_RET_NOT_CONFIGURED;
	}

	if (retval == MODBUS_RET_UNINITIALIZED && source != 0) {
		// copy the raw value from modbus register data
		raw_register_value = source->registers_values[0];

		// rescale the raw value to target value in percents
		physical_register_value = 	(int8_t) (
							scaling_a * (uint16_t)raw_register_value * (uint16_t)raw_register_value +
							scaling_b * (uint16_t)raw_register_value +
							scaling_c
							/
							scaling_d);

		// check when the value has been updated
		if (main_get_master_time() - last_update_timestam > RTU_MAXIMUM_VALUE_AGE ||
				physical_register_value > 100 || physical_register_value < 0) {
			retval = MODBUS_RET_NOT_AVALIABLE;
		}
		else {
			if (main_get_master_time() - rte_rtu_last_modbus_rx_error_timestamp < RTU_MAXIMUM_VALUE_AGE) {
				retval = MODBUS_RET_DEGRADED;
			}
			else {
				retval = MODBUS_RET_OK;
			}

			*out = physical_register_value;


		}
	}

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

	f1_value = rte_wx_modbus_rtu_f1.registers_values[0];

	f2_value = rte_wx_modbus_rtu_f2.registers_values[0];

	f3_value = rte_wx_modbus_rtu_f3.registers_values[0];

	f4_value = rte_wx_modbus_rtu_f4.registers_values[0];

	f5_value = rte_wx_modbus_rtu_f5.registers_values[0];

	f6_value = rte_wx_modbus_rtu_f6.registers_values[0];

	string_ln = snprintf(out, out_buffer_ln, ">F1V %X, F2V %X, F3V %X, F4V %X, F5V %X, F6V %X",
												(int) f1_value,
												(int) f2_value,
												(int) f3_value,
												(int) f4_value,
												(int) f5_value,
												(int) f6_value);

	*generated_string_ln = (uint8_t)string_ln;
}

