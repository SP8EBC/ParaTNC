/*
 * supervisor.c
 *
 *  Created on: Mar 10, 2025
 *      Author: mateusz
 */

#include "supervisor.h"
#include <stdint.h>

#ifdef STM32L471xx
#include <stm32l4xx.h>
#endif

#include "main_master_time.h"
#include "memory_map.h"

/// ==================================================================================================
///	LOCAL DEFINITIONS
/// ==================================================================================================

#define SUPERVISOR_MAKE_TIMEOUT_ARR(thread, timeout)				\
		timeout,								\

/// ==================================================================================================
///	LOCAL DATA TYPES
/// ==================================================================================================

/// ==================================================================================================
///	LOCAL VARIABLES
/// ==================================================================================================

static uint8_t supervisor_started = 0u;

static uint32_t supervisor_last_im_alive[SUPERVISOR_THREAD_COUNT] = {0u};

static const uint16_t supervisor_timeouts_conf[SUPERVISOR_THREAD_COUNT] = {
		SUPERVISOR_CONFIG(SUPERVISOR_MAKE_TIMEOUT_ARR)
};

static const uint8_t timestamp_idx = 		MEMORY_MAP_SRAM1_SUPERVISOR_LOG_32BWORDS_SIZE - 2;
static const uint8_t checksum_idx = 		MEMORY_MAP_SRAM1_SUPERVISOR_LOG_32BWORDS_SIZE - 1;

/// ==================================================================================================
///	GLOBAL VARIABLES
/// ==================================================================================================

supervisor_tasks_checkpoints_t supervisor_execution_checkpoints = {0u};

/// ==================================================================================================
///	LOCAL FUNCTIONS
/// ==================================================================================================

/**
 * Stores a content of fist 16 entries from 'supervisor_last_im_alive' into NONINIT area 
 */
static void supervisor_store(supervisor_watchlist_t what_failed)
{
	const uint32_t timestamp = 			main_get_master_time();

	uint32_t * monitor_checkpoints = 	(uint32_t *)&supervisor_execution_checkpoints;
	volatile uint32_t * supervisor_store_area = (volatile uint32_t *)MEMORY_MAP_SRAM1_SUPERVISOR_LOG_START;
	uint8_t ptr_it = 0;

	uint32_t checksum = 0;

	// clear storage
	for (int i = 0; i < MEMORY_MAP_SRAM1_SUPERVISOR_LOG_32BWORDS_SIZE; i++)	// currently 30 words
	{
		supervisor_store_area[i] = 0;
	}

	// save current supervisor timeout value AND execution checkpoint bitmask
	// so increment iterator by 2, for each configured entry
	for (int i = 0; i < SUPERVISOR_THREAD_COUNT; i++)
	{
		if (ptr_it == (timestamp_idx))
		{
			break;
		}
		else
		{
			const uint32_t lastCallToSprvs = (timestamp - supervisor_last_im_alive[i]);
			// current time after last call to supervisor in miliseconds
			supervisor_store_area[ptr_it++] = lastCallToSprvs;
			if ((supervisor_watchlist_t)i == what_failed)
			{
				supervisor_store_area[ptr_it++] = monitor_checkpoints[i] & 0x80000000u;
			}
			else
			{
				supervisor_store_area[ptr_it++] = monitor_checkpoints[i];
			}
		}
	}

	// save current master time
	supervisor_store_area[timestamp_idx] 	= timestamp;

	// everything except checksum, stored in last word
	for (int i = 0; i < checksum_idx; i++)
	{
		const uint32_t elem = supervisor_store_area[i];
		checksum += elem;
	}

	supervisor_store_area[checksum_idx] 	= 0xFFFFFFFFu - checksum;
}


/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

/**
 * Called by all supervised threads or library from a place in which the entity
 * could be considered as alive and working OK
 * @param thread_or_library
 */
void supervisor_iam_alive(supervisor_watchlist_t thread_or_library)
{
	if (thread_or_library < SUPERVISOR_THREAD_COUNT)
	{
		supervisor_last_im_alive[thread_or_library] = main_get_master_time();
	}
}

/**
 * Should be called periodically from systick
 * @return non zero if something died
 */
int supervisor_service(void)
{
	int nok = 0;

	if (supervisor_started == 0)
	{
		return nok;
	}

	// current time since bootup
	const uint32_t current_time = main_get_master_time();

	for (volatile int i = 0; i < SUPERVISOR_THREAD_COUNT; i++)
	{
		// number of miliseconds since last time this library / thread has reported
		volatile const int32_t since_last_alive = current_time - supervisor_last_im_alive[i];

		// maximum timeout (in seconds!!!!)
		volatile const uint16_t max_seconds_since = supervisor_timeouts_conf[i];

		if (since_last_alive > (int32_t)(max_seconds_since * 1000))
		{
			supervisor_store((supervisor_watchlist_t)i);
			nok = 1;
			break;
		}
	}


	return nok;

}

/**
 * Check if noinit area contains valid postmortem supervisor coredump
 * @return
 */
int supervisor_check_have_postmortem(void)
{
	int have = 0;

	volatile uint32_t * supervisor_store_area = (volatile uint32_t *)MEMORY_MAP_SRAM1_SUPERVISOR_LOG_START;

	uint32_t checksum = 0;
	const uint32_t stored_checksum = supervisor_store_area[checksum_idx];

	for (int i = 0; i < checksum_idx; i++)
	{
		const uint32_t elem = supervisor_store_area[i];
		checksum += elem;
	}

	const uint32_t calculated_checksum = 0xFFFFFFFFu - checksum;

	if (stored_checksum == calculated_checksum)
	{
		have = 1;
	}

	return have;
}

void supervisor_start(void)
{
	// current time since bootup
	const uint32_t current_time = main_get_master_time();

	memset(&supervisor_execution_checkpoints, 0x00, sizeof(supervisor_tasks_checkpoints_t));

	for (int i = 0; i < SUPERVISOR_THREAD_COUNT; i++)
	{
		supervisor_last_im_alive[i] = current_time;
	}

	supervisor_started = 1;
}

int supervisor_is_started(void)
{
	return supervisor_started;
}
