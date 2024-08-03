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

typedef struct __attribute__((aligned (4))) config_data_mode_t {

#define WX_ENABLED 					(1)
#define WX_INTERNAL_AS_BACKUP 		(1 << 1)
#define WX_INTERNAL_SPARKFUN_WIND	(1 << 2)
#define WX_INTERNAL_DISABLE_DALLAS	(1 << 3)

#define WX_MODBUS_DEBUG				(1 << 1)

#define WX_DUST_SDS011_PWM			(1 << 1)
#define WX_DUST_SDS011_SERIAL		(1 << 2)

	uint8_t digi;

	uint8_t wx;

	uint8_t wx_umb;

	uint8_t wx_modbus;

	uint8_t wx_davis;

	uint8_t wx_ms5611_or_bme;		// set to one to choose bme, zero to ms5611

	uint8_t wx_anemometer_pulses_constant;		// #define _ANEMOMETER_PULSES_IN_10SEC_PER_ONE_MS_OF_WINDSPEED 10

	uint8_t wx_dust_sensor;

	/**
	 * 0x00 or 0xFF - PT sensor disabled
	 * bit0 - enabled / disabled
	 * bit1 - PT100 (1) or PT1000 (0)
	 * bit2 through bit7 - resistor value from lookup table
	 * 	 */
	uint8_t wx_pt_sensor;

	uint8_t victron;

	uint8_t digi_viscous;

	uint8_t digi_only_ssids;

	uint8_t digi_viscous_delay_sec;

	uint8_t digi_delay_100msec;		// in 100msec increments

	// only for ParaMETEO
	config_data_powersave_mode_t powersave;

	// only for ParaMETEO - keeps GSM modem always on if GSM is configured to be used
	uint8_t powersave_keep_gsm_always_enabled;

	// only for ParaMETEO
	uint8_t gsm;

	// only for ParaMETEO
	uint8_t nvm_logger;

} config_data_mode_t;

typedef struct __attribute__((aligned (4))) config_data_basic_t {

	#define ENGINEERING1						(1)
	#define ENGINEERING1_INH_WX_PWR_HNDL		(1 << 1)
	#define ENGINEERING1_EARLY_TX_ASSERT		(1 << 2)
	#define ENGINEERING1_PWRCYCLE_GSM_ON_NOCOMM	(1 << 2)

	#define ENGINEERING2					(1)
	#define ENGINEERING2_REBOOT_AFTER_24	(1 << 1)
	#define ENGINEERING2_POWER_CYCLE_R		(1 << 2)

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

	/**
	 *	bit0 - must be set to ZERO to enable this engineering
	 *	bit1 - inhibit 'wx_pwr_switch_periodic_handle'
	 *	bit2 - early_tx_assert
	 *	bit3 -
	 *	bit4 -
	 *	bit5 -
	 *	bit6 -
	 *	bit7 -
	 */
	uint8_t engineering1;

	/**
	 * Ugly and nasty workarounds of (mostly hardware) problems, which should
	 * be fixed, not inhibited by stupid workaround. Use only
	 * where there is no hope left.
	 *
	 *	bit0 - must be set to ZERO to enable this engineering
	 *  bit1 - reboot after 99 telemetry frames
	 *  bit2 - power cycle vbat_r two minutes before weather frame
	 */
	uint8_t engineering2;

	uint16_t battery_scalling_a;

	uint16_t battery_scalling_b;

	/**
	 * 	BUTTON_SEND_WX = 1,
	BUTTON_SEND_WX_INTERNET = 2,
	BUTTON_SEND_BEACON = 3,
	BUTTON_FORCE_UART_KISS = 4,
	BUTTON_FORCE_UART_LOG = 5,
	BUTTON_RESET_GSM_GPRS = 6,
		BUTTON_RECONNECT_APRSIS = 7,
	 */
	#define BUTTON_FUNCTION_SEND_WX					1U
	#define BUTTON_FUNCTION_SEND_WX_INET			2U
	#define BUTTON_FUNCTION_SEND_BEACON				3U
	#define BUTTON_FUNCTION_UART_KISS				4U
	#define BUTTON_FUNCTION_UART_LOG				5U
	#define BUTTON_FUNCION_RESET_GSM_GPRS			6U
	#define BUTTON_FUNCTION_RECONNECT_APRSIS		7U
	#define BUTTON_FUNCTION_SIMULATE_APRSIS_TIMEOUT	8U


	uint8_t button_one_left;

	uint8_t button_two_right;

	#define CONFIGURATION_SEC_ROUTINE_READ_OFFSET	1U
	#define CONFIGURATION_SEC_ROUTINE_ELSE_OFFSET	3U
	#define CONFIGURATION_SEC_MEDIUM_APRSIS	1U
	#define CONFIGURATION_SEC_MEDIUM_RADIO	2U
	#define CONFIGURATION_SEC_MEDIUM_KISS	4U
	/**
	 * Configuration of how UDS diagnostics are secured access different
	 * mediums. GET_VERSION_AND_ID and SECURITY_ACCESS are never locked.
	 * If the service shall not(!!) be locked respective bit should be set to 0.
	 * By default, when memory is fully erased everything is locked
	 *
	 * Serial Port
	 *  	0 -	Read DID
	 *  	1 - Read Memory by address (RAM2, RAM2_NOINIT, everything > FLASH)
	 *  	2 - Read Memory by address (without limit)
	 *  	3 - Restart Reset
	 *  	4 - Configuration reset
	 *		5 - Write memory by address
	 *		6 - Erase and program startup config
	 *		7 - Get running config
	 *		8 - Request file transfer and transfer data
	 *
	 *  	13 -
	 *
	 * Validity bits
	 * 		14
	 * 		15
	 * 	these bits are sum of ( (uds_diagnostics_security_access & 0x3FFF) +
	 * 	(uds_diagnostics_security_access & 3FFF0000) >> 16) & 0x3. If this value
	 * 	doesn't match a configuration from here is discarded completely and
	 * 	default settings are applied:
	 * 	1. Everything over serial port is unlocked
	 * 	2. Read DID and one restart per day
	 *
	 * APRS Message (Radio network or APRS-IS server)
	 *
	 *  	16 -
	 *  	17 -
	 *
	 *		29 -
	 *
	 * Unlock all services by default when accessed via APRSMSG_TRANSPORT_ENCRYPTED_HEXSTRING
	 *		30 - this should be zero to enable
	 *		31 - this should be one to enable
	 */
	uint32_t uds_diagnostics_security_access;

} config_data_basic_t;

