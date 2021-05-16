/*
 * wx_handler.c
 *
 *  Created on: 26.01.2019
 *      Author: mateusz
 */

#include "wx_handler.h"
#include "wx_handler_humidity.h"
#include "wx_handler_pressure.h"
#include "wx_handler_temperature.h"

#include <rte_wx.h>
#include <rte_rtu.h>
#include <math.h>
#include <stm32f10x.h>

#include "drivers/analog_anemometer.h"

#include "station_config.h"

#include "modbus_rtu/rtu_getters.h"
#include "modbus_rtu/rtu_return_values.h"

#include "delay.h"
#include "telemetry.h"
#include "main.h"

#define WX_WATCHDOG_PERIOD (SYSTICK_TICKS_PER_SECONDS * SYSTICK_TICKS_PERIOD * 90)
#define WX_WATCHDOG_RESET_DURATION (SYSTICK_TICKS_PER_SECONDS * SYSTICK_TICKS_PERIOD * 3)

uint32_t wx_last_good_wind_time = 0;
uint32_t wx_last_good_temperature_time = 0;
wx_pwr_state_t wx_pwr_state;
uint32_t wx_wind_pool_call_counter = 0;

static const float direction_constant = M_PI/180.0f;
static const config_data_wx_sources_t internal = {
		.temperature = WX_SOURCE_INTERNAL,
		.pressure = WX_SOURCE_INTERNAL,
		.humidity = WX_SOURCE_INTERNAL,
		.wind = WX_SOURCE_INTERNAL
};

#define MODBUS_QF_TEMPERATURE_FULL		1
#define MODBUS_QF_TEMPERATURE_DEGR		(1 << 1)
#define MODBUS_QF_TEMPERATURE_NAVB		(1 << 2)
#define MODBUS_QF_HUMIDITY_FULL 		(1 << 3)
#define MODBUS_QF_HUMIDITY_DEGR 		(1 << 4)
#define MODBUS_QF_PRESSURE_FULL			(1 << 5)
#define MODBUS_QF_PRESSURE_DEGR			(1 << 6)


void wx_get_all_measurements(const config_data_wx_sources_t * const config_sources, const config_data_mode_t * const config_mode, const config_data_umb_t * const config_umb, const config_data_rtu_t * const config_rtu) {

	int32_t parameter_result = 0;						// stores which parameters have been retrieved successfully. this is used for failsafe handling
	int32_t backup_parameter_result = 0;				// uses during retrieving backup

	parameter_result |= wx_get_temperature_measurement(config_sources, config_mode, config_umb, config_rtu);
	parameter_result |= wx_get_pressure_measurement(config_sources, config_mode, config_umb, config_rtu);
	parameter_result |= wx_get_humidity_measurement(config_sources, config_mode, config_umb, config_rtu);

	// check if all parameters (except wind) were collected successfully
	if (parameter_result == (WX_HANDLER_PARAMETER_RESULT_TEMPERATURE | WX_HANDLER_PARAMETER_RESULT_PRESSURE | WX_HANDLER_PARAMETER_RESULT_HUMIDITY | WX_HANDLER_PARAMETER_RESULT_TEMP_INTERNAL)) {
		;	// if everything were OK do nothing
	}
	else {
		// if not check what was faulty and backup with an internal sensor
		if ((parameter_result & WX_HANDLER_PARAMETER_RESULT_TEMPERATURE) == 0) {
			// if we don't have temperature
			// check what is the primary source of temperature
			if (config_sources->temperature != WX_SOURCE_INTERNAL) {
				// if this is something different than an internal source use the internal sensor
				backup_parameter_result |= wx_get_temperature_measurement(&internal, config_mode, config_umb, config_rtu);
			}
			else {
				; //
			}
		}

		if ((parameter_result & WX_HANDLER_PARAMETER_RESULT_PRESSURE) == 0) {

			if (config_sources->pressure != WX_SOURCE_INTERNAL) {
				backup_parameter_result |= wx_get_pressure_measurement(&internal, config_mode, config_umb, config_rtu);
			}
		}

		if ((parameter_result & WX_HANDLER_PARAMETER_RESULT_HUMIDITY) == 0) {

			if (config_sources->pressure != WX_SOURCE_INTERNAL) {
				backup_parameter_result |= wx_get_humidity_measurement(&internal, config_mode, config_umb, config_rtu);
			}
		}
	}


}



int32_t wx_get_bme280_temperature_pressure_humidity(float * const temperature, float * const pressure, int8_t * const humidity) {

	int32_t return_value = 0;

	// reading raw values from BME280 sensor
	return_value = bme280_read_raw_data(bme280_data_buffer);

	if (return_value == BME280_OK) {

		// setting back the Quality Factor to FULL to trace any problems with sensor readouts
		rte_wx_bme280_qf = BME280_QF_FULL;

		// converting raw values to temperature
		//bme280_get_temperature(temperature, bme280_get_adc_t(), &rte_wx_bme280_qf);

		// if modbus RTU is enabled but the quality factor for RTU-pressure is set to NOT_AVALIABLE
		//bme280_get_pressure(pressure, bme280_get_adc_p(), &rte_wx_bme280_qf);

		// if modbus RTU is enabled but the quality factor for RTU-humidity is set to NOT_AVALIABLE
		//bme280_get_humidity(humidity, bme280_get_adc_h(), &rte_wx_bme280_qf);

	}
	else {
		// set the quality factor is sensor is not responding on the i2c bus
		rte_wx_bme280_qf = BME280_QF_NOT_AVAILABLE;
	}

	return return_value;
}


