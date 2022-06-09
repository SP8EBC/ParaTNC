/*
 * rte_wx.c
 *
 *  Created on: 26.01.2019
 *      Author: mateusz
 */


#include <rte_wx.h>
#include <wx_handler.h>
#include "main.h"

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

float rte_wx_temperature_external = 0.0f, rte_wx_temperature_external_valid = 0.0f;
float rte_wx_temperature_external_slew_rate = 0.0f;
float rte_wx_temperature_average_external_valid = 0.0f;
float rte_wx_temperature_min_external_valid = 0.0f, rte_wx_temperature_max_external_valid = 0.0f;
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

uint16_t rte_wx_pm10 = 0;		// 2.5um
uint16_t rte_wx_pm2_5 = 0;		// 1um
#endif
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
dallas_average_t rte_wx_dallas_average;
ms5611_qf_t rte_wx_ms5611_qf = MS5611_QF_UNKNOWN;
bme280_qf_t rte_wx_bme280_qf = BME280_QF_UKNOWN;
analog_wind_qf_t rte_wx_wind_qf = AN_WIND_QF_UNKNOWN;


umb_frame_t rte_wx_umb;
umb_context_t rte_wx_umb_context;
uint8_t rte_wx_umb_last_status = 0;
int16_t rte_wx_umb_channel_values[UMB_CHANNELS_STORAGE_CAPAC][2];	// first dimension stores the channel number and the second one
															// stores the value in 0.1 incremenets
umb_qf_t rte_wx_umb_qf = UMB_QF_UNITIALIZED;

uint8_t rte_wx_davis_station_avaliable = 0;
uint8_t rte_wx_davis_loop_packet_avaliable = 0;
davis_loop_t rte_wx_davis_loop_content;

void rte_wx_init(void) {
	int i = 0;

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
