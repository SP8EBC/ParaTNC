/*
 * event_log.h
 *
 *  Created on: May 26, 2024
 *      Author: mateusz
 */

#ifndef EVENT_LOG_H_
#define EVENT_LOG_H_

#include "event_log_t.h"
#include "event_log_to_string.h"
#include "stdint.h"

/// ==================================================================================================
///	GLOBAL MACROS
/// ==================================================================================================

#define EVENT_LOG_GET_SEVERITY(x) (x & 0x7F)

#define EVENT_LOG_GET_SOURCE(x) (x & 0x7F)

#define EVENT_LOG_SET_SEVERITY_SOURCE(severity, source) \
	(((uint8_t)severity & 0xF) << 4) | ((uint8_t)source & 0xF)

#define EVENT_LOG_PACK_ARR_TO_UINT32(arr, offset)                                  \
	(uint32_t) (arr[offset] | (arr[offset + 1] << 8U) | (arr[offset + 2] << 16U) | \
				(arr[offset + 3] << 24U))

/// ==================================================================================================
///	GLOBAL DEFINITIONS
/// ==================================================================================================

#define EVENT_LOG_TIMESYNC_BOOTUP_WPARAM (0x77U)

/// ==================================================================================================
///	GLOBAL TYPEDEFS
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL VARIABLES
/// ==================================================================================================

extern uint8_t event_log_rtos_running;

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

/**
 * Initializes everything log related
 */
void event_log_init (void);

/**
 * Stores new event asynchronously. Events are written into all volatile, RAM mapped areas
 * immediately, but FLASH based areas are synchronized periodically.
 * @param severity
 * @param source
 * @param wparam
 * @param lparam
 * @param lparam2
 */
int8_t event_log (event_log_severity_t severity, event_log_source_t source, uint8_t event_id,
				  uint8_t param, uint8_t param2, uint16_t wparam, uint16_t wparam2, uint32_t lparam,
				  uint32_t lparam2);

/**
 * Stores an event synchronously to all targer areas
 * @param severity
 * @param source
 * @param event_id
 * @param param
 * @param param2
 * @param wparam
 * @param wparam2
 * @param lparam
 * @param lparam2
 * @return
 */
int8_t event_log_sync (event_log_severity_t severity, event_log_source_t source, uint8_t event_id,
					   uint8_t param, uint8_t param2, uint16_t wparam, uint16_t wparam2,
					   uint32_t lparam, uint32_t lparam2);

/**
 * Stores an event synchronously to all target areas, version with all wparams
 * @param severity
 * @param source
 * @param event_id
 * @param param
 * @param param2
 * @param wparam
 * @param wparam2
 * @param wparam3
 * @param lparam
 * @param lparam2
 * @return
 */
int8_t event_log_sync_triple (event_log_severity_t severity, event_log_source_t source,
							  uint8_t event_id, uint8_t param, uint8_t param2, uint16_t wparam,
							  uint16_t wparam2, uint16_t wparam3, uint32_t lparam,
							  uint32_t lparam2);



#endif /* EVENT_LOG_H_ */
