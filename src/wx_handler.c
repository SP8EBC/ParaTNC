/*
 * wx_handler.c
 *
 *  Created on: 26.01.2019
 *      Author: mateusz
 */

#include "wx_handler.h"

#include <rte_wx.h>
#include <math.h>
#include <stm32f10x.h>
#include "drivers/_dht22.h"
#include "drivers/ms5611.h"
#include "drivers/analog_anemometer.h"
#include "drivers/tx20.h"

#include "station_config.h"

#ifdef _MODBUS_RTU
#include "modbus_rtu/rtu_getters.h"
#include "modbus_rtu/rtu_return_values.h"
#endif

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
uint32_t wx_wind_pool_call_counter = 0;

static const float direction_constant = M_PI/180.0f;

void wx_get_all_measurements(void) {

	int8_t j = 0;
	int32_t i = 0;
	int32_t return_value = 0;
	float pressure_average_sum = 0.0f;

#if defined(_UMB_MASTER) && !defined(_DAVIS_SERIAL) && !defined(_MODBUS_RTU)
	if (rte_wx_umb_qf == UMB_QF_FULL) {
		rte_wx_temperature_average_dallas_valid = umb_get_temperature();
		rte_wx_pressure_valid = umb_get_qfe();
	}
#endif

#if !defined(_UMB_MASTER) && !defined(_DAVIS_SERIAL) && defined(_MODBUS_RTU)
	// modbus rtu
	rtu_get_temperature(&rte_wx_temperature_average_dallas_valid);
	rtu_get_humidity(&rte_wx_humidity_valid);
	rtu_get_pressure(&rte_wx_pressure_valid);
#endif

#if (!defined(_UMB_MASTER) && !defined(_DAVIS_SERIAL) && !defined(_MODBUS_RTU) && defined (_SENSOR_MS5611)) || (defined (_SENSOR_MS5611) && defined(_INTERNAL_AS_BACKUP))
	// quering MS5611 sensor for temperature
	return_value = ms5611_get_temperature(&rte_wx_temperature_ms, &rte_wx_ms5611_qf);

	if (return_value == MS5611_OK) {
		rte_wx_temperature_ms_valid = rte_wx_temperature_ms;

	}

#endif

#if (!defined(_UMB_MASTER) && !defined(_DAVIS_SERIAL) && !defined(_MODBUS_RTU) && defined (_SENSOR_BME280)) || (defined (_SENSOR_BME280) && defined(_INTERNAL_AS_BACKUP))
	// reading raw values
	return_value = bme280_read_raw_data(bme280_data_buffer);

	if (return_value == BME280_OK) {

		// setting back the Quality Factor to FULL to trace any problems with sensor readouts
		rte_wx_bme280_qf = BME280_QF_FULL;

		// converting raw values to temperature
		bme280_get_temperature(&rte_wx_temperature_ms, bme280_get_adc_t(), &rte_wx_bme280_qf);

		// converting raw values to pressure
		bme280_get_pressure(&rte_wx_pressure, bme280_get_adc_p(), &rte_wx_bme280_qf);

		// converting raw values to humidity
		bme280_get_humidity(&rte_wx_humidity, bme280_get_adc_h(), &rte_wx_bme280_qf);

		if (rte_wx_bme280_qf == BME280_QF_FULL) {

			rte_wx_pressure_valid = rte_wx_pressure;
			rte_wx_temperature_ms_valid = rte_wx_temperature_ms;
			rte_wx_humidity_valid = rte_wx_humidity;

			// add the current pressure into buffer
			rte_wx_pressure_history[rte_wx_pressure_it++] = rte_wx_pressure;

			// reseting the average length iterator
			j = 0;

			// check if and end of the buffer was reached
			if (rte_wx_pressure_it >= PRESSURE_AVERAGE_LN) {
				rte_wx_pressure_it = 0;
			}

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
	}
	else {
		// set the quality factor is sensor is not responding on the i2c bus
		rte_wx_bme280_qf = BME280_QF_NOT_AVAILABLE;
	}
#endif

#if (!defined(_UMB_MASTER) && !defined(_DAVIS_SERIAL) && !defined(_MODBUS_RTU)) || defined(_INTERNAL_AS_BACKUP) || defined (_DALLAS_AS_TELEM)
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

#if (!defined(_UMB_MASTER) && !defined(_DAVIS_SERIAL) && !defined(_MODBUS_RTU) && defined (_SENSOR_MS5611)) || (defined (_SENSOR_MS5611) && defined(_INTERNAL_AS_BACKUP))
	// quering MS5611 sensor for pressure
	return_value = ms5611_get_pressure(&rte_wx_pressure,  &rte_wx_ms5611_qf);

	if (return_value == MS5611_OK) {
		// add the current pressure into buffer
		rte_wx_pressure_history[rte_wx_pressure_it++] = rte_wx_pressure;

		// reseting the average length iterator
		j = 0;

		// check if and end of the buffer was reached
		if (rte_wx_pressure_it >= PRESSURE_AVERAGE_LN) {
			rte_wx_pressure_it = 0;
		}

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

#endif

#ifdef _METEO
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
			rte_wx_humidity = rte_wx_dht.humidity;
			dht22State = DHT22_STATE_DONE;
			break;
		case DHT22_STATE_TIMEOUT:
			rte_wx_dht_valid.qf = DHT22_QF_UNAVALIABLE;
			break;
		default: break;
	}

}

void wx_pool_anemometer(void) {

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

#ifdef _MODBUS_RTU
	int32_t modbus_retval;
#endif

	wx_wind_pool_call_counter++;

	uint16_t scaled_windspeed = 0;

	// internal sensors
	#if defined(_ANEMOMETER_ANALOGUE) && !defined(_UMB_MASTER) && !defined(_MODBUS_RTU) || (defined(_INTERNAL_AS_BACKUP) && defined(_ANEMOMETER_ANALOGUE))
	// this windspeed is scaled * 10. Example: 0.2 meters per second is stored as 2
	scaled_windspeed = analog_anemometer_get_ms_from_pulse(rte_wx_windspeed_pulses);
	#endif

	#if defined(_ANEMOMETER_TX20) && !defined(_UMB_MASTER) && !defined(_MODBUS_RTU) || (defined(_INTERNAL_AS_BACKUP) && defined(_ANEMOMETER_TX20))
	scaled_windspeed = tx20_get_scaled_windspeed();
	rte_wx_winddirection_last = tx20_get_wind_direction();
	#endif

	#if defined(_UMB_MASTER)
	rte_wx_average_winddirection = umb_get_winddirection();
	rte_wx_average_windspeed = umb_get_windspeed();
	rte_wx_max_windspeed = umb_get_windgusts();
	#else

	#ifdef _MODBUS_RTU
	// get the value from modbus registers
	modbus_retval = rtu_get_wind_speed(&scaled_windspeed);

	// check if this value has been processed w/o errors
	if (modbus_retval == MODBUS_RET_OK || modbus_retval == MODBUS_RET_DEGRADED) {
		// if yes continue to further processing
		modbus_retval = rtu_get_wind_direction(&rte_wx_winddirection_last);

	}

	// the second IF to check if the return value was the same for wind direction
	if (modbus_retval != MODBUS_RET_OK && modbus_retval != MODBUS_RET_DEGRADED) {
		// if the value is not available (like modbus is not configured as a source
		// for wind data) get the value from internal sensors..
		#ifdef _INTERNAL_AS_BACKUP
			// .. if they are configured
			scaled_windspeed = analog_anemometer_get_ms_from_pulse(rte_wx_windspeed_pulses);
		#endif
	}
	#endif

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

#if defined (_MODBUS_RTU)
	if (modbus_retval == MODBUS_RET_OK) {
		rte_wx_wind_qf = AN_WIND_QF_FULL;
	}
	else if (modbus_retval == MODBUS_RET_DEGRADED) {
		rte_wx_wind_qf = AN_WIND_QF_DEGRADED;
	}
	else {
		#ifdef _INTERNAL_AS_BACKUP
			rte_wx_wind_qf = analog_anemometer_get_qf();
		#else
			rte_wx_wind_qf = AN_WIND_QF_NOT_AVALIABLE;
		#endif
	}
#elif defined(_ANEMOMETER_ANALOGUE)
	rte_wx_wind_qf = analog_anemometer_get_qf();
#else
	rte_wx_wind_qf = AN_WIND_QF_UNKNOWN;
#endif


	#endif
}

void wx_pwr_init(void) {
	// RELAY_CNTRL
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
#if (defined PARATNC_HWREV_A || defined PARATNC_HWREV_B)
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
#elif (defined PARATNC_HWREV_C)
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
#else
#error ("Hardware Revision not chosen.")
#endif
	GPIO_Init(GPIOB, &GPIO_InitStructure);

#if (defined PARATNC_HWREV_C)
	// +12V PWR_CNTRL
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
#endif

	wx_pwr_state = WX_PWR_OFF;

	GPIO_ResetBits(GPIOB, GPIO_Pin_8);

#if (defined PARATNC_HWREV_C)
	// +12V_SW PWR_CNTRL
	GPIO_ResetBits(GPIOA, GPIO_Pin_6);
#endif

}

void wx_pwr_periodic_handle(void) {
	// check when last measuremenets was sent by wind or temperature sensor
	if (	(master_time - wx_last_good_temperature_time >= WX_WATCHDOG_PERIOD ||
			 master_time - wx_last_good_wind_time >= WX_WATCHDOG_PERIOD) &&
			 wx_pwr_state == WX_PWR_ON) {

		// if timeout watchod expired there is a time to reset the supply voltage
		wx_pwr_state = WX_PWR_UNDER_RESET;

		// pulling the output down to switch the relay and disable +5V_ISOL (VDD_SW)
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

#if (defined PARATNC_HWREV_C)
		// Turn on the +12V_SW voltage
		GPIO_SetBits(GPIOA, GPIO_Pin_6);
#endif

		delay_fixed(100);

		// Turn on the +5V_ISOL (VDD_SW) voltage
		GPIO_SetBits(GPIOB, GPIO_Pin_8);

		// power is off after power-up and needs to be powered on
		wx_pwr_state = WX_PWR_ON;
		break;
	case WX_PWR_ON:
		break;
	case WX_PWR_UNDER_RESET:

		// Turn on the +5V_ISOL (VDD_SW) voltage
		GPIO_SetBits(GPIOB, GPIO_Pin_8);

		wx_pwr_state = WX_PWR_ON;

		break;
	case WX_PWR_DISABLED:
		break;
	}
}

void wx_pwr_disable_12v_sw(void) {
#if (defined PARATNC_HWREV_C)
	wx_pwr_state = WX_PWR_DISABLED;

	GPIO_ResetBits(GPIOA, GPIO_Pin_6);

#endif
}

void wx_pwr_disable_5v_isol(void) {
	wx_pwr_state = WX_PWR_DISABLED;

	GPIO_ResetBits(GPIOB, GPIO_Pin_8);


}

void wx_pwr_enable_12v_sw(void) {
#if (defined PARATNC_HWREV_C)
	wx_pwr_state = WX_PWR_OFF;

	// setting last good measurements timers to inhibit relay clicking
	// just after the power is applied
	wx_last_good_temperature_time =  master_time;
	wx_last_good_wind_time = master_time;

#endif
}

void wx_pwr_enable_5v_isol(void) {
#if (defined PARATNC_HWREV_C)
	wx_pwr_state = WX_PWR_OFF;

	// setting last good measurements timers to inhibit relay clicking
	// just after the power is applied
	wx_last_good_temperature_time =  master_time;
	wx_last_good_wind_time = master_time;

#endif
}
