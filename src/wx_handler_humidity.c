/*
 * wx_handler_humidity.c
 *
 *  Created on: Apr 15, 2021
 *      Author: mateusz
 */

#include "wx_handler_humidity.h"
#include <wx_handler.h>

#include "rte_wx.h"

#include <modbus_rtu/rtu_getters.h>
#include <modbus_rtu/rtu_return_values.h>

int32_t wx_get_humidity_measurement(const config_data_wx_sources_t * const config_sources, const config_data_mode_t * const config_mode, const config_data_umb_t * const config_umb) {

	int32_t out = 0;
	int32_t measurement_result = 0;

	// choose a temperature source from the configuration
	switch(config_sources->humidity) {
		case WX_SOURCE_INTERNAL: {

			if (config_mode->wx_ms5611_or_bme == 0) {
				// MS5611 is chosen
				//

				// but MS5611 isn't a humidity sensor
				;
			}
			else {
				// BME280
				measurement_result = wx_get_humidity_bme280(&rte_wx_humidity);

				// if humidity has been retrieved successfully
				if (measurement_result == BME280_OK) {
					rte_wx_humidity_valid = rte_wx_humidity;
				}
			}

			break;
		}
		case WX_SOURCE_UMB: {
			break;
		}
		case WX_SOURCE_RTU:
		case WX_SOURCE_FULL_RTU: {

			// get the value read from RTU registers
			measurement_result = rtu_get_humidity(&rte_wx_humidity);

			// check
			if (measurement_result == MODBUS_RET_OK || measurement_result == MODBUS_RET_DEGRADED) {

				// set the flag that external temperature is available
				out |= WX_HANDLER_PARAMETER_RESULT_HUMIDITY;

				if (measurement_result == BME280_OK) {
					rte_wx_humidity_valid = rte_wx_humidity;
				}
			}

			break;
		}
		case WX_SOURCE_DAVIS_SERIAL:
			break;
	}

	return out;
}

int32_t wx_get_humidity_bme280(int8_t * const humidity) {
	int32_t output = 0;

	if (rte_wx_bme280_qf == BME280_QF_FULL) {

		output = bme280_get_humidity(humidity, bme280_get_adc_h(), &rte_wx_bme280_qf);
	}

	return output;
}
