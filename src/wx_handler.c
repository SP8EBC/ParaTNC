/*
 * wx_handler.c
 *
 *  Created on: 26.01.2019
 *      Author: mateusz
 */

#include "wx_handler.h"

#include <rte_wx.h>
#include <stm32f10x.h>
#include <math.h>
#include "drivers/_dht22.h"
#include "drivers/ms5611.h"
#include "drivers/analog_anemometer.h"

#include "station_config.h"

#include "delay.h"
#include "telemetry.h"
#include "main.h"

#define WX_WATCHDOG_PERIOD (SYSTICK_TICKS_PER_SECONDS * SYSTICK_TICKS_PERIOD * 90)
#define WX_WATCHDOG_RESET_DURATION (SYSTICK_TICKS_PER_SECONDS * SYSTICK_TICKS_PERIOD * 3)

#define WX_MAX_TEMPERATURE_SLEW_RATE 4.0f

uint32_t wx_last_good_temperature_time = 0;
uint32_t wx_last_good_wind_time = 0;
wx_pwr_state_t wx_pwr_state;
uint8_t wx_inhibit_slew_rate_check = 1;

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

		// calculate the slew rate
		rte_wx_temperature_dalls_slew_rate = rte_wx_temperature_dallas - rte_wx_temperature_dallas_valid;

		// chcecking the positive (ascending) slew rate of the temperature measuremenets
		if (rte_wx_temperature_dalls_slew_rate >  WX_MAX_TEMPERATURE_SLEW_RATE && wx_inhibit_slew_rate_check == 0) {

			// if temeperature measuremenet has changed more than maximum allowed slew rate set degradadet QF
			rte_wx_error_dallas_qf = DALLAS_QF_DEGRADATED;

			// and increase the temperature only by 1.0f to decrease slew rate
			rte_wx_temperature_dallas += 1.0f;

		}

		// chcecking the negaive (descending) slew rate of the temperature measuremenets
		if (rte_wx_temperature_dalls_slew_rate < -WX_MAX_TEMPERATURE_SLEW_RATE && wx_inhibit_slew_rate_check == 0) {

			// if temeperature measuremenet has changed more than maximum allowed slew rate set degradadet QF
			rte_wx_error_dallas_qf = DALLAS_QF_DEGRADATED;

			// and decrease the temperature only by 1.0f to decrease slew rate
			rte_wx_temperature_dallas -= 1.0f;

		}

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

		// updating last good measurement time
		wx_last_good_temperature_time = master_time;
	}
	else {
		// if there were a communication error set the error to unavaliable
		rte_wx_error_dallas_qf = DALLAS_QF_NOT_AVALIABLE;

	}

	// enabling slew rate checking after first power up
	wx_inhibit_slew_rate_check = 0;
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

void wx_pool_analog_anemometer(void) {

	// locals
	uint32_t average_windspeed = 0;
	uint32_t wind_direction_x_avg = 0;
	uint32_t wind_direction_y_avg = 0;
	uint16_t wind_direction_x = 0;
	uint16_t wind_direction_y = 0;
	short i = 0;

	// this windspeed is scaled * 10. Example: 0.2 meters per second is stored as 2
	uint16_t scaled_windspeed = analog_anemometer_get_ms_from_pulse(rte_wx_windspeed_pulses);

	// putting the wind speed into circular buffer
	rte_wx_windspeed[rte_wx_windspeed_it] = scaled_windspeed;

	// increasing the iterator to the windspeed buffer
	rte_wx_windspeed_it++;

	// checking if iterator reached an end of the buffer
	if (rte_wx_windspeed_it >= WIND_AVERAGE_LEN)
		rte_wx_windspeed_it = 0;

	// calculating the average windspeed
	for (i = 0; i < WIND_AVERAGE_LEN; i++)
		average_windspeed += rte_wx_windspeed[i];

	average_windspeed /= WIND_AVERAGE_LEN;

	// store the value in rte
	rte_wx_average_windspeed = average_windspeed;

	// reuse the local variable to find maximum value
	average_windspeed = 0;

	// looking for gusts
	for (i = 0; i < WIND_AVERAGE_LEN; i++) {
		if (average_windspeed < rte_wx_windspeed[i])
			average_windspeed = rte_wx_windspeed[i];
	}

	// storing wind gusts value in rte
	rte_wx_max_windspeed = average_windspeed;

	// calculating average wind direction
	for (i = 0; i < WIND_AVERAGE_LEN; i++) {
		// split the wind direction into x and y component
		wind_direction_x = (uint16_t)(100.0f * cosf((float)rte_wx_winddirection[i] * M_PI/180.0f));
		wind_direction_y = (uint16_t)(100.0f * sinf((float)rte_wx_winddirection[i] * M_PI/180.0f));

		// adding components to calculate average
		wind_direction_x_avg += wind_direction_x;
		wind_direction_y_avg += wind_direction_y;

	}

	// dividing to get average of x and y componen
	wind_direction_x_avg /= WIND_AVERAGE_LEN;
	wind_direction_y_avg /= WIND_AVERAGE_LEN;

	// converting x & y component of wind direction back to an angle
	rte_wx_average_winddirection = (uint16_t)(atan2f(wind_direction_y_avg , wind_direction_x_avg) * 180.0f/M_PI);

	if (rte_wx_average_winddirection < 0)
		rte_wx_average_winddirection += 360;
}

void wx_pwr_init(void) {
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	wx_pwr_state = WX_PWR_OFF;

	GPIO_ResetBits(GPIOB, GPIO_Pin_8);

}

void wx_pwr_periodic_handle(void) {
	// check when last measuremenets was sent by wind or temperature sensor
	if (	(master_time - wx_last_good_temperature_time >= WX_WATCHDOG_PERIOD ||
			 master_time - wx_last_good_wind_time >= WX_WATCHDOG_PERIOD) &&
			 wx_pwr_state == WX_PWR_ON) {

		// if timeout watchod expired there is a time to reset the supply voltage
		wx_pwr_state = WX_PWR_UNDER_RESET;

		// pulling the output down to switch the relay
		GPIO_ResetBits(GPIOB, GPIO_Pin_8);

		// setting the last_good timers to current value to prevent reset loop
		wx_last_good_temperature_time = master_time;
		wx_last_good_wind_time = master_time;

		return;
	}

	// service actual supply state
	switch (wx_pwr_state) {
	case WX_PWR_OFF:

		// one second delay
		delay_fixed(2000);

		GPIO_SetBits(GPIOB, GPIO_Pin_8);

		// power is off after power-up and needs to be powered on
		wx_pwr_state = WX_PWR_ON;
		break;
	case WX_PWR_ON:
		break;
	case WX_PWR_UNDER_RESET:

		GPIO_SetBits(GPIOB, GPIO_Pin_8);

		wx_pwr_state = WX_PWR_ON;

		break;
	}
}
