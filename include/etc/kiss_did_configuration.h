/*
 * kiss_did_configuration.h
 *
 *	Configuration of all DIDs with theirs data source. There are separate
 *	definitions for DIDs which return strings and numeric values
 *
 *
 *	DID subsystem consist of three files:
 *		kiss_xmacro_helpers.h - files with macros used for expanding config
 *		kiss_did_configuration.h - definitions which DID return what data
 *		kiss_did.c - implementation of arrays with content definition and
 *					 function which are responsible for returning raw
 *					 binary data basing on configuration
 *
 *  Created on: Jun 21, 2023
 *      Author: mateusz
 */

#ifndef KISS_DID_CONFIGURATION_H_
#define KISS_DID_CONFIGURATION_H_

#include "kiss_communication/types/kiss_xmacro_helpers.h"
#include "rte_main.h"
#include "rte_wx.h"
#include "rte_rtu.h"
#include "main_master_time.h"
#include "main.h"
#include "aprsis.h"
#include "gsm/sim800c.h"
#include "packet_tx_handler.h"
#include "nvm/nvm_event_externs.h"
#include <stored_configuration_nvm/config_data_externs.h>
#include "software_version.h"
#include "memory_map.h"
#include "drivers/max31865.h"

//!< Dummy variable used only as end of definition marker in tables
extern char did_dummy_data;

//!< Definition of all DIDs with theirs source data for PARAMETEO platform
#if defined(PARAMETEO)
	#define DIDS_STRING(ENTRY)		\
		ENTRY(0x1500U, gsm_sim800_registered_network)		\
		ENTRY(0x1501U, gsm_sim800_simcard_status_string)		\
		ENTRY(0x1502U, gsm_sim800_cellid)		\
		ENTRY(0x1503U, gsm_sim800_lac)		\
		ENTRY(0x5555U, main_test_string)		\

	#define DIDS_FLOAT(ENTRY)	\
		ENTRY(0x2000U, rte_wx_temperature_average_external_valid, DID_EMPTY, DID_EMPTY)	\
		ENTRY(0x2001U, rte_wx_temperature_internal_valid, DID_EMPTY, DID_EMPTY)	\
		ENTRY(0x2002U, rte_wx_pressure_history[0], rte_wx_pressure_history[1], rte_wx_pressure_history[2])	\
		ENTRY(0x1505U, gsm_sim800_signal_level_dbm, DID_EMPTY, DID_EMPTY)	\
		ENTRY(0x2222U, main_test_float, DID_EMPTY, DID_EMPTY)	\

	#define DIDS_NUMERIC(ENTRY)		\
		ENTRY(0x1000U, master_time, DID_EMPTY, DID_EMPTY)	\
		ENTRY(0x1001U, rx10m, DID_EMPTY, DID_EMPTY)	\
		ENTRY(0x1002U, tx10m, DID_EMPTY, DID_EMPTY)	\
		ENTRY(0x1003U, digi10m, DID_EMPTY, DID_EMPTY)	\
		ENTRY(0x1004U, digidrop10m, DID_EMPTY, DID_EMPTY)	\
		ENTRY(0x1100U, rte_main_battery_voltage, rte_main_average_battery_voltage, DID_EMPTY)	\
		ENTRY(0x2003U, rte_wx_temperature_average_dallas, rte_wx_temperature_average_pt, rte_wx_temperature_average_internal)	\
		ENTRY(0x2004U, rte_wx_average_winddirection, rte_wx_average_windspeed, rte_wx_max_windspeed)	\
		ENTRY(0x2005U, rte_wx_windspeed[0], rte_wx_windspeed[1], rte_wx_windspeed[2])	\
		ENTRY(0x2006U, rte_wx_winddirection[0], rte_wx_winddirection[1], rte_wx_winddirection[2])	\
		ENTRY(0x2007U, rte_wx_humidity, DID_EMPTY, DID_EMPTY)	\
		ENTRY(0x2008U, rte_wx_humidity, DID_EMPTY, DID_EMPTY)	\
		ENTRY(0x2009U, rte_wx_analog_anemometer_counter_timer_has_been_fired, rte_wx_analog_anemometer_counter_slew_limit_fired, rte_wx_analog_anemometer_counter_direction_doesnt_work)	\
		ENTRY(0x2010U, rte_wx_umb_context.current_routine, rte_wx_umb_context.current_channel, rte_wx_umb_context.state)	\
		ENTRY(0x2011U, rte_wx_umb_context.time_of_last_nok, rte_wx_umb_context.time_of_last_comms_timeout, rte_wx_umb_context.time_of_last_successful_comms)	\
		ENTRY(0x2012U, rte_wx_umb_channel_values[0][0], rte_wx_umb_channel_values[0][1], DID_EMPTY)	\
		ENTRY(0x2013U, rte_wx_umb_channel_values[1][0], rte_wx_umb_channel_values[1][1], DID_EMPTY)	\
		ENTRY(0x2014U, rte_wx_umb_channel_values[2][0], rte_wx_umb_channel_values[2][1], DID_EMPTY)	\
		ENTRY(0x2015U, rte_wx_umb_channel_values[3][0], rte_wx_umb_channel_values[3][1], DID_EMPTY)	\
		ENTRY(0x2020U, rte_rtu_number_of_serial_io_errors, rte_rtu_number_of_successfull_serial_comm, DID_EMPTY)	\
		ENTRY(0x2021U, rte_rtu_last_modbus_rx_error_timestamp, rte_rtu_last_modbus_exception_timestamp, DID_EMPTY)	\
		ENTRY(0x2022U, rte_wx_modbus_rtu_f1.base_address, rte_wx_modbus_rtu_f1.registers_values[0], rte_wx_modbus_rtu_f1.registers_values[1])	\
		ENTRY(0x2023U, rte_wx_modbus_rtu_f2.base_address, rte_wx_modbus_rtu_f2.registers_values[0], rte_wx_modbus_rtu_f2.registers_values[1])	\
		ENTRY(0x2024U, rte_wx_modbus_rtu_f3.base_address, rte_wx_modbus_rtu_f3.registers_values[0], rte_wx_modbus_rtu_f3.registers_values[1])	\
		ENTRY(0x2025U, rte_wx_modbus_rtu_f4.base_address, rte_wx_modbus_rtu_f4.registers_values[0], rte_wx_modbus_rtu_f4.registers_values[1])	\
		ENTRY(0x2100U, aprsis_logged, aprsis_connected, aprsis_unsucessfull_conn_counter)	\
		ENTRY(0x2200U, packet_tx_beacon_counter, packet_tx_meteo_counter, packet_tx_telemetry_counter)	\
		ENTRY(0x2201U, packet_tx_beacon_interval, packet_tx_meteo_interval, packet_tx_telemetry_interval)	\
		ENTRY(0x3000U, max31865_raw_result, max31865_physical_result, max31865_current_fault_status)	\
		ENTRY(0x1504U, gsm_sim800_signal_level_dbm, DID_EMPTY, DID_EMPTY)	\
		ENTRY(0xF000U, config_running_pgm_counter, DID_EMPTY, DID_EMPTY)	\
		ENTRY(0xFF00U, main_flash_log_start, main_flash_log_end, DID_EMPTY)	\
		ENTRY(0xFF0FU, nvm_event_oldestFlash, nvm_event_newestFlash, DID_EMPTY)	\
		ENTRY(0xFFFFU, did_dummy_data, did_dummy_data, did_dummy_data)
