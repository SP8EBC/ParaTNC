/*
 * rte_wx.h
 *
 *  Created on: 26.01.2019
 *      Author: mateusz
 */

#ifndef RTE_WX_H_
#define RTE_WX_H_

#include "station_config.h"
#include "station_config_target_hw.h"

#include "drivers/dallas.h"
#include "drivers/analog_anemometer.h"
#include "davis_vantage/davis_qf_t.h"
#include "davis_vantage/davis_loop_t.h"
#include "umb_master/umb_master.h"
#include "umb_master/umb_qf_t.h"

#include "drivers/ms5611.h"
#include "drivers/bme280.h"

#include "float_average.h"


#define WIND_AVERAGE_LEN 18

#define PRESSURE_AVERAGE_LN 4

#define RTE_WX_MEASUREMENT_WIND 		460
#define RTE_WX_MEASUREMENT_TEMPERATURE	100
#define RTE_WX_MEASUREMENT_PRESSUERE	300

extern float rte_wx_temperature_average_external_valid;
extern float rte_wx_temperature_internal, rte_wx_temperature_internal_valid;
extern float rte_wx_pressure, rte_wx_pressure_valid;
extern float rte_wx_pressure_history[PRESSURE_AVERAGE_LN];
extern uint8_t rte_wx_pressure_it;

#if defined(STM32L471xx)
extern int16_t rte_wx_temperature_average_dallas;
extern int16_t rte_wx_temperature_average_pt;
extern int16_t rte_wx_temperature_average_internal;
extern int16_t rte_wx_temperature_average_modbus;
extern uint16_t rte_wx_pressure_average;

#endif
extern uint16_t rte_wx_pm10;		// 2.5um
extern uint16_t rte_wx_pm2_5;		// 1um

extern uint16_t rte_wx_windspeed_pulses;
extern uint16_t rte_wx_windspeed[WIND_AVERAGE_LEN];
extern uint8_t rte_wx_windspeed_it;
extern uint16_t rte_wx_winddirection[WIND_AVERAGE_LEN];
extern uint8_t rte_wx_winddirection_it;
extern uint16_t rte_wx_winddirection_last;
extern uint16_t rte_wx_average_windspeed;
extern uint16_t rte_wx_max_windspeed;
extern int16_t rte_wx_average_winddirection;

extern uint8_t rte_wx_analog_anemometer_counter_timer_has_been_fired;
extern uint8_t rte_wx_analog_anemometer_counter_slew_limit_fired;
extern uint8_t rte_wx_analog_anemometer_counter_deboucing_fired;
extern uint8_t rte_wx_analog_anemometer_counter_direction_doesnt_work;

inline uint16_t rte_wx_get_minimum_windspeed(void) {
	uint16_t out = 0xFFFF;

	for (int i = 0 ; i < WIND_AVERAGE_LEN; i++) {
		if (rte_wx_windspeed[i] < out) {
			out = rte_wx_windspeed[i];
		}
	}

	return out;
}

extern int8_t rte_wx_humidity, rte_wx_humidity_valid;

extern uint8_t rte_wx_tx20_excessive_slew_rate;

extern dallas_qf_t rte_wx_current_dallas_qf, rte_wx_error_dallas_qf;
extern float_average_t rte_wx_dallas_average;
extern ms5611_qf_t rte_wx_ms5611_qf;
extern bme280_qf_t rte_wx_bme280_qf;
extern analog_wind_qf_t rte_wx_wind_qf;
extern uint8_t rte_wx_humidity_available;
extern uint8_t rte_wx_dallas_degraded_counter;


extern umb_frame_t rte_wx_umb;
extern umb_context_t rte_wx_umb_context;
extern uint8_t rte_wx_umb_last_status;
extern int16_t rte_wx_umb_channel_values[UMB_CHANNELS_STORAGE_CAPAC][2];
															// stores the value in 0.1 incremenets
extern umb_qf_t rte_wx_umb_qf;

extern uint8_t rte_wx_davis_station_avaliable;
extern uint8_t rte_wx_davis_loop_packet_avaliable;
extern davis_loop_t rte_wx_davis_loop_content;


#ifdef __cplusplus
extern "C"
{
#endif

void rte_wx_init(void);
void rte_wx_update_last_measuremenet_timers(uint16_t measurement_type);
void rte_wx_reset_last_measuremenet_timers(uint16_t measurement_type);
int8_t rte_wx_check_weather_measurements(void);


#ifdef __cplusplus
}
#endif

#endif /* RTE_WX_H_ */
