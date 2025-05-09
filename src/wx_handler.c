/*
 * wx_handler.c
 *
 *  Created on: 26.01.2019
 *      Author: mateusz
 */

#include "wx_handler.h"
#include "wx_handler_humidity.h"
#include "wx_handler_pressure.h"
#include "wx_handler_temperature.h"

#include <rte_wx.h>
#include <rte_rtu.h>
#include <rte_main.h>
#include <math.h>

#ifdef STM32F10X_MD_VL
#include <stm32f10x.h>
#endif

#ifdef STM32L471xx
#include <stm32l4xx.h>
#include <stm32l4xx_ll_gpio.h>
#include "drivers/max31865.h"
#endif
#include "drivers/analog_anemometer.h"

#include "station_config.h"

#include "modbus_rtu/rtu_getters.h"
#include "modbus_rtu/rtu_return_values.h"

#include "io.h"
#include "delay.h"
#include "telemetry.h"
#include "main.h"

#define WX_WATCHDOG_PERIOD (SYSTICK_TICKS_PER_SECONDS * SYSTICK_TICKS_PERIOD * 90)
#define WX_WATCHDOG_RESET_DURATION (SYSTICK_TICKS_PER_SECONDS * SYSTICK_TICKS_PERIOD * 3)

uint32_t wx_last_good_wind_time = 0;
uint32_t wx_last_good_temperature_time = 0;
uint32_t wx_wind_pool_call_counter = 0;
uint8_t wx_force_i2c_sensor_reset = 0;

static const float direction_constant = M_PI/180.0f;
static const config_data_wx_sources_t internal = {
		.temperature = WX_SOURCE_INTERNAL,
		.pressure = WX_SOURCE_INTERNAL,
		.humidity = WX_SOURCE_INTERNAL,
		.wind = WX_SOURCE_INTERNAL
};

#define MODBUS_QF_TEMPERATURE_FULL		1
#define MODBUS_QF_TEMPERATURE_DEGR		(1 << 1)
#define MODBUS_QF_TEMPERATURE_NAVB		(1 << 2)
#define MODBUS_QF_HUMIDITY_FULL 		(1 << 3)
#define MODBUS_QF_HUMIDITY_DEGR 		(1 << 4)
#define MODBUS_QF_PRESSURE_FULL			(1 << 5)
#define MODBUS_QF_PRESSURE_DEGR			(1 << 6)


void wx_check_force_i2c_reset(void) {

	if (wx_force_i2c_sensor_reset == 1) {
		wx_force_i2c_sensor_reset = 0;

#ifdef STM32L471xx
		max31865_init(main_config_data_mode->wx_pt_sensor & 0x3, (main_config_data_mode->wx_pt_sensor & 0xFC) >> 2);
#endif

		if (main_config_data_mode->wx_ms5611_or_bme == 0) {
		 ms5611_reset(&rte_wx_ms5611_qf);
		 ms5611_read_calibration(SensorCalData, &rte_wx_ms5611_qf);
		 ms5611_trigger_measure(0, 0);
		}
		else if (main_config_data_mode->wx_ms5611_or_bme == 1) {
		 bme280_reset(&rte_wx_bme280_qf);
		 bme280_setup();
		 bme280_read_calibration(bme280_calibration_data);
		}
		else {
			;
		}
	}

}

void wx_get_all_measurements(const config_data_wx_sources_t * const config_sources, const config_data_mode_t * const config_mode, const config_data_umb_t * const config_umb, const config_data_rtu_t * const config_rtu) {

	int32_t parameter_result = 0;						// stores which parameters have been retrieved successfully. this is used for failsafe handling
	int32_t backup_parameter_result = 0;				// uses during retrieving backup

	if (io_get_5v_isol_sw___cntrl_vbat_s() == 0 && io_get_cntrl_vbat_s() == 0) {
		// inhibit any measurement when power is not applied to sensors
		return;
	}

	parameter_result |= wx_get_temperature_measurement(config_sources, config_mode, config_umb, config_rtu, &rte_wx_temperature_average_external_valid);
	parameter_result |= wx_get_pressure_measurement(config_sources, config_mode, config_umb, config_rtu);
	parameter_result |= wx_get_humidity_measurement(config_sources, config_mode, config_umb, config_rtu);

	// check if all parameters (except wind) were collected successfully
	if (parameter_result == (WX_HANDLER_PARAMETER_RESULT_TEMPERATURE | WX_HANDLER_PARAMETER_RESULT_PRESSURE | WX_HANDLER_PARAMETER_RESULT_HUMIDITY | WX_HANDLER_PARAMETER_RESULT_TEMP_INTERNAL)) {
		;	// if everything were OK do nothing
	}
	else {
		// if not check what was faulty and backup with an internal sensor
		if ((parameter_result & WX_HANDLER_PARAMETER_RESULT_TEMPERATURE) == 0) {
			// if we don't have temperature
			// check what is the primary source of temperature
			if (config_sources->temperature != WX_SOURCE_INTERNAL) {
				// if this is something different than an internal source use the internal sensor
				backup_parameter_result |= wx_get_temperature_measurement(&internal, config_mode, config_umb, config_rtu, &rte_wx_temperature_average_external_valid);
			}
			else {
				; //
			}
		}

		if ((parameter_result & WX_HANDLER_PARAMETER_RESULT_PRESSURE) == 0) {

			if (config_sources->pressure != WX_SOURCE_INTERNAL) {
				backup_parameter_result |= wx_get_pressure_measurement(&internal, config_mode, config_umb, config_rtu);
			}
		}

		if ((parameter_result & WX_HANDLER_PARAMETER_RESULT_HUMIDITY) == 0) {

			if (config_sources->pressure != WX_SOURCE_INTERNAL) {
				backup_parameter_result |= wx_get_humidity_measurement(&internal, config_mode, config_umb, config_rtu);
			}
		}
	}


}

