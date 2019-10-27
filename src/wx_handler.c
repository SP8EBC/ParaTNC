/*
 * wx_handler.c
 *
 *  Created on: 26.01.2019
 *      Author: mateusz
 */

#include <rte_wx.h>
#include "drivers/_dht22.h"
#include "drivers/ms5611.h"

#include "station_config.h"

#include "telemetry.h"

void wx_get_all_measurements(void) {

	int32_t return_value = 0;

#ifdef _METEO
	// quering MS5611 sensor for temperature
	return_value = ms5611_get_temperature(&rte_wx_temperature_ms, &rte_wx_ms5611_qf);

	if (return_value == MS5611_OK) {
		rte_wx_temperature_ms_valid = rte_wx_temperature_ms;

	}

#endif

#if defined _METEO || defined _DALLAS_AS_TELEM

	// quering dallas DS12B20 thermometer for current temperature
	rte_wx_temperature_dallas = dallas_query(&rte_wx_current_dallas_qf);

	// checking if communication was successfull
	if (rte_wx_temperature_dallas != -128.0f) {

		// store current value
		rte_wx_temperature_dallas_valid = rte_wx_temperature_dallas;

		// include current temperature into the average
		dallas_average(rte_wx_temperature_dallas, &rte_wx_dallas_average);

		// update the current temperature with current average
		rte_wx_temperature_average_dallas_valid = dallas_get_average(&rte_wx_dallas_average);

		// update current minimal temperature
		rte_wx_temperature_min_dallas_valid = dallas_get_min(&rte_wx_dallas_average);

		// and update maximum also
		rte_wx_temperature_max_dallas_valid = dallas_get_max(&rte_wx_dallas_average);
	}
	else {
		// if there were a communication error set the error to unavaliable
		rte_wx_error_dallas_qf = DALLAS_QF_NOT_AVALIABLE;

	}
	//else
	//	rte_wx_temperature_dallas_valid = 0.0f;
#endif

#ifdef _METEO
	// quering MS5611 sensor for pressure
	return_value = ms5611_get_pressure(&rte_wx_pressure,  &rte_wx_ms5611_qf);

	if (return_value == MS5611_OK) {
		rte_wx_pressure_valid = rte_wx_pressure;
	}

	// if humidity sensor is idle trigger the communiction & measuremenets
	if (dht22State == DHT22_STATE_DONE || dht22State == DHT22_STATE_TIMEOUT)
		dht22State = DHT22_STATE_IDLE;

#endif
}

void wx_pool_dht22(void) {

	dht22_timeout_keeper();

	switch (dht22State) {
		case DHT22_STATE_IDLE:
			dht22_comm(&rte_wx_dht);
			break;
		case DHT22_STATE_DATA_RDY:
			dht22_decode(&rte_wx_dht);
			break;
		case DHT22_STATE_DATA_DECD:
			rte_wx_dht_valid = rte_wx_dht;			// powrot do stanu DHT22_STATE_IDLE jest w TIM3_IRQHandler
			//rte_wx_dht_valid.qf = DHT22_QF_FULL;
			dht22State = DHT22_STATE_DONE;

#ifdef _DBG_TRACE
			trace_printf("DHT22: temperature=%d,humi=%d\r\n", dht_valid.scaledTemperature, dht_valid.humidity);
#endif
			break;
		case DHT22_STATE_TIMEOUT:
			rte_wx_dht_valid.qf = DHT22_QF_UNAVALIABLE;
			break;
		default: break;
	}

}
