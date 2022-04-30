/*
 * api_status_content.h
 *
 *  Created on: Apr 24, 2022
 *      Author: mateusz
 */

#ifndef API_STATUS_CONTENT_H_
#define API_STATUS_CONTENT_H_

#include "api_xmacro_helpers.h"

#include "main.h"
#include "rte_main.h"
#include "rte_wx.h"

#define ENTRIES_STRING(ENTRY)			\
	ENTRY(main_config_data_basic->callsign, callsign)	\

#define ENTRIES_32INT_STATUS(ENTRY)							\
	ENTRY(master_time, master_time)						\
	ENTRY(rte_main_rx_total, rx_total)			\
	ENTRY(rte_main_tx_total, tx_total)			\
	ENTRY(rte_main_last_sleep_master_time, last_sleep_master_time)	\

#define ENTRIES_16INT_STATUS(ENTRY)			\
	ENTRY(main_cpu_load, cpu_load)					\
	ENTRY(rx10m, rx_10m)									\
	ENTRY(tx10m, tx_10m)									\
	ENTRY(digi10m, digi_10m)								\
	ENTRY(digidrop10m, digi_drop_10m)						\
	ENTRY(kiss10m, kiss_10m)								\
	ENTRY(rte_main_average_battery_voltage, average_battery_voltage)		\
	ENTRY(rte_main_wakeup_count, wakeup_count)	\
	ENTRY(rte_main_going_sleep_count, going_sleep_count)	\
	ENTRY(main_config_data_basic->ssid, ssid)	\


#define ENTRIES_16INT_WEATHER(ENTRY)							\
	ENTRY(rte_wx_temperature_average_dallas, temperature_dallas)	\
	ENTRY(rte_wx_temperature_average_pt, temperature_pt)			\
	ENTRY(rte_wx_temperature_average_internal, temperature_internal)\
	ENTRY(rte_wx_pressure_average, pressure)						\
	ENTRY(rte_wx_average_winddirection, wind_direction)				\
	ENTRY(rte_wx_average_windspeed, wind_speed)					\
	ENTRY(rte_wx_max_windspeed, wind_gust)					\

#endif /* API_STATUS_CONTENT_H_ */

/**

 */