void wx_pool_anemometer(const config_data_wx_sources_t * const config_sources, const config_data_mode_t * const config_mode, const config_data_umb_t * const config_umb, const config_data_rtu_t * const config_rtu) {

	// locals
	uint32_t average_windspeed = 0;
	int32_t wind_direction_x_avg = 0;
	int32_t wind_direction_y_avg = 0;
	int16_t wind_direction_x = 0;
	int16_t wind_direction_y = 0;
	volatile float dir_temp = 0;
	volatile float arctan_value = 0.0f;
	short i = 0;
	uint8_t average_ln;

	int32_t modbus_retval = MODBUS_RET_UNINITIALIZED;
	uint16_t scaled_windspeed = 0;

	(void)(config_mode);


	wx_wind_pool_call_counter++;

	// internal sensors
	if (config_sources->wind == WX_SOURCE_INTERNAL) {
		// this windspeed is scaled * 10. Example: 0.2 meters per second is stored as 2
		scaled_windspeed = analog_anemometer_get_ms_from_pulse(rte_wx_windspeed_pulses);
	}

	else if (config_sources->wind == WX_SOURCE_UMB) {
		rte_wx_average_winddirection = umb_get_winddirection(config_umb);
		rte_wx_average_windspeed = umb_get_windspeed(config_umb);
		rte_wx_max_windspeed = umb_get_windgusts(config_umb);
	}

	else if (config_sources->wind == WX_SOURCE_RTU) {
		// get the value from modbus registers
		modbus_retval = rtu_get_wind_speed(&scaled_windspeed, config_rtu);

		// check if this value has been processed w/o errors
		if (modbus_retval == MODBUS_RET_OK) {
			// if yes continue to further processing
			modbus_retval = rtu_get_wind_direction(&rte_wx_winddirection_last, config_rtu);

		}

		// the second IF to check if the return value was the same for wind direction
		if (modbus_retval == MODBUS_RET_OK || modbus_retval == MODBUS_RET_DEGRADED) {
			// if the value is not available (like modbus is not configured as a source
			// for wind data) get the value from internal sensors..
			#ifdef _INTERNAL_AS_BACKUP
				// .. if they are configured
				scaled_windspeed = analog_anemometer_get_ms_from_pulse(rte_wx_windspeed_pulses);
			#endif
		}
	}

	else if (config_sources->wind == WX_SOURCE_FULL_RTU) {
		// get the value from modbus registers
		modbus_retval = rtu_get_wind_direction((uint16_t*)(&rte_wx_average_winddirection), config_rtu);

		// check if this value has been processed w/o errors
		if (modbus_retval == MODBUS_RET_OK || modbus_retval == MODBUS_RET_DEGRADED) {
			// if yes continue to further processing
			modbus_retval = rtu_get_wind_gusts(&rte_wx_max_windspeed, config_rtu);
			modbus_retval = rtu_get_wind_speed(&rte_wx_winddirection_last, config_rtu);

		}
	}
	else {
		;
	}

	if (config_sources->wind != WX_SOURCE_FULL_RTU && config_sources->wind != WX_SOURCE_UMB) {
		// check how many times before the pool function was called
		if (wx_wind_pool_call_counter < WIND_AVERAGE_LEN) {
			// if it was called less time than a length of buffers, the average length
			// needs to be shortened to handle the underrun properly
			average_ln = (uint8_t)wx_wind_pool_call_counter;
		}
		else {
			average_ln = WIND_AVERAGE_LEN;
		}

		// putting the wind speed into circular buffer
		rte_wx_windspeed[rte_wx_windspeed_it] = scaled_windspeed;

		// increasing the iterator to the windspeed buffer
		rte_wx_windspeed_it++;

		// checking if iterator reached an end of the buffer
		if (rte_wx_windspeed_it >= WIND_AVERAGE_LEN) {
			rte_wx_windspeed_it = 0;
		}

		// calculating the average windspeed
		for (i = 0; i < average_ln; i++)
			average_windspeed += rte_wx_windspeed[i];

		average_windspeed /= average_ln;

		// store the value in rte
		rte_wx_average_windspeed = average_windspeed;

		// reuse the local variable to find maximum value
		average_windspeed = 0;

		// looking for gusts
		for (i = 0; i < average_ln; i++) {
			if (average_windspeed < rte_wx_windspeed[i])
				average_windspeed = rte_wx_windspeed[i];
		}

		// storing wind gusts value in rte
		rte_wx_max_windspeed = average_windspeed;

		// adding last wind direction to the buffers
		if (rte_wx_winddirection_it >= WIND_AVERAGE_LEN)
			rte_wx_winddirection_it = 0;

		rte_wx_winddirection[rte_wx_winddirection_it++] = rte_wx_winddirection_last;

		// calculating average wind direction
		for (i = 0; i < average_ln; i++) {

			dir_temp = (float)rte_wx_winddirection[i];

			// split the wind direction into x and y component
			wind_direction_x = (int16_t)(100.0f * cosf(dir_temp * direction_constant));
			wind_direction_y = (int16_t)(100.0f * sinf(dir_temp * direction_constant));

			// adding components to calculate average
			wind_direction_x_avg += wind_direction_x;
			wind_direction_y_avg += wind_direction_y;

		}

		// dividing to get average of x and y componen
		wind_direction_x_avg /= average_ln;
		wind_direction_y_avg /= average_ln;

		// converting x & y component of wind direction back to an angle
		arctan_value = atan2f(wind_direction_y_avg , wind_direction_x_avg);

		rte_wx_average_winddirection = (int16_t)(arctan_value * (180.0f/M_PI));

		if (rte_wx_average_winddirection < 0)
			rte_wx_average_winddirection += 360;

	}

	rte_wx_wind_qf = AN_WIND_QF_NOT_AVALIABLE;

	if (config_sources->wind == WX_SOURCE_UMB) {
		rte_wx_wind_qf = umb_get_current_qf(&rte_wx_umb_context, main_get_master_time());
	}


}