void wx_pool_anemometer(const config_data_wx_sources_t * const config_sources, const config_data_mode_t * const config_mode, const config_data_umb_t * const config_umb, const config_data_rtu_t * const config_rtu) {

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

	int32_t modbus_retval;

	wx_wind_pool_call_counter++;

	uint16_t scaled_windspeed = 0;

	// internal sensors
	if (config_sources->wind == WX_SOURCE_INTERNAL) {
		// this windspeed is scaled * 10. Example: 0.2 meters per second is stored as 2
		scaled_windspeed = analog_anemometer_get_ms_from_pulse(rte_wx_windspeed_pulses);
	}

	else if (config_sources->wind == WX_SOURCE_UMB) {
		rte_wx_average_winddirection = umb_get_winddirection(config_umb);
		rte_wx_average_windspeed = umb_get_windspeed(config_umb);
		rte_wx_max_windspeed = umb_get_windgusts(config_umb);
	}

	else if (config_sources->wind == WX_SOURCE_RTU) {
		// get the value from modbus registers
		modbus_retval = rtu_get_wind_speed(&scaled_windspeed, config_rtu);

		// check if this value has been processed w/o errors
		if (modbus_retval == MODBUS_RET_OK) {
			// if yes continue to further processing
			modbus_retval = rtu_get_wind_direction(&rte_wx_winddirection_last, config_rtu);

		}

		// the second IF to check if the return value was the same for wind direction
		if (modbus_retval == MODBUS_RET_OK || modbus_retval == MODBUS_RET_DEGRADED) {
			// if the value is not available (like modbus is not configured as a source
			// for wind data) get the value from internal sensors..
			#ifdef _INTERNAL_AS_BACKUP
				// .. if they are configured
				scaled_windspeed = analog_anemometer_get_ms_from_pulse(rte_wx_windspeed_pulses);
			#endif
		}
	}

	else if (config_sources->wind == WX_SOURCE_FULL_RTU) {
		// get the value from modbus registers
		modbus_retval = rtu_get_wind_direction(&rte_wx_average_winddirection, config_rtu);

		// check if this value has been processed w/o errors
		if (modbus_retval == MODBUS_RET_OK || modbus_retval == MODBUS_RET_DEGRADED) {
			// if yes continue to further processing
			modbus_retval = rtu_get_wind_gusts(&rte_wx_max_windspeed, config_rtu);
			modbus_retval = rtu_get_wind_speed(&rte_wx_winddirection_last, config_rtu);

		}
	}
	else {
		;
	}

	if (config_sources->wind != WX_SOURCE_FULL_RTU) {
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

	}

	if (config_sources->wind == WX_SOURCE_FULL_RTU || config_sources->wind != WX_SOURCE_RTU) {
		if (modbus_retval == MODBUS_RET_OK) {
			rte_wx_wind_qf = AN_WIND_QF_FULL;
		}
		else if (modbus_retval == MODBUS_RET_DEGRADED) {
			rte_wx_wind_qf = AN_WIND_QF_DEGRADED;
		}
		else if (modbus_retval == MODBUS_RET_NOT_AVALIABLE) {
			rte_wx_wind_qf = AN_WIND_QF_NOT_AVALIABLE;
		}
		else {
			if ((config_mode->wx & WX_INTERNAL_AS_BACKUP) != 0)
				rte_wx_wind_qf = analog_anemometer_get_qf();
			else
				rte_wx_wind_qf = AN_WIND_QF_NOT_AVALIABLE;
		}
	}
	else if (config_sources->wind == WX_SOURCE_INTERNAL) {
		rte_wx_wind_qf = analog_anemometer_get_qf();
	}
	else {
		rte_wx_wind_qf = AN_WIND_QF_UNKNOWN;
	}


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

	// do a last valid measuremenets timestamps only if power is currently applied
	if (wx_pwr_state == WX_PWR_ON) {

		// the value of 0xFFFFFFFF is a magic word which disables the check for this parameter
		if (wx_last_good_temperature_time != 0xFFFFFFFF &&
			master_time - wx_last_good_temperature_time >= WX_WATCHDOG_PERIOD)
		{
			wx_pwr_state = WX_PWR_UNDER_RESET;
		}

		// as the weather station could be configured not to perform wind measurements at all
		if (wx_last_good_wind_time != 0xFFFFFFFF &&
			master_time - wx_last_good_wind_time >= WX_WATCHDOG_PERIOD)
		{
			wx_pwr_state = WX_PWR_UNDER_RESET;

			rte_wx_wind_qf = AN_WIND_QF_DEGRADED;
		}

		if (wx_pwr_state == WX_PWR_UNDER_RESET) {
			// if timeout watchod expired there is a time to reset the supply voltage
			wx_pwr_state = WX_PWR_UNDER_RESET;

			// pull the output down to switch the relay and disable +5V_ISOL (VDD_SW)
			GPIO_ResetBits(GPIOB, GPIO_Pin_8);

			// setting the last_good timers to current value to prevent reset loop
			wx_last_good_temperature_time = master_time;
			wx_last_good_wind_time = master_time;

			return;
		}

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
