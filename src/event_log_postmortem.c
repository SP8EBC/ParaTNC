/*
 * evnet_log_postmortem.c
 *
 *  Created on: Mar 11, 2025
 *      Author: mateusz
 */

#include "event_log_postmortem.h"
#include "event_log.h"
#include "supervisor.h"
#include "debug_hardfault.h"
#include "memory_map.h"

#include "./events_definitions/events_main.h"

#include <string.h>
#include <stm32l4xx.h>

/// ==================================================================================================
///	LOCAL DEFINITIONS
/// ==================================================================================================

/// ==================================================================================================
///	LOCAL DATA TYPES
/// ==================================================================================================

/// ==================================================================================================
///	LOCAL VARIABLES
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL VARIABLES
/// ==================================================================================================

/// ==================================================================================================
///	LOCAL FUNCTIONS
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================
/**
 * Check if noinit area contains a stack frame from hardfault. If yes stores it
 * as set of events in
 */
void event_log_postmortem_checknstore_hardfault(void)
{
	const uint8_t have = debug_hardfault_check_have_postmortem();

	if (have != 0)
	{
		debug_hardfault_postmortem_stackframe_t frame;

		debug_hardfault_get_postmortem(&frame);

		event_log_sync(
				EVENT_ERROR,
				EVENT_SRC_MAIN,
				EVENTS_MAIN_POSTMORTEM_HARDFAULT,
				1, 0, 0, 0, frame.lr, frame.pc);

		event_log_sync(
				EVENT_ERROR,
				EVENT_SRC_MAIN,
				EVENTS_MAIN_POSTMORTEM_HARDFAULT,
				2, 0, 0, 0, frame.r0, frame.r1);

		event_log_sync(
				EVENT_ERROR,
				EVENT_SRC_MAIN,
				EVENTS_MAIN_POSTMORTEM_HARDFAULT,
				3, 0, 0, 0, frame.r2, frame.r3);

		event_log_sync(
				EVENT_ERROR,
				EVENT_SRC_MAIN,
				EVENTS_MAIN_POSTMORTEM_HARDFAULT,
				4, 0, 0, 0, frame.r12, frame.cfsr);

		event_log_sync(
				EVENT_ERROR,
				EVENT_SRC_MAIN,
				EVENTS_MAIN_POSTMORTEM_HARDFAULT,
				5, 0, 0, 0, frame.source, frame.xpsr);

		debug_hardfault_delete_postmortem();
	}
}

/**
 * Checks if noinit are contains a coredump of supervisor array at the moment
 * fail has been detected
 */