typedef enum config_data_wx_sources_enum_t {
	/**
	 * Internal sensors are:
	 * 	- Dallas DS18B20 for temperature
	 * 	- MS5611 or BME280 for pressure/humidity
	 * 	- analog/mechanical anemometer for wind
	 */
	WX_SOURCE_INTERNAL = 1,
	WX_SOURCE_INTERNAL_PT100 = 6,

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

typedef struct __attribute__((aligned (4))) config_data_wx_sources_t {

	config_data_wx_sources_enum_t temperature;
	config_data_wx_sources_enum_t pressure;
	config_data_wx_sources_enum_t humidity;
	config_data_wx_sources_enum_t wind;

	config_data_wx_sources_enum_t temperature_telemetry;


} config_data_wx_sources_t;

typedef struct __attribute__((aligned (4))) config_data_umb_t {

	uint16_t slave_class;

	uint16_t slave_id;

	uint16_t channel_windspeed;

	uint16_t channel_wingsusts;

	uint16_t channel_winddirection;

	uint16_t channel_temperature;

	uint16_t channel_qnh;

	uint16_t serial_speed;
/**
 * #define _UMB_CHANNEL_WINDSPEED			460
#define _UMB_CHANNEL_WINDGUSTS			440
#define _UMB_CHANNEL_WINDDIRECTION		580
#define _UMB_CHANNEL_TEMPERATURE		100
#define _UMB_CHANNEL_QFE
 */
} config_data_umb_t;

typedef struct __attribute__((aligned (4))) config_data_rtu_t {
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

typedef struct __attribute__((aligned (4))) config_data_gsm_t {

	char pin[5];

	char apn[24];

	// username for APN connection
	char username[24];

	// password for APN connection
	char password[24];

	uint8_t api_enable;

	// http://pogoda.cc:8080/meteo_backend
	char api_base_url[64];

	char api_station_name[32];

	uint8_t aprsis_enable;

	char aprsis_server_address[64];

	uint16_t aprsis_server_port;

	uint32_t aprsis_passcode;

	uint8_t sms_wx_info;

	char sms_wx_inf_first[12];

	char sms_wx_inf_second[12];

} config_data_gsm_t;

/**
 * This structure holds compatibility numbers, which are used to check if
 * configuration data stored in the NVM is compatible with running software.
 * It is useful after firmware upgrade, which doesn't alter the configuration
 * data. Each config block has a separate entry here (like basic, mode, RTU).
 * During startup, if CRC verification is passed the software reads uint32_t
 * word for each config block, and compare with internally hardcoded value
 * If value in NVM is bigger than what application has, the app AND that
 * with a bitmask (also hardcoded) to check if it can safely ignore that.
 *
 * If it cannot ignore, the configuration is too new and supposedly
 * not compatible. The application should then either restore default values,
 * or switch to different interpretation of that data - at least do something. 
 *
 * If the value from NVM is less than value from application (but not equal!!!)
 * it is additionally ORed with another hardcoded bitmask. If a result of
 * this OR is bigger than application
 *
 *
 */
typedef struct __attribute__((aligned (4))) config_data_compatibility_version_t {

	uint64_t mode_block_compatiblity_number;		// 8 bytes

	uint64_t basic_block_compatiblity_number;		// 8 bytes

	uint64_t sources_block_compatiblity_number;		// 8 bytes

	uint64_t umb_block_compatiblity_number;			// 8 bytes

	uint64_t rtu_block_compatiblity_number;			// 8 bytes

	uint64_t gsm_block_compatiblity_number;			// 8 bytes

	uint64_t seventh_block_compatibility_number;

	uint64_t eight_block_compatibility_number;


}config_data_compatibility_version_t;

#endif /* CONFIG_DATA_H_ */
