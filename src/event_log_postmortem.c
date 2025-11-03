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

#ifdef STM32L471xx
#include <stm32l4xx.h>
#endif

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
	const int have = supervisor_check_have_postmortem();

	if (have != 0)
	{
		uint32_t * ptr = (uint32_t *)MEMORY_MAP_SRAM1_SUPERVISOR_LOG_START;

		for (int i = 0; i < MEMORY_MAP_SRAM1_SUPERVISOR_LOG_32BWORDS_SIZE; i+=2)
		{
			// current time after last call to supervisor in miliseconds
			const uint32_t time = ptr[i];
			const uint32_t checkpoints = ptr[i + 1];

			event_log_sync(
					EVENT_ERROR,
					EVENT_SRC_MAIN,
					EVENTS_MAIN_POSTMORTEM_SUPERVISOR,
					i, 0, 0, 0, time, checkpoints);

			ptr[i] 		= 0;
			ptr[i + 1] 	= 0;

		}

	}

	return;
}
