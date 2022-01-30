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

/**
 * This enum is used to configure how the ParaMETEO controller will switch to different powersave modes
 */
typedef enum config_data_powersave_mode_t {

	/**
	 *	Micro will be kept constantly in RUN mode, +5V_S, +5V_R and +4V_G will be kept on.
	 *	This is suitable if station is powered from mains and needs to be operated as APRS
	 *	digipeater and/or igate. Wind measurements will be very accurate as anemometer
	 *	readings will be collected all the time
	 */
	PWSAVE_NONE = 0,
	PWSAVE_NORMAL = 1,
	PWSAVE_AGGRESV = 3

	//PWSAVE_


}config_data_powersave_mode_t;

typedef struct config_data_mode_t {

#define WX_ENABLED 					(1)
#define WX_INTERNAL_AS_BACKUP 		(1 << 1)
#define WX_INTERNAL_SPARKFUN_WIND	(1 << 2)

#define WX_MODBUS_DEBUG				(1 << 1)

	uint8_t digi;

	uint8_t wx;

	uint8_t wx_umb;

	uint8_t wx_modbus;

	uint8_t wx_davis;

	uint8_t wx_ms5611_or_bme;		// set to one to choose bme, zero to ms5611

	uint8_t wx_anemometer_pulses_constant;		// #define _ANEMOMETER_PULSES_IN_10SEC_PER_ONE_MS_OF_WINDSPEED 10

	uint8_t victron;

	uint8_t digi_viscous;

	uint8_t digi_only_ssids;

	uint8_t digi_viscous_delay_sec;

	uint8_t digi_delay_100msec;		// in 100msec increments

	// only for ParaMETEO
	config_data_powersave_mode_t powersave;

	// only for ParaMETEO
	uint8_t gsm;

} config_data_mode_t;

typedef struct config_data_basic_t {

	char callsign[7];

	uint8_t ssid;

	float latitude;

	// N or S
	char n_or_s;

	float longitude;

	// E or W
	char e_or_w;

	char comment[128];

	// 0 - _SYMBOL_DIGI			// uncomment if you want digi symbol(green star with D inside)
	// 1 - _SYMBOL_WIDE1_DIGI	// uncomment if you want 'little' digi symbol (green star with digit 1 overlaid)
	// 2 - _SYMBOL_HOUSE			// uncomment if you want house symbol
	// 3 - _SYMBOL_RXIGATE		// uncomment if you want rxigate symbol (black diamond with R)
	// 4 - _SYMBOL_IGATE			// uncomment if you want igate symol (black diamond with I)
	// 5 - _SYMBOL_SAILBOAT
	uint8_t symbol;

	// 0 - no path
	// 1 - WIDE1-1 path
	// 2 - WIDE2-1 path
	uint8_t path_type;

	uint8_t beacon_at_bootup;

	uint8_t wx_transmit_period;

	uint8_t beacon_transmit_period;

	uint8_t wx_double_transmit;

} config_data_basic_t;

typedef enum config_data_wx_sources_enum_t {
	/**
	 * Internal sensors are:
	 * 	- Dallas DS18B20 for temperature
	 * 	- MS5611 or BME280 for pressure/humidity
	 * 	- analog/mechanical anemometer for wind
	 */
	WX_SOURCE_INTERNAL = 1,

	/**
	 * Lufft UMB devices
	 */
	WX_SOURCE_UMB = 2,

	/**
	 * RTU can be used for any measuremd parameter, but by default
	 * it is used for wind in the same way as the mechanic anemometer is.
	 * The controller asks for the windspeed and treat it as momentary value,
	 * which then is put into the circular buffer to calculate average and
	 * max from.
	 */
	WX_SOURCE_RTU = 3,

	/**
	 * This option makes a difference only for wind measurements. With all
	 * other parameters it works exactly the same as WX_SOURCE_RTU. In this mode
	 * the controller queries for average and maximum wind speed.
	 */
	WX_SOURCE_FULL_RTU = 4,
	WX_SOURCE_DAVIS_SERIAL = 5
} config_data_wx_sources_enum_t;

typedef struct config_data_wx_sources_t {

	config_data_wx_sources_enum_t temperature;
	config_data_wx_sources_enum_t pressure;
	config_data_wx_sources_enum_t humidity;
	config_data_wx_sources_enum_t wind;

} config_data_wx_sources_t;

typedef struct config_data_umb_t {

	uint16_t slave_class;

	uint16_t slave_id;

	uint16_t channel_windspeed;

	uint16_t channel_wingsusts;

	uint16_t channel_winddirection;

	uint16_t channel_temperature;

	uint16_t channel_qnh;
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

typedef struct config_data_gsm_t {

	char pin[5];

	char apn[24];

	char username[24];

	char password[24];


} config_data_gsm_t;

#endif /* CONFIG_DATA_H_ */
