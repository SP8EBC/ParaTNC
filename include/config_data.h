/*
 * config_data.h
 *
 *  Created on: Jan 12, 2021
 *      Author: mateusz
 */

#ifndef CONFIG_DATA_H_
#define CONFIG_DATA_H_

#include <stdint.h>

/**
 * This is NOT an editable configuration file where ParaTNC settings are made! Do not touch this!
 * Please look at 'station_config.h' instead to set all parameters.
 */

typedef struct config_data_mode_t {

	uint8_t digi;

	uint8_t wx;

	uint8_t victron;



} config_data_mode_t;

typedef struct config_data_basic_t {

	char * callsign;

	uint8_t ssid;

	float latitude;

	// N or S
	uint8_t zero_to_n_one_to_s;

	float longitude;

	// E or W
	uint8_t zero_to_e_one_to_w;

	char * comment;

	// 0 - _SYMBOL_DIGI			// uncomment if you want digi symbol(green star with D inside)
	// 1 - _SYMBOL_WIDE1_DIGI	// uncomment if you want 'little' digi symbol (green star with digit 1 overlaid)
	// 2 - _SYMBOL_HOUSE			// uncomment if you want house symbol
	// 3 - _SYMBOL_RXIGATE		// uncomment if you want rxigate symbol (black diamond with R)
	// 4 - _SYMBOL_IGATE			// uncomment if you want igate symol (black diamond with I)
	uint8_t symbol;

	// 0 - no path
	// 1 - WIDE1-1 path
	// 2 - WIDE2-1 path
	uint8_t path_type;

	uint8_t beacon_at_bootup;

	uint8_t wx_transmit_period;

	uint8_t beacon_transmit_period;

} config_data_basic_t;

extern const config_data_basic_t config_data_basic;

#endif /* CONFIG_DATA_H_ */
