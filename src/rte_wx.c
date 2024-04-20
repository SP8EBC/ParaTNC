/*
 * rte_wx.c
 *
 *  Created on: 26.01.2019
 *      Author: mateusz
 */


#include <rte_wx.h>
#include <wx_handler.h>
#include "main.h"
#include "misc_config.h"

#ifndef RTE_WX_PROBLEMS_MAX_THRESHOLD
#define RTE_WX_PROBLEMS_MAX_THRESHOLD 20
#endif

/**
 * A little word of explanataion:
 * 	-> rte_wx_temperature_external_xxxxxx - these are default temperature readings provided
 * 											by a sensor of choice. It might be dallas, modbus-rtu or other
 *
 * 	-> rte_wx_temperature_inernal_xxxxx - 	this is always a temperature measured by inernal pressure and/or
 * 											humidity sensor. The intension here is to monitor how hot the
 * 											controller internally
 *
 */

float rte_wx_temperature_average_external_valid = 0.0f;	//<! This name should be refactored
float rte_wx_temperature_internal = 0.0f, rte_wx_temperature_internal_valid = 0.0f;
float rte_wx_pressure = 0.0f, rte_wx_pressure_valid = 0.0f;
float rte_wx_pressure_history[PRESSURE_AVERAGE_LN];
uint8_t rte_wx_pressure_it;

#if defined(STM32L471xx)
/**
 * These values are scaled * 10. As for now 'rte_wx_temperature_average_dallas'
 * is rescaled version of 'rte_wx_temperature_average_external_valid'. Some of
 * values are redundant as for now. Normally 'rte_wx_temperature_external'
 * stores the temperature measured by the sensor of choice (like dallas), which are
 * then used to send APRS meteo packets. In normal circumstances they might be more
 * sensors which are measuring temperature. Values below are used by API client
 * to POST them directly into Meteo System backend, which accepts more than one
 * source of temperature at once
 */
int16_t rte_wx_temperature_average_dallas = 0;
int16_t rte_wx_temperature_average_pt = 0;
int16_t rte_wx_temperature_average_modbus = 0;
int16_t rte_wx_temperature_average_internal = 0;
uint16_t rte_wx_pressure_average = 0;
#endif
uint16_t rte_wx_pm10 = 0;		// 2.5um
uint16_t rte_wx_pm2_5 = 0;		// 1um

int16_t rte_wx_average_winddirection = 0;
uint16_t rte_wx_average_windspeed = 0;
uint16_t rte_wx_max_windspeed = 0;

uint16_t rte_wx_windspeed_pulses = 0;
uint16_t rte_wx_windspeed[WIND_AVERAGE_LEN];
uint8_t rte_wx_windspeed_it = 0;
uint16_t rte_wx_winddirection[WIND_AVERAGE_LEN];
uint8_t rte_wx_winddirection_it = 0;
uint16_t rte_wx_winddirection_last = 0;

int8_t rte_wx_humidity = 0, rte_wx_humidity_valid = 0;

dallas_qf_t rte_wx_current_dallas_qf, rte_wx_error_dallas_qf = DALLAS_QF_UNKNOWN;
float_average_t rte_wx_dallas_average;
ms5611_qf_t rte_wx_ms5611_qf = MS5611_QF_UNKNOWN;
bme280_qf_t rte_wx_bme280_qf = BME280_QF_UKNOWN;
analog_wind_qf_t rte_wx_wind_qf = AN_WIND_QF_UNKNOWN;
uint8_t rte_wx_humidity_available = 0;
uint8_t rte_wx_dallas_degraded_counter = 0;


umb_frame_t rte_wx_umb;
umb_context_t rte_wx_umb_context;
uint8_t rte_wx_umb_last_status = 0;
int16_t rte_wx_umb_channel_values[UMB_CHANNELS_STORAGE_CAPAC][2];	// first dimension stores the channel number and the second one
															// stores the value in 0.1 incremenets
umb_qf_t rte_wx_umb_qf = UMB_QF_UNITIALIZED;

