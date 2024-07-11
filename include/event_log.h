/*
 * event_log.h
 *
 *  Created on: May 26, 2024
 *      Author: mateusz
 */

#ifndef EVENT_LOG_H_
#define EVENT_LOG_H_

#include "stdint.h"


/// ==================================================================================================
///	GLOBAL MACROS
/// ==================================================================================================

#define EVENT_LOG_GET_SEVERITY(x)			((x & 0xF0) >> 4)

#define EVENT_LOG_GET_SOURCE(x)				(x & 0x0F)

/// ==================================================================================================
///	GLOBAL DEFINITIONS
/// ==================================================================================================

#define EVENT_LOG_TIMESYNC_BOOTUP_WPARAM	(0x77U)


/// ==================================================================================================
///	GLOBAL TYPEDEFS
/// ==================================================================================================

/**
 * TYpe used to distinguish between event of different severity
 */
typedef enum event_log_severity_t {
	EVENT_BOOTUP,  /**< EVENT_BOOTUP all info events generated during bootup */
	EVENT_TIMESYNC,/**< EVENT_TIMESYNC event generated once at startup and then every 6 hours to keep master_time and RTC date and time sync */
	EVENT_DEBUG,   /**< EVENT_DEBUG */
	EVENT_INFO,    /**< EVENT_INFO */
	EVENT_WARNING, /**< EVENT_WARNING */
	EVENT_ERROR,   /**< EVENT_ERROR */
	EVENT_ASSERT   /**< EVENT_ASSERT assert failure, which result in hard reset*/
}event_log_severity_t;

/**
 * Where this long entry was generated
 */
typedef enum event_log_source_t {
	EVENT_SRC_MAIN,				/**< EVENT_SRC_MAIN everywhere within main.c source file */
	EVENT_SRC_WX_HANDLER,       /**< EVENT_SRC_WX_HANDLER */
	EVENT_SRC_PWR_SAVE,         /**< EVENT_SRC_PWR_SAVE */
	EVENT_SRC_PACKET_TX_HANDLER,/**< EVENT_SRC_PACKET_TX_HANDLER */
	EVENT_SRC_APRSIS,           /**< EVENT_SRC_APRSIS */
	EVENT_SRC_KISS,             /**< EVENT_SRC_KISS */
	EVENT_SRC_APRS_RF,          /**< EVENT_SRC_APRS_RF */
	EVENT_SRC_GSM_GPRS,         /**< EVENT_SRC_GSM_GPRS */
	EVENT_SRC_TCPIP,            /**< EVENT_SRC_TCPIP */
	EVENT_SRC_HTTP_CLIENT,      /**< EVENT_SRC_HTTP_CLIENT */
	EVENT_SRC_MODBUS,           /**< EVENT_SRC_MODBUS */
	EVENT_SRC_UMB,              /**< EVENT_SRC_UMB */
	EVENT_SRC_DRV_ANEMOMETER,   /**< EVENT_SRC_DRV_ANEMOMETER */
	EVENT_SRC_DRV_I2C,          /**< EVENT_SRC_DRV_I2C */
	EVENT_SRC_DRV_UART,         /**< EVENT_SRC_DRV_UART */
	EVENT_SRC_DRV_SPI,          /**< EVENT_SRC_DRV_SPI */
}event_log_source_t;


/**
 * Structure used to store single system event in RAM2 area and Flash
 * non volatile storage
 */
typedef struct __attribute__((aligned(1))) event_log_t {
	uint32_t event_counter_id;		//!< counter used to check which event is the oldest and newest one
	uint32_t event_master_time;		//!< value of maser time at the moment an event is generated
	uint8_t severity_and_source;	//!< high nibble -> severity level, low nibble -> source
	uint8_t event_id;				//!< event id, unique across different sources & severity level
	uint8_t param;					//!< Optional single-byte data, specific per event
	uint8_t param2;					//!< Optional single-byte data, specific per event
	uint16_t wparam;				//!< Optional 2-byte data, specific per event
	uint16_t wparam2;				//!< Optional 2-byte data, specific per event
	uint32_t lparam;				//!< Optional 4-byte data, specific per event
	uint32_t lparam2;				//!< Optional 4-byte data, specific per event
}event_log_t;


/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

/**
 * Initializes everything log related
 */
void event_log_init(uint8_t flash_enabled_severity, uint8_t ram_enabled_severity);

/**
 * Stores new event in RAM2_NOINIT area. It might trigger asynchronous
 * @param severity
 * @param source
 * @param wparam
 * @param lparam
 * @param lparam2
 */
void event_log(event_log_severity_t severity, event_log_source_t source, uint16_t wparam, uint32_t lparam, uint32_t lparam2);

void event_log_sync(event_log_severity_t severity, event_log_source_t source, uint16_t wparam, uint32_t lparam, uint32_t lparam2);


#endif /* EVENT_LOG_H_ */
