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

#include "./kiss_communication/kiss_xmacro_helpers.h"
#include "rte_wx.h"
#include "main_master_time.h"
#include "main.h"
#include "rte_main.h"
#include "gsm/sim800c.h"
#include "software_version.h"

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
		ENTRY(0x1504U, gsm_sim800_signal_level_dbm, DID_EMPTY, DID_EMPTY)	\
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
		ENTRY(0xFFFFU, did_dummy_data, did_dummy_data, did_dummy_data)	\

#endif

#endif /* KISS_DID_CONFIGURATION_H_ */
