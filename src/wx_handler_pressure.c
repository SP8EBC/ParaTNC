/*
 * wx_handler_pressure.c
 *
 *  Created on: Apr 15, 2021
 *      Author: mateusz
 */

#include "wx_handler_pressure.h"
#include "wx_handler.h"
#include "wx_handler_temperature.h"

#include "main.h"
#include "rte_wx.h"

#include <event_log.h>
#include <events_definitions/events_wx_handler.h>

#include <drivers/bme280.h>
#include <drivers/ms5611.h>

#include <modbus_rtu/rtu_getters.h>
#include <modbus_rtu/rtu_return_values.h>

int32_t wx_get_pressure_measurement (const config_data_wx_sources_t *const config_sources,
									 const config_data_mode_t *const config_mode,
									 const config_data_umb_t *const config_umb,
									 const config_data_rtu_t *const config_rtu)
{

	int32_t output = 0;
	int32_t measurement_retval = 0;
	int32_t i = 0, j = 0;
	float pressure_average_sum = 0.0f;
	umb_qf_t umb_quality_factor = UMB_QF_UNITIALIZED; // quality factor for UMB communication

	(void)config_rtu;

	switch (config_sources->pressure) {
	case WX_SOURCE_INTERNAL: {

		// switch between MS5611 and BME280 depends on user configuration
		if (config_mode->wx_ms5611_or_bme == 1) {
			measurement_retval = wx_get_pressure_bme280 (&rte_wx_pressure);
		}
		else if (config_mode->wx_ms5611_or_bme == 0) {
			measurement_retval = wx_get_pressure_ms5611 (&rte_wx_pressure);
		}
		else {
			measurement_retval = 0xFFFFFFFF;
		}

		// check if pressure has been retrieved correctly
		if (measurement_retval == BME280_OK || measurement_retval == MS5611_OK) {
			// BME280 measures all three things at one call to the driver
			output |= WX_HANDLER_PARAMETER_RESULT_PRESSURE;

			//			// get internal temperature
			//			if (config_mode->wx_ms5611_or_bme == 1) {
			//				measurement_retval =
			//wx_get_temperature_bme280(&rte_wx_temperature_internal);
			//			}
			//			else {
			//				measurement_retval =
			//wx_get_temperature_ms5611(&rte_wx_temperature_internal);
			//			}

			// incrementing iterator over pressure history
			rte_wx_pressure_it++;

			// check if and end of the buffer was reached
			if (rte_wx_pressure_it >= PRESSURE_AVERAGE_LN) {
				rte_wx_pressure_it = 0;
			}

			// add the current pressure into buffer for average calculation
			rte_wx_pressure_history[rte_wx_pressure_it] = rte_wx_pressure;

			// reseting the average length iterator
			j = 0;

			// calculating the average of pressure measuremenets
			for (i = 0; i < PRESSURE_AVERAGE_LN; i++) {

				// skip empty slots in the history to provide proper value even for first wx packet
				if (rte_wx_pressure_history[i] < 10.0f) {
					continue;
				}

				// add to the average
				pressure_average_sum += rte_wx_pressure_history[i];

				// increase the average lenght iterator
				j++;
			}

			rte_wx_pressure_valid = pressure_average_sum / (float)j;
		}
		else if (measurement_retval == (int32_t)0xFFFFFFFF) {
			;
		}
		else {
			// store an event
			event_log_sync (EVENT_WARNING,
							EVENT_SRC_WX_HANDLER,
							EVENTS_WX_HANDLER_WARN_PRESSURE_FAILED,
							measurement_retval,
							0,
							0,
							0,
							0,
							0);
		}
		break;
	}
	case WX_SOURCE_UMB: {
		// get current UMB bus quality factor
		umb_quality_factor = umb_get_current_qf (&rte_wx_umb_context, master_time);

		// if there are any data collected from UMB sensors
		if (umb_quality_factor == UMB_QF_FULL || umb_quality_factor == UMB_QF_DEGRADED) {

			// get the average pressure directly, there is no need for any further processing
			rte_wx_pressure = umb_get_qnh (config_umb);

			rte_wx_pressure_valid = rte_wx_pressure;

			// set the flag that external temperature is available
			output |= WX_HANDLER_PARAMETER_RESULT_PRESSURE;
		}
		else {
			// do nothing if no new data was received from UMB sensor in last 10 minutes
			;
		}

		break;
	}
	case WX_SOURCE_RTU:
	case WX_SOURCE_FULL_RTU: {

		//		// get the value read from RTU registers
		//		measurement_retval = rtu_get_humidity(&rte_wx_humidity, config_rtu);
		//
		//		// check
		//		if (measurement_retval == MODBUS_RET_OK || measurement_retval ==
		//MODBUS_RET_DEGRADED) {
		//
		//			// set the flag that external temperature is available
		//			output |= WX_HANDLER_PARAMETER_RESULT_HUMIDITY;
		//
		//			if (measurement_retval == BME280_OK) {
		//				rte_wx_humidity_valid = rte_wx_humidity;
		//			}
		//		}
		//
		break;
	}
	case WX_SOURCE_DAVIS_SERIAL:
	case WX_SOURCE_INTERNAL_PT100: break;
	}

#if defined(STM32L471xx)
	rte_wx_pressure_average = (uint16_t)(rte_wx_pressure_valid * 10.0f);

#endif

	return output;
}

int32_t wx_get_pressure_ms5611 (float *const pressure)
{

	int32_t return_value = 0;

	// quering MS5611 sensor for pressure
	return_value = ms5611_get_pressure (pressure, &rte_wx_ms5611_qf);

	return return_value;
}

int32_t wx_get_pressure_bme280 (float *const pressure)
{

	int32_t retval = 0;

	// check if raw data is avaliable
	if (rte_wx_bme280_qf == BME280_QF_FULL) {
		retval = bme280_get_pressure (pressure, bme280_get_adc_p (), &rte_wx_bme280_qf);
	}

	return retval;
}
