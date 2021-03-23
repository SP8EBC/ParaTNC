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

/**
 *
 */
const config_data_mode_t config_data_mode = {
#ifdef _DIGI
		.digi = 1,
#else
		.digi = 0,
#endif

#ifdef _METEO
		.wx = 1,
#else
		.wx = 0,
#endif

#ifdef _MODBUS_RTU
		.wx_modbus = 1,
#else
		.wx_modbus = 0,
#endif

#ifdef _MODBUS_RTU
		.wx_modbus = 1,
#else
		.wx_modbus = 0,
#endif

#ifdef _UMB_MASTER
		.wx_umb = 1,
#else
		.wx_umb = 0,
#endif


#ifdef _VICTRON
		.victron = 1,
#else
		.victron = 0,
#endif



#ifdef _DIGI_ONLY_789
		.digi_only_ssids = 1,
#else
		.digi_only_ssids = 0,
#endif

#ifdef _DIGI_VISCOUS
		.digi_viscous = 1,
#else
		.digi_viscous = 0,
#endif

#ifdef _DIGI_VISCOUS_DEALY
		.digi_viscous_delay_sec = _DIGI_VISCOUS_DEALY
#else
		.digi_viscous_delay_sec = 3
#endif
};

/**
 *
 */
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

/**
 *
 */
const config_data_umb_t config_data_umb = {
#ifdef _UMB_SLAVE_ID
		.slave_id = _UMB_SLAVE_ID,
#else
		.slave_id = 0,
#endif

#ifdef _UMB_SLAVE_CLASS
		.slave_class = _UMB_SLAVE_CLASS,
#else
		.slave_class = 0,
#endif

#if defined (_UMB_SLAVE_ID) && defined (_UMB_SLAVE_CLASS)
		.channel_windspeed = _UMB_CHANNEL_WINDSPEED,
		.channel_wingsusts = _UMB_CHANNEL_WINDGUSTS,
		.channel_winddirection = _UMB_CHANNEL_WINDDIRECTION,
		.channel_temperature = _UMB_CHANNEL_TEMPERATURE,
		.channel_qfe = _UMB_CHANNEL_QFE
#else
		.channel_windspeed = 0xFFFF,
		.channel_wingsusts = 0xFFFF,
		.channel_winddirection = 0xFFFF,
		.channel_temperature = 0xFFFF,
		.channel_qfe = 0xFFFF
#endif
};


/**
 *
 */
const config_data_rtu_t config_data_rtu = {
		.slave_speed;

		.slave_parity;

		.slave_stop_bits;

		.use_full_wind_data;

		// sources
		.temperature_source;

		.humidity_source;

		.pressure_source;

		.wind_direction_source;

		.wind_speed_source;

		// channel 1
		.slave_1_bus_address;

		.slave_1_function;

		.slave_1_register_address;

		.slave_1_lenght;

		.slave_1_scaling_a;

		.slave_1_scaling_b;

		.slave_1_scaling_c;

		.slave_1_scaling_d;

		.slave_1_unsigned_signed;

		// channel 2
		.slave_2_bus_address;

		.slave_2_function;

		.slave_2_register_address;

		.slave_2_lenght;

		.slave_2_scaling_a;

		.slave_2_scaling_b;

		.slave_2_scaling_c;

		.slave_2_scaling_d;

		.slave_2_unsigned_signed;

		// channel 3
		.slave_3_bus_address;

		.slave_3_function;

		.slave_3_register_address;

		.slave_3_lenght;

		.slave_3_scaling_a;

		.slave_3_scaling_b;

		.slave_3_scaling_c;

		.slave_3_scaling_d;

		.slave_3_unsigned_signed;

		// channel 4
		.slave_4_bus_address;

		.slave_4_function;

		.slave_4_register_address;

		.slave_4_lenght;

		.slave_4_scaling_a;

		.slave_4_scaling_b;

		.slave_4_scaling_c;

		.slave_4_scaling_d;

		.slave_4_unsigned_signed;

		// channel 5
		.slave_5_bus_address;

		.slave_5_function;

		.slave_5_register_address;

		.slave_5_lenght;

		.slave_5_scaling_a;

		.slave_5_scaling_b;

		.slave_5_scaling_c;

		.slave_5_scaling_d;

		.slave_5_unsigned_signed;

		// channel 6
		.slave_6_bus_address;

		.slave_6_function;

		.slave_6_register_address;

		.slave_6_lenght;

		.slave_6_scaling_a;

		.slave_6_scaling_b;

		.slave_6_scaling_c;

		.slave_6_scaling_d;
};