uint16_t wx_get_nvm_record_temperature(void) {

	uint16_t out = 0;
	uint16_t scaled_temperature = 0;
	uint16_t scaled_humidity = 0;

	if (rte_wx_temperature_average_external_valid > -50.0f && rte_wx_temperature_average_external_valid < 50.0f) {
		scaled_temperature = (uint16_t)((rte_wx_temperature_average_external_valid + 50.0f) * 5.0f);
	}

	scaled_humidity = rte_wx_humidity_valid / 2;

	out = (scaled_temperature & 0x1FF) | ((scaled_humidity & 0x1F) << 10);

	return out;
}

uint16_t wx_get_nvm_record_wind(void) {

	uint16_t out = 0;

	uint8_t scaled_average_windspeed = 0;
	uint8_t scaled_windgusts = 0;
	uint16_t wind_direction = 0;

	scaled_average_windspeed = rte_wx_average_windspeed / 2;
	if ((rte_wx_max_windspeed - scaled_average_windspeed) < 52) {
		scaled_windgusts = (uint8_t)lroundf((rte_wx_max_windspeed - scaled_average_windspeed) / 3.0f);
	}

	if (wind_direction <= 11 || wind_direction >= 349)
		wind_direction = 0;
	else if (wind_direction <= 34 && wind_direction > 11)
		wind_direction = 1;
	else if (wind_direction <= 56 && wind_direction > 34)
		wind_direction = 2;
	else if (wind_direction <= 79 && wind_direction > 56)
		wind_direction = 3;
	else if (wind_direction <= 101 && wind_direction > 79)
		wind_direction = 4;
	else if (wind_direction <= 124 && wind_direction > 101)
		wind_direction = 5;
	else if (wind_direction <= 146 && wind_direction > 124)
		wind_direction = 6;
	else if (wind_direction <= 169 && wind_direction > 146)
		wind_direction = 7;
	else if (wind_direction <= 191 && wind_direction > 169)
		wind_direction = 8;
	else if (wind_direction <= 214 && wind_direction > 191)
		wind_direction = 9;
	else if (wind_direction <= 236 && wind_direction > 214)
		wind_direction = 10;
	else if (wind_direction <= 259 && wind_direction > 236)
		wind_direction = 11;
	else if (wind_direction <= 281 && wind_direction > 259)
		wind_direction = 12;
	else if (wind_direction <= 304 && wind_direction > 281)
		wind_direction = 13;
	else if (wind_direction <= 327 && wind_direction > 304)
		wind_direction = 14;
	else if (wind_direction <= 349 && wind_direction > 327)
		wind_direction = 15;
	else {
		;
	}

	out = (scaled_average_windspeed | (scaled_windgusts & 0xF) << 8 | (wind_direction & 0xF) << 12 );

	return out;
}

