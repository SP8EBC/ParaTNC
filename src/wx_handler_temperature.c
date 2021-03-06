/*
 * wx_handler_temperature.c
 *
 *  Created on: Apr 14, 2021
 *      Author: mateusz
 */

#include "wx_handler_temperature.h"

#include <rte_wx.h>
#include <wx_handler.h>
#include <main.h>

#include <drivers/dallas.h>
#include <drivers/ms5611.h>
#include <drivers/bme280.h>

#include <modbus_rtu/rtu_getters.h>
#include <modbus_rtu/rtu_return_values.h>

#define WX_MAX_TEMPERATURE_SLEW_RATE 4.0f

uint8_t wx_inhibit_slew_rate_check = 1;

int32_t wx_get_temperature_measurement(const config_data_wx_sources_t * const config_sources, const config_data_mode_t * const config_mode, const config_data_umb_t * const config_umb, const config_data_rtu_t * const config_rtu) {


	int32_t measurement_result = -1;						// used for return values from various functions
	int32_t parameter_result = 0;						// stores which parameters have been retrieved successfully. this is used for failsafe handling
	umb_qf_t umb_quality_factor = UMB_QF_UNITIALIZED;	// wuality factor for UMB communication

	// choose a temperature source from the configuration
	switch(config_sources->temperature) {
		// controller measures two temperatures
		//	internal - provided by pressure/humidity sensor on PCB
		//  external - usually dallas one wire but it might by something different

		case WX_SOURCE_INTERNAL: {
			// internal means sensors connected directly to the controller - one-wire and/or I2C on the PCB
			// it has nothing to do with distinction between external and internal temperature

			// check which sensor is configured. it doesn't check which one is
			// in fact installed. if the configuration doesn't mach with hardware
			// the measuremenet won't be retrieved
			if (config_mode->wx_ms5611_or_bme == 1) {
				// this will get all three parameters (humidity, pressure, internal temp) in single call
				measurement_result = wx_get_temperature_bme280(&rte_wx_temperature_internal);
			}
			else {
				// ms5611 is a bit different as the sensor (internal) temperature is collected separately from pressure
				measurement_result = wx_get_temperature_ms5611(&rte_wx_temperature_internal);
			}

			// check if temperature from pressure sensor has been retrieved w/o errors
			if (measurement_result == BME280_OK || measurement_result == MS5611_OK) {

				// set the flag for internal temperature
				parameter_result = parameter_result | WX_HANDLER_PARAMETER_RESULT_TEMP_INTERNAL;

			}

			// measure an external temperature using Dallas one wire sensor.
			// this function has blockin I/O which also adds a delay required by MS5611
			// sensor to finish data acquisition after the pressure measurement
			// is triggered.
			measurement_result = wx_get_temperature_dallas();

			// check if communication with dallas sensor has successed
			if (measurement_result == 0) {
				// if yes set the local variable with flag signalling that we have an external temperature
				parameter_result = parameter_result | WX_HANDLER_PARAMETER_RESULT_TEMPERATURE;
			}

			break;
		}
		case WX_SOURCE_UMB: {
			// get current UMB bus quality factor
			umb_quality_factor = umb_get_current_qf(&rte_wx_umb_context, master_time);

			// if there are any data collected from UMB sensors
			if (umb_quality_factor == UMB_QF_FULL || umb_quality_factor == UMB_QF_DEGRADED) {

				// get the average temperature directly, there is no need for any further processing
				rte_wx_temperature_average_external_valid = umb_get_temperature(config_umb);

				// set the flag that external temperature is available
				parameter_result = parameter_result | WX_HANDLER_PARAMETER_RESULT_TEMPERATURE;
			}
			else {
				// do nothing if no new data was received from UMB sensor in last 10 minutes
				;
			}

			break;
		}
		case WX_SOURCE_RTU:
		case WX_SOURCE_FULL_RTU: {

			// get the value read from RTU registers
			measurement_result = rtu_get_temperature(&rte_wx_temperature_external, config_rtu);

			// check
			if (measurement_result == MODBUS_RET_OK || measurement_result == MODBUS_RET_DEGRADED) {

				// set the flag that external temperature is available
				parameter_result |= WX_HANDLER_PARAMETER_RESULT_TEMPERATURE;
			}

			break;
		}
		case WX_SOURCE_DAVIS_SERIAL:
			break;

	}

	return parameter_result;
}

int32_t wx_get_temperature_dallas() {

	int32_t output = 0;

	// get the value from dallas one-wire sensor
	rte_wx_temperature_external = dallas_query(&rte_wx_current_dallas_qf);

	// checking if communication was successfull
	if (rte_wx_temperature_external != -128.0f) {

		// calculate the slew rate
		rte_wx_temperature_external_slew_rate = rte_wx_temperature_external - rte_wx_temperature_external_valid;

		// chcecking the positive (ascending) slew rate of the temperature measuremenets
		if (rte_wx_temperature_external_slew_rate >  WX_MAX_TEMPERATURE_SLEW_RATE && wx_inhibit_slew_rate_check == 0) {

			// if temeperature measuremenet has changed more than maximum allowed slew rate set degradadet QF
			rte_wx_error_dallas_qf = DALLAS_QF_DEGRADATED;

			// and increase the temperature only by 1.0f to decrease slew rate
			rte_wx_temperature_external += 1.0f;

		}

		// chcecking the negaive (descending) slew rate of the temperature measuremenets
		if (rte_wx_temperature_external_slew_rate < -WX_MAX_TEMPERATURE_SLEW_RATE && wx_inhibit_slew_rate_check == 0) {

			// if temeperature measuremenet has changed more than maximum allowed slew rate set degradadet QF
			rte_wx_error_dallas_qf = DALLAS_QF_DEGRADATED;

			// and decrease the temperature only by 1.0f to decrease slew rate
			rte_wx_temperature_external -= 1.0f;

		}

		// store current value
		rte_wx_temperature_external_valid = rte_wx_temperature_external;

		// include current temperature into the average
		dallas_average(rte_wx_temperature_external, &rte_wx_dallas_average);

		// update the current temperature with current average
		rte_wx_temperature_average_external_valid = dallas_get_average(&rte_wx_dallas_average);

		// update current minimal temperature
		rte_wx_temperature_min_external_valid = dallas_get_min(&rte_wx_dallas_average);

		// and update maximum also
		rte_wx_temperature_max_external_valid = dallas_get_max(&rte_wx_dallas_average);

		// updating last good measurement time
		wx_last_good_temperature_time = master_time;
	}
	else {
		// if there were a communication error set the error to unavaliable
		rte_wx_error_dallas_qf = DALLAS_QF_NOT_AVALIABLE;

		// set the output value
		output = -1;
	}

	return output;
}

int32_t wx_get_temperature_ms5611(float * const temperature) {
	int32_t return_value = 0;

	// quering MS5611 sensor for temperature
	return_value = ms5611_get_temperature(temperature, &rte_wx_ms5611_qf);

	return return_value;
}

int32_t wx_get_temperature_bme280(float * const temperature){
	int32_t output = 0;

	if (rte_wx_bme280_qf == BME280_QF_FULL) {
		output = bme280_get_temperature(temperature, bme280_get_adc_t(), &rte_wx_bme280_qf);
	}

	return output;
}


