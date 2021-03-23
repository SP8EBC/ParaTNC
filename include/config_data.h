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

	uint8_t wx_umb;

	uint8_t wx_modbus;

	uint8_t wx_davis;

	uint8_t wx_rtu;

	uint8_t victron;

	uint8_t digi_viscous;

	uint8_t digi_only_ssids;

	uint8_t digi_viscous_delay_sec;


} config_data_mode_t;

typedef struct config_data_basic_t {

	char callsign[7];

	uint8_t ssid;

	float latitude;

	// N or S
	uint8_t zero_to_n_one_to_s;

	float longitude;

	// E or W
	uint8_t zero_to_e_one_to_w;

	char comment[128];

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

typedef struct config_data_umb_t {

	uint16_t slave_class;

	uint16_t slave_id;

	uint16_t channel_windspeed;

	uint16_t channel_wingsusts;

	uint16_t channel_winddirection;

	uint16_t channel_temperature;

	uint16_t channel_qfe;
/**
 * #define _UMB_CHANNEL_WINDSPEED			460
#define _UMB_CHANNEL_WINDGUSTS			440
#define _UMB_CHANNEL_WINDDIRECTION		580
#define _UMB_CHANNEL_TEMPERATURE		100
#define _UMB_CHANNEL_QFE
 */
} config_data_umb_t;

typedef struct config_data_rtu_t {
	uint16_t slave_speed;

	uint8_t slave_parity;

	uint8_t slave_stop_bits;

	uint8_t use_full_wind_data;

	// sources
	uint8_t temperature_source;

	uint8_t humidity_source;

	uint8_t pressure_source;

	uint8_t wind_direction_source;

	uint8_t wind_speed_source;

	uint8_t wind_gusts_source;

	// channel 1
	uint8_t slave_1_bus_address;

	uint8_t slave_1_function;

	uint16_t slave_1_register_address;

	uint16_t slave_1_lenght;

	uint8_t slave_1_scaling_a;

	uint8_t slave_1_scaling_b;

	uint8_t slave_1_scaling_c;

	uint8_t slave_1_scaling_d;

	uint8_t slave_1_unsigned_signed;

	// channel 2
	uint8_t slave_2_bus_address;

	uint8_t slave_2_function;

	uint16_t slave_2_register_address;

	uint16_t slave_2_lenght;

	uint8_t slave_2_scaling_a;

	uint8_t slave_2_scaling_b;

	uint8_t slave_2_scaling_c;

	uint8_t slave_2_scaling_d;

	uint8_t slave_2_unsigned_signed;

	// channel 3
	uint8_t slave_3_bus_address;

	uint8_t slave_3_function;

	uint16_t slave_3_register_address;

	uint16_t slave_3_lenght;

	uint8_t slave_3_scaling_a;

	uint8_t slave_3_scaling_b;

	uint8_t slave_3_scaling_c;

	uint8_t slave_3_scaling_d;

	uint8_t slave_3_unsigned_signed;

	// channel 4
	uint8_t slave_4_bus_address;

	uint8_t slave_4_function;

	uint16_t slave_4_register_address;

	uint16_t slave_4_lenght;

	uint8_t slave_4_scaling_a;

	uint8_t slave_4_scaling_b;

	uint8_t slave_4_scaling_c;

	uint8_t slave_4_scaling_d;

	uint8_t slave_4_unsigned_signed;

	// channel 5
	uint8_t slave_5_bus_address;

	uint8_t slave_5_function;

	uint16_t slave_5_register_address;

	uint16_t slave_5_lenght;

	uint8_t slave_5_scaling_a;

	uint8_t slave_5_scaling_b;

	uint8_t slave_5_scaling_c;

	uint8_t slave_5_scaling_d;

	uint8_t slave_5_unsigned_signed;

	// channel 6
	uint8_t slave_6_bus_address;

	uint8_t slave_6_function;

	uint16_t slave_6_register_address;

	uint16_t slave_6_lenght;

	uint8_t slave_6_scaling_a;

	uint8_t slave_6_scaling_b;

	uint8_t slave_6_scaling_c;

	uint8_t slave_6_scaling_d;

	uint8_t slave_6_unsigned_signed;


} config_data_rtu_t;

extern const config_data_basic_t config_data_basic;
extern const config_data_mode_t config_data_mode;
extern const config_data_umb_t config_data_umb;

#endif /* CONFIG_DATA_H_ */
