/*
 * config_data_default.c
 *
 *  Created on: Apr 27, 2021
 *      Author: mateusz
 */

// this file contains default configuration used if both first and second config_data section doesn't contains valid
// configuration data (CRC32 calculated from both sections is wrong). In such case the software erases both sections and
// reprogram it from the default set stored somewhere within .code section

#include "config_data.h"

#include "station_config.h"


#ifndef _RTU_SLAVE_LENGHT_1
	#define _RTU_SLAVE_LENGHT_1 0x1
#endif

#ifndef _RTU_SLAVE_LENGHT_2
	#define _RTU_SLAVE_LENGHT_2 0x1
#endif

#ifndef _RTU_SLAVE_LENGHT_3
	#define _RTU_SLAVE_LENGHT_3 0x1
#endif

#ifndef _RTU_SLAVE_LENGHT_4
	#define _RTU_SLAVE_LENGHT_4 0x1
#endif

#ifndef _RTU_SLAVE_LENGHT_5
	#define _RTU_SLAVE_LENGHT_5 0x1
#endif

#ifndef _RTU_SLAVE_LENGHT_6
	#define _RTU_SLAVE_LENGHT_6 0x1
#endif


/**
 *
 */
const config_data_mode_t __attribute__((section(".config_section_default.mode"))) config_data_mode_default = {
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

		.wx_pt_sensor = 0x71,		// TODO:

#ifdef _DUST_SDS011_SERIAL
		.wx_dust_sensor = WX_DUST_SDS011_SERIAL,
#endif

#ifdef _VICTRON
		.victron = 1,
#else
		.victron = 0,
#endif

#ifdef _SENSOR_BME280
		.wx_ms5611_or_bme = 1,
#else
		.wx_ms5611_or_bme = 0,
#endif

		.wx_anemometer_pulses_constant = _ANEMOMETER_PULSES_IN_10SEC_PER_ONE_MS_OF_WINDSPEED,

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
		.digi_viscous_delay_sec = _DIGI_VISCOUS_DEALY,
#else
		.digi_viscous_delay_sec = 3,
#endif

		.digi_delay_100msec = _DELAY_BASE,

#if (defined _POWERSAVE_NORMAL)
		.powersave = PWSAVE_NORMAL,
#elif (defined _POWERSAVE_AGGRESIVE)
		.powersave = PWSAVE_AGGRESV,
#else
		.powersave = PWSAVE_NONE,
#endif

#if (defined _GSM)
		.gsm = 1,
#else
		.gsm = 0,
#endif

#if (defined _GSM_KEEP_MODEM_ALWAYS_ON)
		.powersave_keep_gsm_always_enabled = 1
#else
		.powersave_keep_gsm_always_enabled = 0
#endif
};

/**
 *
 */
const config_data_basic_t __attribute__((section(".config_section_default.basic"))) config_data_basic_default = {
		.callsign = _CALL,
		.ssid = _SSID,
		.latitude = _LAT,
		.longitude = _LON,
#if (_LATNS == 'N')
		.n_or_s = 'N',
#else
		.n_or_s = 'S',
#endif

#if (_LONWE == 'E')
		.e_or_w = 'E',
#else
		.e_or_w = 'W',
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
#ifdef _SYMBOL_SAILBOAT
		.symbol = 5,
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
		.beacon_at_bootup = 1,
#else
		.beacon_at_bootup = 0,
#endif

#ifdef _WX_DOUBLE_TRANSMIT
		.wx_double_transmit = 1,
#else
		.wx_double_transmit = 0,
#endif

#ifdef ENG1
		.engineering1 = ENG1
#else
		.engineering1 = 0xFF
#endif
};

/**
 * Data sources for different parameters
 *
 */
