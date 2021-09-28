/*
 * rte_wx.h
 *
 *  Created on: 26.01.2019
 *      Author: mateusz
 */

#ifndef RTE_WX_H_
#define RTE_WX_H_

#include "station_config.h"

#include "drivers/dallas.h"
#include "drivers/analog_anemometer.h"
#include "davis_vantage/davis_qf_t.h"
#include "davis_vantage/davis_loop_t.h"
#include "umb_master/umb_master.h"
#include "umb_master/umb_qf_t.h"


//#ifdef _SENSOR_MS5611
#include "drivers/ms5611.h"
//#endif

//#ifdef _SENSOR_BME280
#include "drivers/bme280.h"
//#endif


#define WIND_AVERAGE_LEN 18

#define PRESSURE_AVERAGE_LN 4

#define RTE_WX_MEASUREMENT_WIND 		460
#define RTE_WX_MEASUREMENT_TEMPERATURE	100
#define RTE_WX_MEASUREMENT_PRESSUERE	300

extern float rte_wx_temperature_external, rte_wx_temperature_external_valid;
extern float rte_wx_temperature_external_slew_rate;
extern float rte_wx_temperature_average_external_valid;
extern float rte_wx_temperature_min_external_valid, rte_wx_temperature_max_external_valid;
extern float rte_wx_temperature_internal, rte_wx_temperature_internal_valid;
extern float rte_wx_pressure, rte_wx_pressure_valid;
extern float rte_wx_pressure_history[PRESSURE_AVERAGE_LN];
extern uint8_t rte_wx_pressure_it;

extern uint16_t rte_wx_windspeed_pulses;
extern uint16_t rte_wx_windspeed[WIND_AVERAGE_LEN];
extern uint8_t rte_wx_windspeed_it;
extern uint16_t rte_wx_winddirection[WIND_AVERAGE_LEN];
extern uint8_t rte_wx_winddirection_it;
extern uint16_t rte_wx_winddirection_last;
extern uint16_t rte_wx_average_windspeed;
extern uint16_t rte_wx_max_windspeed;
extern int16_t rte_wx_average_winddirection;

extern int8_t rte_wx_humidity, rte_wx_humidity_valid;

extern uint8_t rte_wx_tx20_excessive_slew_rate;

extern dallas_qf_t rte_wx_current_dallas_qf, rte_wx_error_dallas_qf;
extern dallas_average_t rte_wx_dallas_average;
extern ms5611_qf_t rte_wx_ms5611_qf;
extern bme280_qf_t rte_wx_bme280_qf;
extern analog_wind_qf_t rte_wx_wind_qf;



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


#ifdef __cplusplus
}
#endif

#endif /* RTE_WX_H_ */
