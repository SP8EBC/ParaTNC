/*
 * event_log.c
 *
 *  Created on: May 26, 2024
 *      Author: mateusz
 */

#include "event_log.h"
#include "./nvm/nvm_event.h"
#include "debug_hardfault.h"
#include "main_master_time.h"
#include "variant.h"

#include "crc_.h"

#include <stdio.h>
#include <string.h>

/* FreeRTOS includes. */
#include <FreeRTOS.h>
#include <task.h>

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
///	GLOBAL VARIABLES
/// ==================================================================================================

uint8_t event_log_rtos_running = 0;

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

/**
 *
 */
void event_log_init (void)
{
	memset (event_log_async_fifo, 0x00, sizeof (event_log_t) * EVENT_LOG_ASYNC_FIFO_LENGTH);
	event_log_fifo_current_depth = 0;
}

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
				  uint32_t lparam2)
{
	(void)severity;
	(void)source;
	(void)event_id;
	(void)param;
	(void)param2;
	(void)wparam;
	(void)wparam2;
	(void)lparam;
	(void)lparam2;
	return 0;
}

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
					   uint32_t lparam, uint32_t lparam2)
{
	event_log_t new_event = {0u};

	// left this to zero, to be automatically set to appropriate value by
	// pushing function
	new_event.event_counter_id = 0;

	new_event.event_id = event_id;
	new_event.event_master_time = main_get_master_time ();
	new_event.event_rtc = main_get_nvm_timestamp ();
	new_event.severity = EVENT_LOG_GET_SEVERITY (severity);
	new_event.source = EVENT_LOG_GET_SOURCE (source);

	new_event.param = param;
	new_event.param2 = param2;
	new_event.wparam = wparam;
	new_event.wparam2 = wparam2;
	new_event.lparam = lparam;
	new_event.lparam2 = lparam2;

	if (event_log_rtos_running) {
		taskENTER_CRITICAL ();
	}

	const nvm_event_result_t res = nvm_event_log_push_new_event (&new_event);

	if (event_log_rtos_running) {
		taskEXIT_CRITICAL ();
	}

	if (res == NVM_EVENT_OK) {
		return 0;
	}
	else {
		return -1;
	}
}

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
int8_t event_log_sync_triple (event_log_severity_t severity, event_log_source_t source,
							  uint8_t event_id, uint8_t param, uint8_t param2, uint16_t wparam,
							  uint16_t wparam2, uint16_t wparam3, uint32_t lparam, uint32_t lparam2)
{
	event_log_t new_event = {0u};

	// left this to zero, to be automatically set to appropriate value by
	// pushing function
	new_event.event_counter_id = 0;

	new_event.event_id = event_id;
	new_event.event_master_time = main_get_master_time ();
	new_event.event_rtc = main_get_nvm_timestamp ();
	new_event.severity = EVENT_LOG_GET_SEVERITY (severity);
	new_event.source = EVENT_LOG_GET_SOURCE (source);

	new_event.param = param;
	new_event.param2 = param2;
	new_event.wparam = wparam;
	new_event.wparam2 = wparam2;
	new_event.wparam3 = wparam3;
	new_event.lparam = lparam;
	new_event.lparam2 = lparam2;

	if (event_log_rtos_running) {
		taskENTER_CRITICAL ();
	}

	const nvm_event_result_t res = nvm_event_log_push_new_event (&new_event);

	if (event_log_rtos_running) {
		taskEXIT_CRITICAL ();
	}

	if (res == NVM_EVENT_OK) {
		return 0;
	}
	else {
		return -1;
	}
}


