/*
 * rte_wx.h
 *
 *  Created on: 26.01.2019
 *      Author: mateusz
 */

#ifndef RTE_WX_H_
#define RTE_WX_H_

#include "station_config.h"

#include "drivers/_dht22.h"
#include "drivers/dallas.h"
#include "drivers/analog_anemometer.h"
#include "davis_vantage/davis_qf_t.h"
#include "davis_vantage/davis_loop_t.h"
#include "umb_master/umb_master.h"
#include "umb_master/umb_qf_t.h"


#ifdef _SENSOR_MS5611
#include "drivers/ms5611.h"
#endif

#ifdef _SENSOR_BME280
#include "drivers/bme280.h"
#endif

#ifdef _MODBUS_RTU
#include "modbus_rtu/rtu_configuration.h"
#include "modbus_rtu/rtu_register_data_t.h"
#include "modbus_rtu/rtu_exception_t.h"
#include "modbus_rtu/rtu_getters.h"
#include "modbus_rtu/rtu_pool_queue_t.h"
#endif


#define WIND_AVERAGE_LEN 18

#define PRESSURE_AVERAGE_LN 4

#define RTE_WX_MEASUREMENT_WIND 		460
#define RTE_WX_MEASUREMENT_TEMPERATURE	100
#define RTE_WX_MEASUREMENT_PRESSUERE	300

extern float rte_wx_temperature_dallas, rte_wx_temperature_dallas_valid;
extern float rte_wx_temperature_dalls_slew_rate;
extern float rte_wx_temperature_average_dallas_valid;
extern float rte_wx_temperature_min_dallas_valid, rte_wx_temperature_max_dallas_valid;
extern float rte_wx_temperature_ms, rte_wx_temperature_ms_valid;
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

extern dht22Values rte_wx_dht, rte_wx_dht_valid;
extern dallas_qf_t rte_wx_current_dallas_qf, rte_wx_error_dallas_qf;
extern dallas_average_t rte_wx_dallas_average;
#ifdef _SENSOR_MS5611
extern ms5611_qf_t rte_wx_ms5611_qf;
#endif
#ifdef _SENSOR_BME280
extern bme280_qf_t rte_wx_bme280_qf;
#endif
extern analog_wind_qf_t rte_wx_wind_qf;


#ifdef _UMB_MASTER

extern umb_frame_t rte_wx_umb;
extern umb_context_t rte_wx_umb_context;
extern uint8_t rte_wx_umb_last_status;
extern int16_t rte_wx_umb_channel_values[UMB_CHANNELS_STORAGE_CAPAC][2];
															// stores the value in 0.1 incremenets
#endif
extern umb_qf_t rte_wx_umb_qf;

extern uint8_t rte_wx_davis_station_avaliable;
extern uint8_t rte_wx_davis_loop_packet_avaliable;
extern davis_loop_t rte_wx_davis_loop_content;

#ifdef _MODBUS_RTU

	#if defined(_RTU_SLAVE_ID_1) && (_RTU_SLAVE_FUNC_1 == 0x03 || _RTU_SLAVE_FUNC_1 == 0x04)
		extern rtu_register_data_t RTU_GETTERS_F1_NAME;
	#endif

	#if defined(_RTU_SLAVE_ID_2) && (_RTU_SLAVE_FUNC_2 == 0x03 || _RTU_SLAVE_FUNC_2 == 0x04)
		extern rtu_register_data_t RTU_GETTERS_F2_NAME;
	#endif

	#if defined(_RTU_SLAVE_ID_3) && (_RTU_SLAVE_FUNC_3 == 0x03 || _RTU_SLAVE_FUNC_3 == 0x04)
		extern rtu_register_data_t RTU_GETTERS_F3_NAME;
	#endif

	#if defined(_RTU_SLAVE_ID_4) && (_RTU_SLAVE_FUNC_4 == 0x03 || _RTU_SLAVE_FUNC_4 == 0x04)
		extern rtu_register_data_t RTU_GETTERS_F4_NAME;
	#endif

	#if defined(_RTU_SLAVE_ID_5) && (_RTU_SLAVE_FUNC_5 == 0x03 || _RTU_SLAVE_FUNC_5 == 0x04)
		extern rtu_register_data_t RTU_GETTERS_F5_NAME;
	#endif

	#if defined(_RTU_SLAVE_ID_6) && (_RTU_SLAVE_FUNC_6 == 0x03 || _RTU_SLAVE_FUNC_6 == 0x04)
		extern rtu_register_data_t RTU_GETTERS_F6_NAME;
	#endif

extern rtu_exception_t rte_wx_last_modbus_exception;
extern uint32_t rte_wx_last_modbus_rx_error_timestamp;
extern uint32_t rte_wx_last_modbus_exception_timestamp;
extern rtu_pool_queue_t rte_wx_rtu_pool_queue;

#endif

#ifdef __cplusplus
extern "C"
{
#endif

void rte_wx_init(void);
void rte_wx_update_last_measuremenet_timers(uint16_t measurement_type);

#ifdef __cplusplus
}
#endif

#endif /* RTE_WX_H_ */
