/*
 * event_log.c
 *
 *  Created on: May 26, 2024
 *      Author: mateusz
 */

#include "event_log.h"
#include "nvm_event.h"
#include "main_master_time.h"

/// ==================================================================================================
///	LOCAL DEFINITIONS
/// ==================================================================================================

#define EVENT_LOG_ASYNC_FIFO_LENGTH 16

/// ==================================================================================================
///	LOCAL VARIABLES
/// ==================================================================================================

static event_log_t event_log_async_fifo[EVENT_LOG_ASYNC_FIFO_LENGTH];

static int8_t event_log_fifo_current_depth = 0;

/// ==================================================================================================
///	LOCAL FUNCTIONS
/// ==================================================================================================

void event_log_init (void)
{
	memset (event_log_async_fifo, 0x00, sizeof (event_log_t) * EVENT_LOG_ASYNC_FIFO_LENGTH);
	event_log_fifo_current_depth = 0;
}

void event_log (event_log_severity_t severity,
				event_log_source_t source,
				uint8_t event_id,
				uint8_t param,
				uint8_t param2,
				uint16_t wparam,
				uint16_t wparam2,
				uint32_t lparam,
				uint32_t lparam2)
{
}

void event_log_sync (event_log_severity_t severity,
					 event_log_source_t source,
 					 uint8_t event_id,
					 uint8_t param,
					 uint8_t param2,
					 uint16_t wparam,
					 uint16_t wparam2,
					 uint32_t lparam,
					 uint32_t lparam2)
{
	event_log_t new_event = {0u};

	new_event.event_id = event_id;
	new_event.event_master_time = main_get_master_time();
	new_event.severity_and_source = EVENT_LOG_SET_SEVERITY_SOURCE(severity, source);

	new_event.param = param;
	new_event.param2 = param2;
	new_event.wparam = wparam;
	new_event.wparam2 = wparam2;
	new_event.lparam = lparam;
	new_event.lparam2 = lparam2;

	nvm_event_log_push_new_event(&new_event);
}