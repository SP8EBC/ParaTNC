/*
 * event_log.h
 *
 *  Created on: May 26, 2024
 *      Author: mateusz
 */

#ifndef EVENT_LOG_H_
#define EVENT_LOG_H_

#include "stdint.h"
#include "event_log_t.h"

/// ==================================================================================================
///	GLOBAL MACROS
/// ==================================================================================================

#define EVENT_LOG_GET_SEVERITY(x) (x & 0x7F)

#define EVENT_LOG_GET_SOURCE(x) (x & 0x7F)

#define EVENT_LOG_SET_SEVERITY_SOURCE(severity, source) \
	(((uint8_t)severity & 0xF) << 4) | ((uint8_t)source & 0xF)

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
int8_t event_log (event_log_severity_t severity,
				event_log_source_t source,
				uint8_t event_id,
				uint8_t param,
				uint8_t param2,
				uint16_t wparam,
				uint16_t wparam2,
				uint32_t lparam,
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
int8_t event_log_sync (event_log_severity_t severity,
					 event_log_source_t source,
					 uint8_t event_id,
					 uint8_t param,
					 uint8_t param2,
					 uint16_t wparam,
					 uint16_t wparam2,
					 uint32_t lparam,
					 uint32_t lparam2);

const char * event_log_severity_to_str(event_log_severity_t severity);

/**
 * Returns a pointer to a string representing event source
 * @param src
 * @return
 */
const char * event_log_source_to_str(event_log_source_t src);

/**
 *
 * @param source
 * @param event_id
 * @return
 */
const char * event_id_to_str(event_log_source_t source, uint8_t event_id);

/**
 * Generates string representation of given event log in exposed form
 * @param exposed pointer to an event to be converted
 * @param output char buffer to place a string into
 * @param output_ln maximum length of output string
 * @return length of assembled string
 */
uint16_t event_exposed_to_string(const event_log_exposed_t * exposed, char * output, uint16_t output_ln);

#endif /* EVENT_LOG_H_ */