#endif

//!< Definition of all DIDs with theirs source data for PARATNC platform
#if defined (PARATNC)
	#define DIDS_STRING(ENTRY)		\
		ENTRY(0xFD01U, software_version_str)		\

	#define DIDS_FLOAT(ENTRY)	\
		ENTRY(0x2000U, rte_wx_temperature_average_external_valid, DID_EMPTY, DID_EMPTY)	\
		ENTRY(0x2001U, rte_wx_temperature_internal_valid, DID_EMPTY, DID_EMPTY)	\
		ENTRY(0x2002U, rte_wx_pressure_history[0], rte_wx_pressure_history[1], rte_wx_pressure_history[2])	\

	#define DIDS_NUMERIC(ENTRY)		\
		ENTRY(0x1000U, master_time, DID_EMPTY, DID_EMPTY)	\
		ENTRY(0x1001U, rx10m, DID_EMPTY, DID_EMPTY)	\
		ENTRY(0x1002U, tx10m, DID_EMPTY, DID_EMPTY)	\
		ENTRY(0x1003U, digi10m, DID_EMPTY, DID_EMPTY)	\
		ENTRY(0x1004U, digidrop10m, DID_EMPTY, DID_EMPTY)	\
		ENTRY(0x2004U, rte_wx_average_winddirection, rte_wx_average_windspeed, rte_wx_max_windspeed)	\
		ENTRY(0x2005U, rte_wx_windspeed[0], rte_wx_windspeed[1], rte_wx_windspeed[2])	\
		ENTRY(0x2006U, rte_wx_winddirection[0], rte_wx_winddirection[1], rte_wx_winddirection[2])	\
		ENTRY(0x2007U, rte_wx_humidity, DID_EMPTY, DID_EMPTY)	\
		ENTRY(0x2008U, rte_wx_humidity, DID_EMPTY, DID_EMPTY)	\
		ENTRY(0x2200U, packet_tx_beacon_counter, packet_tx_meteo_counter, packet_tx_telemetry_counter)	\
		ENTRY(0x2201U, packet_tx_beacon_interval, packet_tx_meteo_interval, packet_tx_telemetry_interval)	\
		ENTRY(0xFFFFU, did_dummy_data, did_dummy_data, did_dummy_data)	\

#endif

#endif /* KISS_DID_CONFIGURATION_H_ */