uint8_t rte_wx_davis_station_avaliable = 0;
uint8_t rte_wx_davis_loop_packet_avaliable = 0;
davis_loop_t rte_wx_davis_loop_content;

uint8_t rte_wx_problems_wind_buffers = 0;	//!< Problems detected with buffers content
uint8_t rte_wx_problems_wind_values = 0;	//!< Problems with values calculated from buffers content

void rte_wx_init(void) {
	int i = 0;

	rte_wx_problems_wind_buffers = 0;
	rte_wx_problems_wind_values = 0;

	for (; i < WIND_AVERAGE_LEN; i++) {
		rte_wx_windspeed[i] = 0;
		rte_wx_winddirection[i] = 0;
	}

	rte_wx_pressure_it = 0;

	for (i = 0; i < 4; i++) {
		rte_wx_pressure_history[i] = 0.0f;
	}

}

void rte_wx_update_last_measuremenet_timers(uint16_t parameter_type) {
	if (parameter_type == RTE_WX_MEASUREMENT_WIND)
		wx_last_good_wind_time = master_time;
	else if (parameter_type == RTE_WX_MEASUREMENT_TEMPERATURE)
		wx_last_good_temperature_time = master_time;
	else {
		;
	}
}

void rte_wx_reset_last_measuremenet_timers(uint16_t parameter_type) {
	if (parameter_type == RTE_WX_MEASUREMENT_WIND)
		wx_last_good_wind_time = 0xFFFFFFFF;
	else if (parameter_type == RTE_WX_MEASUREMENT_TEMPERATURE)
		wx_last_good_temperature_time = 0xFFFFFFFF;
	else {
		;
	}
}

/**
 * This function checks if weather measurements looks to be valid and if they
 * are changing over time. The function shall be called in one minute interval.
 * @return
 */
int8_t rte_wx_check_weather_measurements(void) {
	int8_t looks_good = 1;

	uint8_t i = 0;	// loop iterator

	// go through wind direction buffer and checks if it contains the same value
	for (i = 0; i < WIND_AVERAGE_LEN - 1; i++) {
		if (rte_wx_winddirection[i] != rte_wx_winddirection[i + 1]) {
			break;
		}
	}

	// check if an end of the buffer has been reached
	if (i >= WIND_AVERAGE_LEN - 1) {
		rte_wx_problems_wind_buffers++;
	}

	// go through wind speed buffer and checks if it contains the same value
	for (i = 0; i < WIND_AVERAGE_LEN - 1; i++) {
		if (rte_wx_windspeed[i] != rte_wx_windspeed[i + 1]) {
			break;
		}

		// break the loop if the windspeed is zero anywhere, not to reset
		// periodically if wind sensor is not connected.
		if (rte_wx_windspeed[i] == 0) {
			break;
		}
	}

	// check if an end of the buffer has been reached
	if (i >= WIND_AVERAGE_LEN - 1) {
		rte_wx_problems_wind_buffers++;
	}

	// check if average wind speed is different from zero and the same than gusts
	if (rte_wx_average_windspeed != 0 &&
			(rte_wx_average_windspeed == rte_wx_max_windspeed))
	{
		// if so a wind sensor had been blocked by icing very rapidly
		// before next DMA interrupt so rte_wx_windspeed is also
		// not updated at all
		rte_wx_problems_wind_values++;
	}

	// check if wind direction equals exactly north (zero degrees)
	if (rte_wx_average_winddirection == 0) {
		// open wind direction input (anemometer disconnected) gives
		// a reading of about 6 do 8 degrees. If it is stuck on zero
		// the U->f converted or its reference clock generator
		// might not work at all
		rte_wx_problems_wind_values++;
	}
	else {
		rte_wx_problems_wind_values = 0;
	}

	if (rte_wx_problems_wind_values > RTE_WX_PROBLEMS_MAX_THRESHOLD) {
		looks_good = 0;
	}

	if (rte_wx_problems_wind_buffers > RTE_WX_PROBLEMS_MAX_THRESHOLD * 3) {
		looks_good = 0;
	}

	return looks_good;
}
