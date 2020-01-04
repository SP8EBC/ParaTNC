/*
 * rte_wx.c
 *
 *  Created on: 26.01.2019
 *      Author: mateusz
 */


#include <rte_wx.h>

float rte_wx_temperature_dallas = 0.0f, rte_wx_temperature_dallas_valid = 0.0f;
float rte_wx_temperature_dalls_slew_rate = 0.0f;
float rte_wx_temperature_average_dallas_valid = 0.0f;
float rte_wx_temperature_min_dallas_valid = 0.0f, rte_wx_temperature_max_dallas_valid = 0.0f;
float rte_wx_temperature_ms = 0.0f, rte_wx_temperature_ms_valid = 0.0f;
float rte_wx_pressure = 0.0f, rte_wx_pressure_valid = 0.0f;

uint16_t rte_wx_windspeed_pulses = 0;
uint16_t rte_wx_windspeed[WIND_AVERAGE_LEN];
uint8_t rte_wx_windspeed_it = 0;
uint16_t rte_wx_winddirection[WIND_AVERAGE_LEN];
uint8_t rte_wx_winddirection_it = 0;
uint16_t rte_wx_average_windspeed = 0;
uint16_t rte_wx_max_windspeed = 0;
int16_t rte_wx_average_winddirection = 0;

uint8_t rte_wx_tx20_excessive_slew_rate = 0;

dht22Values rte_wx_dht, rte_wx_dht_valid;		// quality factor inside this structure
DallasQF rte_wx_current_dallas_qf, rte_wx_error_dallas_qf = DALLAS_QF_UNKNOWN;
DallasAverage_t rte_wx_dallas_average;
ms5611_qf_t rte_wx_ms5611_qf;

void rte_wx_init(void) {
	int i = 0;

	for (; i < WIND_AVERAGE_LEN; i++) {
		rte_wx_windspeed[i] = 0;
		rte_wx_winddirection[i] = 0;
	}
}