const config_data_wx_sources_t __attribute__((section(".config_section_default.sources"))) config_data_wx_sources_default = {

		.temperature_telemetry = WX_SOURCE_INTERNAL,

#ifdef _TEMPERATURE_INTERNAL
		.temperature = WX_SOURCE_INTERNAL,
#endif
#ifdef _TEMPERATURE_UMB
		.temperature = WX_SOURCE_UMB,
#endif
#ifdef _TEMPERATURE_RTU
		.temperature = WX_SOURCE_RTU,
#endif
#ifdef _TEMPERATURE_DAVIS
		.temperature = WX_SOURCE_DAVIS_SERIAL,
#endif
#ifdef _TEMPERATURE_INTERNAL_PT100
		.temperature = WX_SOURCE_INTERNAL_PT100,
#endif


#ifdef _PRESSURE_INTERNAL
		.pressure = WX_SOURCE_INTERNAL,
#endif
#ifdef _PRESSURE_UMB
		.pressure = WX_SOURCE_UMB,
#endif
#ifdef _PRESSURE_RTU
		.pressure = WX_SOURCE_RTU,
#endif
#ifdef _PRESSURE_DAVIS
		.pressure = WX_SOURCE_DAVIS_SERIAL,
#endif


#ifdef _HUMIDITY_INTERNAL
		.humidity = WX_SOURCE_INTERNAL,
#endif
#ifdef _HUMIDITY_UMB
		.humidity = WX_SOURCE_UMB,
#endif
#ifdef _HUMIDITY_RTU
		.humidity = WX_SOURCE_RTU,
#endif
#ifdef _HUMIDITY_DAVIS
		.humidity = WX_SOURCE_DAVIS_SERIAL,
#endif



#ifdef _WIND_INTERNAL
		.wind = WX_SOURCE_INTERNAL
#endif
#ifdef _WIND_UMB
		.wind = WX_SOURCE_UMB
#endif
#ifdef _WIND_RTU
		.wind = WX_SOURCE_RTU
#endif
#ifdef _WIND_FULL_RTU
		.wind = WX_SOURCE_FULL_RTU
#endif
#ifdef _WIND_DAVIS
		.wind = WX_SOURCE_DAVIS_SERIAL
#endif
};

/**
 *
 */
