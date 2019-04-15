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

#if defined _METEO || defined _DALLAS_AS_TELEM

	// quering dallas DS12B20 thermometer for current temperature
	rte_wx_temperature_dallas = dallas_query(&rte_wx_current_dallas_qf);

	// checking if communication was successfull
	if (rte_wx_temperature_dallas != -128.0f) {

		// update the current temperature
		rte_wx_temperature_dallas_valid = rte_wx_temperature_dallas;

		if (rte_wx_temperature_dallas_valid > TELEMETRY_MIN_DALLAS && rte_wx_temperature_dallas_valid < TELEMETRY_MAX_DALLAS)
			// and set the quality factor
			rte_wx_current_dallas_qf = DALLAS_QF_FULL;
		else
			rte_wx_current_dallas_qf = DALLAS_QF_DEGRADATED;
	}
	else {

		rte_wx_error_dallas_qf = DALLAS_QF_NOT_AVALIABLE;
	}
	//else
	//	rte_wx_temperature_dallas_valid = 0.0f;
#endif

#ifdef _METEO
	// quering MS5611 sensor for temperature
	return_value = ms5611_get_temperature(&rte_wx_temperature, &rte_wx_ms5611_qf);

	if (return_value == MS5611_OK) {
		rte_wx_temperature_valid = rte_wx_temperature;

	}

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
