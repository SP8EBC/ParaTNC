/*
 * event_log_t.h
 *
 *  Created on: Nov 8, 2024
 *      Author: mateusz
 */

#ifndef EVENT_LOG_T_H_
#define EVENT_LOG_T_H_

#include <stdint.h>

/// ==================================================================================================
///	GLOBAL TYPEDEFS
/// ==================================================================================================

/**
 * TYpe used to distinguish between event of different severity
 */
typedef enum event_log_severity_t {
	EVENT_DEBUG, 	/**< EVENT_DEBUG */
	EVENT_INFO,	 	/**< EVENT_INFO */
	EVENT_INFO_CYCLIC,
	EVENT_WARNING,	/**< EVENT_WARNING */
	EVENT_ERROR,	/**< EVENT_ERROR */
	EVENT_ASSERT,	/**< EVENT_ASSERT assert failure, which result in hard reset*/
	EVENT_BOOTUP,	/**< EVENT_BOOTUP all info events generated during bootup */
	EVENT_TIMESYNC, /**< EVENT_TIMESYNC event generated once at startup and then every 6 hours to
					   keep master_time and RTC date and time sync */

	EVENT_MARKER_BAD = 0xBA /**< EVENT_MARKER_BAD this is not a severity level, it is only a marker,
							   which may be placed only at the first element of new memory page. It
							   marks previous memory page as defective and exclude it from pointer
							   arithmetics and looking for newest and oldest  */
} event_log_severity_t;

/**
 * Where this long entry was generated
 */
typedef enum event_log_source_t {
	EVENT_SRC_MAIN,				 /**< EVENT_SRC_MAIN everywhere within main.c source file */
	EVENT_SRC_WX_HANDLER,		 /**< EVENT_SRC_WX_HANDLER */
	EVENT_SRC_PWR_SAVE,			 /**< EVENT_SRC_PWR_SAVE */
	EVENT_SRC_PACKET_TX_HANDLER, /**< EVENT_SRC_PACKET_TX_HANDLER */
	EVENT_SRC_APRSIS,			 /**< EVENT_SRC_APRSIS */
	EVENT_SRC_KISS,				 /**< EVENT_SRC_KISS */
	EVENT_SRC_APRS_RF,			 /**< EVENT_SRC_APRS_RF */
	EVENT_SRC_GSM_GPRS,			 /**< EVENT_SRC_GSM_GPRS */
	EVENT_SRC_TCPIP,			 /**< EVENT_SRC_TCPIP */
	EVENT_SRC_HTTP_CLIENT,		 /**< EVENT_SRC_HTTP_CLIENT */
	EVENT_SRC_MODBUS,			 /**< EVENT_SRC_MODBUS */
	EVENT_SRC_UMB,				 /**< EVENT_SRC_UMB */
	EVENT_SRC_DRV_ANEMOMETER,	 /**< EVENT_SRC_DRV_ANEMOMETER */
	EVENT_SRC_DRV_I2C,			 /**< EVENT_SRC_DRV_I2C */
	EVENT_SRC_DRV_UART,			 /**< EVENT_SRC_DRV_UART */
	EVENT_SRC_DRV_SPI,			 /**< EVENT_SRC_DRV_SPI */
} event_log_source_t;

/**
 * Structure used to store single system event in RAM2 area and Flash
 * non volatile storage. If MSB of severity is set to one, parameters area is
 * used to store 16 character wide string
 */
#ifdef UNIT_TEST
typedef struct __attribute__((packed)) event_log_t {
#else
typedef struct __attribute__ ((packed)) event_log_t {
#endif
	uint32_t event_counter_id;	 //!< counter used to check which event is the oldest and newest one
	uint32_t event_rtc;			 //!< RTC date and time in format returned by @link{main_get_nvm_timestamp}
	uint32_t event_master_time;	 //!< value of maser time at the moment an event is generated
	uint8_t severity; 			 //!< severity level defined by @link{event_log_severity_t}.
	uint8_t source;			     //!< event source defined by @link{event_log_source_t}
	uint8_t event_id;			 //!< event id, unique across different sources & severity level
	uint8_t param;				 //!< Optional single-byte data, specific per event
	uint8_t param2;				 //!< Optional single-byte data, specific per event
	uint16_t wparam;			 //!< Optional 2-byte data, specific per event
	uint16_t wparam2;			 //!< Optional 2-byte data, specific per event
	uint16_t wparam3;			 //!< Optional 2-byte data, specific per event
	uint32_t lparam;			 //!< Optional 4-byte data, specific per event
	uint32_t lparam2;			 //!< Optional 4-byte data, specific per event
	uint8_t crc_checksum;		 //!< less significant byte of CRC32 checksum of this entry
} event_log_t;

/**
 * This is a structure, which holds single event log in exposed and non-packed form, with separate
 * fields for severity, source etc. Used to extract and then send events in APRS message, include to
 * HTTP rest service query, etc.
 */
typedef struct event_log_exposed_t {
	uint32_t event_counter_id;	 //!< counter used to check which event is the oldest and newest one
	uint32_t event_master_time;	 //!< value of master time at the moment an event is generated
	uint32_t event_rtc;			 //!< RTC date and time in format returned by @link{main_get_nvm_timestamp}
	event_log_severity_t severity;
	const char * severity_str;
	event_log_source_t source;
	const char * source_str_name;//!< string representation of source name
	uint8_t event_id;			 //!< event id, unique across different sources & severity level
	const char * event_str_name; //!< string representation of the event
	uint8_t param;				 //!< Optional single-byte data, specific per event
	uint8_t param2;				 //!< Optional single-byte data, specific per event
	uint16_t wparam;			 //!< Optional 2-byte data, specific per event
	uint16_t wparam2;			 //!< Optional 2-byte data, specific per event
	uint16_t wparam3;
	uint32_t lparam;			 //!< Optional 4-byte data, specific per event
	uint32_t lparam2;			 //!< Optional 4-byte data, specific per event
	uint8_t crc8_from_frame;
	uint8_t crc8_calculated;
}event_log_exposed_t;


#endif /* EVENT_LOG_T_H_ */
