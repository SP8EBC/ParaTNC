/*
 * config_data.c
 *
 *  Created on: Jan 12, 2021
 *      Author: mateusz
 */

/**
 * This is NOT an editable configuration file where ParaTNC settings are made! Do not touch this!
 * Please look at 'station_config.h' instead to set all parameters.
 */

#include "config_data.h"

#include "station_config.h"


const config_data_basic_t config_data_basic = {
		.callsign = _CALL,
		.ssid = _SSID,
		.latitude = _LAT,
		.longitude = _LON,
#if (_LATNS == 'N')
		.zero_to_n_one_to_s = 0,
#else
		.zero_to_n_one_to_s = 1,
#endif

#if (_LONWE == 'E')
		.zero_to_e_one_to_w = 0,
#else
		.zero_to_e_one_to_w = 1,
#endif

		.comment = _COMMENT,

#ifdef _SYMBOL_DIGI
		.symbol = 0,
#endif
#ifdef _SYMBOL_WIDE1_DIGI
		.symbol = 1,
#endif
#ifdef _SYMBOL_HOUSE
		.symbol = 2,
#endif
#ifdef _SYMBOL_RXIGATE
		.symbol = 3,
#endif
#ifdef _SYMBOL_IGATE
		.symbol = 4,
#endif

#if defined(_WIDE1_PATH)
		.path_type = 1,
#elif defined(_WIDE21_PATH)
		.path_type = 2,
#else
		.path_type = 0,
#endif

		.wx_transmit_period = _WX_INTERVAL,

		.beacon_transmit_period = _BCN_INTERVAL,

#ifdef _BCN_ON_STARTUP
		.beacon_at_bootup = 1
#else
		.beacon_at_bootup = 0
#endif

};