void event_log_postmortem_checknstore_supervisor(void)
{
	// very local (??) helper macro, can be used for 16 and 32 bit ints
	// make very little sense to use this for 8-bit
#define _PUT_TASKNAME_INTO_WORD(target, char_from_begining, nibble) \
	(target |= (task_name_string[char_from_begining] << (nibble * 8)))

#define _PUT_UNDERSCORE_INTO_WORD(target, nibble) \
	(target |= ('_' << (nibble * 8)))

	const uint8_t timestamp_idx = 		MEMORY_MAP_SRAM1_SUPERVISOR_LOG_32BWORDS_SIZE - 2;
	const uint8_t checksum_idx = 		MEMORY_MAP_SRAM1_SUPERVISOR_LOG_32BWORDS_SIZE - 1;

	// check if there is anything stored, by calculating checksum
	const int have = supervisor_check_have_postmortem();

	// all of those used to store task name
	uint8_t param1 = 0, param2 = 0;
	uint16_t wparam1 = 0, wparam2 = 0;
	uint32_t lparam = 0;

	if (have != 0)
	{
		// pointer to the begining of supervisor log SRAM1 area
		volatile uint32_t * ptr = (volatile uint32_t *)MEMORY_MAP_SRAM1_SUPERVISOR_LOG_START;

		// log in this area is stored in the same order than an enum @link{supervisor_watchlist_t}
		for (int i = 0; i < MEMORY_MAP_SRAM1_SUPERVISOR_LOG_32BWORDS_SIZE; i+=2)
		{
			// last two 32-bit words contain a master time at the moment of sup fail and the checksum
			if (i == timestamp_idx)
			{
				const uint32_t master_time_at_fail = ptr[i];

				event_log_sync_triple(
					EVENT_ERROR,
					EVENT_SRC_MAIN,
					EVENTS_MAIN_POSTMORTEM_SUPERVISOR,
					0xFFu, 0xFFu,						/* prams */
					0xDEADu, 0xBEEFu, 0x00u,					/* wprams */
					master_time_at_fail, 0x00u);			/* lprams */
				// no sense to store checksum
				break;
			}

			// time in miliseconds, since last call to the supervisor
			const uint32_t time = ptr[i] & 0x00FFFFFFu;

			// time since last call to supervisor rescaled to 20ms increment
			// 1   -> 20ms
			// 10  -> 200ms
			// 50  -> 1000ms
			// 300 -> 6000ms
			uint16_t scaled_time = 0u;

			// check if it is possible to do a rescale
			if (time > 1310700u)
			{
				// set fied value if time since last call to supervisor
				// is greater than 1310 seconds ago
				scaled_time = 65535u;
			}
			else
			{
				scaled_time = time / 20u;
			}

			// task id which is stored here
			const supervisor_watchlist_t task = (supervisor_watchlist_t)((ptr[i] & 0xFF000000u) >> 24);

			// sanity check if this task is valid
			if (task >= SUPERVISOR_THREAD_COUNT)
			{
				continue;
			}

			// get a name of this task
			const char * const task_name_string = supervisor_descriptions[task];

			// get length of this name
			const size_t name_len = strlen (task_name_string);

			// get checkpoints
			const uint32_t checkpoints = ptr[i + 1];

			// we can save only 10 characters of the name.
			// if the name is short, put everything from the begining
			if (name_len <= 10) {
				(name_len >= 1) ? (param1 = task_name_string[0]) : (param1 = 0u);
				(name_len >= 2) ? (param2 = task_name_string[1]) : (param2 = 0u);
				(name_len >= 3) ? _PUT_TASKNAME_INTO_WORD (wparam1, 2, 0) : _PUT_UNDERSCORE_INTO_WORD(wparam1, 0);
				(name_len >= 4) ? _PUT_TASKNAME_INTO_WORD (wparam1, 3, 1) : _PUT_UNDERSCORE_INTO_WORD(wparam1, 1);
				(name_len >= 5) ? _PUT_TASKNAME_INTO_WORD (wparam2, 4, 0) : _PUT_UNDERSCORE_INTO_WORD(wparam2, 0);
				(name_len >= 6) ? _PUT_TASKNAME_INTO_WORD (wparam2, 5, 1) : _PUT_UNDERSCORE_INTO_WORD(wparam2, 1);
				(name_len >= 7) ? _PUT_TASKNAME_INTO_WORD (lparam, 6, 0) : _PUT_UNDERSCORE_INTO_WORD(lparam, 0);
				(name_len >= 8) ? _PUT_TASKNAME_INTO_WORD (lparam, 7, 1) : _PUT_UNDERSCORE_INTO_WORD(lparam, 1);
				(name_len >= 9) ? _PUT_TASKNAME_INTO_WORD (lparam, 8, 2) : _PUT_UNDERSCORE_INTO_WORD(lparam, 2);
				(name_len >= 10) ? _PUT_TASKNAME_INTO_WORD (lparam, 9, 3) : _PUT_UNDERSCORE_INTO_WORD(lparam, 3);
			}
			else {
				// if name is longer than 10 characters, use last 10 characters
				param1 = task_name_string[name_len - 10];
				param2 = task_name_string[name_len - 9];
				_PUT_TASKNAME_INTO_WORD (wparam1, name_len - 8, 0);
				_PUT_TASKNAME_INTO_WORD (wparam1, name_len - 7, 1);
				_PUT_TASKNAME_INTO_WORD (wparam2, name_len - 6, 0);
				_PUT_TASKNAME_INTO_WORD (wparam2, name_len - 5, 1);
				_PUT_TASKNAME_INTO_WORD (lparam, name_len - 4, 0);
				_PUT_TASKNAME_INTO_WORD (lparam, name_len - 3, 1);
				_PUT_TASKNAME_INTO_WORD (lparam, name_len - 2, 2);
				_PUT_TASKNAME_INTO_WORD (lparam, name_len - 1, 3);
			}

			event_log_sync_triple(
					EVENT_ERROR,
					EVENT_SRC_MAIN,
					EVENTS_MAIN_POSTMORTEM_SUPERVISOR,
					param1, param2,						/* prams */
					wparam1, wparam2, scaled_time,					/* wprams */
					lparam, checkpoints);			/* lprams */

			ptr[i] 		= 0;
			ptr[i + 1] 	= 0;

			// clear local variables before next use
			wparam1 = 0;
			wparam2 = 0;
			lparam = 0;

		}

	}

	return;
}
