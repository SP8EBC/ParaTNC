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

//!< Dummy variable used only as end of definition marker in tables
extern char did_dummy_data;

//!< Definition of all DIDs with theirs source data for PARAMETEO platform
#if defined(PARAMETEO)
	#define DIDS_NUMERIC(ENTRY)		\
		ENTRY(0x1234, &rte_wx_temperature_average_dallas, &rte_wx_temperature_average_pt, &rte_wx_temperature_average_internal)	\
		ENTRY(0xFFFF, &did_dummy_data, &did_dummy_data, &did_dummy_data)
#endif

//!< Definition of all DIDs with theirs source data for PARATNC platform
#if defined (PARATNC)
	#define DIDS_NUMERIC(ENTRY)		\
		ENTRY(0xFFFF, &did_dummy_data, &did_dummy_data, &did_dummy_data)
#endif

#endif /* KISS_DID_CONFIGURATION_H_ */