const config_data_umb_t __attribute__((section(".config_section_default.umb"))) config_data_umb_default = {
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
		.channel_qnh = _UMB_CHANNEL_QFE
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
const config_data_rtu_t __attribute__((section(".config_section_default.rtu"))) config_data_rtu_default = {
		.slave_speed = _RTU_SLAVE_SPEED,

		.slave_parity = _RTU_SLAVE_PARITY,

		.slave_stop_bits = _RTU_SLAVE_STOP_BITS,

#ifdef _RTU_SLAVE_FULL_WIND_DATA
		.use_full_wind_data = 1,
#else
		.use_full_wind_data = 0,
#endif

		// sources
#ifdef _RTU_SLAVE_TEMPERATURE_SOURCE
		.temperature_source = _RTU_SLAVE_TEMPERATURE_SOURCE,
#else
		.temperature_source = 0,
#endif

#ifdef _RTU_SLAVE_HUMIDITY_SOURCE
		.humidity_source = _RTU_SLAVE_HUMIDITY_SOURCE,
#else
		.humidity_source = 0,
#endif

#ifdef _RTU_SLAVE_PRESSURE_SOURCE
		.pressure_source = _RTU_SLAVE_PRESSURE_SOURCE,
#else
		.pressure_source = 0,
#endif

#ifdef _RTU_SLAVE_WIND_DIRECTION_SORUCE
		.wind_direction_source = _RTU_SLAVE_WIND_DIRECTION_SORUCE,
#else
		.wind_direction_source = 0,
#endif

#ifdef _RTU_SLAVE_WIND_SPEED_SOURCE
		.wind_speed_source = _RTU_SLAVE_WIND_SPEED_SOURCE,
#else
		.wind_speed_source = 0,
#endif

#ifdef _RTU_SLAVE_WIND_GUSTS_SOURCE
		.wind_gusts_source = _RTU_SLAVE_WIND_GUSTS_SOURCE,
#else
		.wind_gusts_source = 0,
#endif

		// channel 1
		.slave_1_bus_address = _RTU_SLAVE_ID_1,

		.slave_1_function = _RTU_SLAVE_FUNC_1,

		.slave_1_register_address = _RTU_SLAVE_ADDR_1,

		.slave_1_lenght = _RTU_SLAVE_LENGHT_1,

		.slave_1_scaling_a = _RTU_SLAVE_SCALING_A_1,

		.slave_1_scaling_b = _RTU_SLAVE_SCALING_B_1,

		.slave_1_scaling_c = _RTU_SLAVE_SCALING_C_1,

		.slave_1_scaling_d = _RTU_SLAVE_SCALING_D_1,

		.slave_1_unsigned_signed = 0,		// 0 - unsigned

		// channel 2
		.slave_2_bus_address = _RTU_SLAVE_ID_2,

		.slave_2_function = _RTU_SLAVE_FUNC_2,

		.slave_2_register_address = _RTU_SLAVE_ADDR_2,

		.slave_2_lenght = _RTU_SLAVE_LENGHT_2,

		.slave_2_scaling_a = _RTU_SLAVE_SCALING_A_2,

		.slave_2_scaling_b = _RTU_SLAVE_SCALING_B_2,

		.slave_2_scaling_c = _RTU_SLAVE_SCALING_C_2,

		.slave_2_scaling_d = _RTU_SLAVE_SCALING_D_2,

		.slave_2_unsigned_signed = 0,

		// channel 3
		.slave_3_bus_address = _RTU_SLAVE_ID_3,

		.slave_3_function = _RTU_SLAVE_FUNC_3,

		.slave_3_register_address = _RTU_SLAVE_ADDR_3,

		.slave_3_lenght = _RTU_SLAVE_LENGHT_3,

		.slave_3_scaling_a = _RTU_SLAVE_SCALING_A_3,

		.slave_3_scaling_b = _RTU_SLAVE_SCALING_B_3,

		.slave_3_scaling_c = _RTU_SLAVE_SCALING_C_3,

		.slave_3_scaling_d = _RTU_SLAVE_SCALING_D_3,

		.slave_3_unsigned_signed = 0,

		// channel 4
		.slave_4_bus_address = _RTU_SLAVE_ID_4,

		.slave_4_function = _RTU_SLAVE_FUNC_4,

		.slave_4_register_address = _RTU_SLAVE_ADDR_4,

		.slave_4_lenght = _RTU_SLAVE_LENGHT_4,

		.slave_4_scaling_a = _RTU_SLAVE_SCALING_A_4,

		.slave_4_scaling_b = _RTU_SLAVE_SCALING_B_4,

		.slave_4_scaling_c = _RTU_SLAVE_SCALING_C_4,

		.slave_4_scaling_d = _RTU_SLAVE_SCALING_D_4,

		.slave_4_unsigned_signed = 0,

		// channel 5
		.slave_5_bus_address = _RTU_SLAVE_ID_5,

		.slave_5_function = _RTU_SLAVE_FUNC_5,

		.slave_5_register_address = _RTU_SLAVE_ADDR_5,

		.slave_5_lenght = _RTU_SLAVE_LENGHT_5,

		.slave_5_scaling_a = _RTU_SLAVE_SCALING_A_5,

		.slave_5_scaling_b = _RTU_SLAVE_SCALING_B_5,

		.slave_5_scaling_c = _RTU_SLAVE_SCALING_C_5,

		.slave_5_scaling_d = _RTU_SLAVE_SCALING_D_5,

		.slave_5_unsigned_signed = 0,

		// channel 6
		.slave_6_bus_address = _RTU_SLAVE_ID_6,

		.slave_6_function = _RTU_SLAVE_FUNC_6,

		.slave_6_register_address = _RTU_SLAVE_ADDR_6,

		.slave_6_lenght = _RTU_SLAVE_LENGHT_6,

		.slave_6_scaling_a = _RTU_SLAVE_SCALING_A_6,

		.slave_6_scaling_b = _RTU_SLAVE_SCALING_B_6,

		.slave_6_scaling_c = _RTU_SLAVE_SCALING_C_6,

		.slave_6_scaling_d = _RTU_SLAVE_SCALING_D_6,

		.slave_6_unsigned_signed = 0
};

#ifdef PARAMETEO

const config_data_gsm_t __attribute__((section(".config_section_default.gsm"))) config_data_gsm_default = {
		.pin = "\0\0\0\0\0",

		.apn = _GSM_APN_NAME,				// PlusGSM  - abonament
		//.apn = "plus\0",				// PlusGSM  - karta
		//.apn = "virgin-internet\0",			// Virgin Mobile

		.username = _GSM_APN_USER,

		.password = _GSM_APN_PASS,

#ifdef _GSM_API_ENABLE
		.api_enable = 1,
#else
		.api_enable = 0,
#endif

		// 78.88.56.14
		//.api_base_url = "http://78.88.56.14:8080/",
		.api_base_url = _GSM_API_BASE_URL,		// 22910
		//.api_base_url = "http://193.33.111.22:8080/meteo_backend",

		.api_station_name = _GSM_API_STATION_NAME,

#ifdef _GSM_APRSIS_ENABLE
		.aprsis_enable = 1,
#else
		.aprsis_enable = 0,
#endif

		.aprsis_passcode = _GSM_APRSIS_PASSCODE,

		.aprsis_server_port = _GSM_APRSIS_PORT,

		.aprsis_server_address = _GSM_APRSIS_ADDRES
};
#endif


