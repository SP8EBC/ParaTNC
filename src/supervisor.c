/*
 * supervisor.c
 *
 *  Created on: Mar 10, 2025
 *      Author: mateusz
 */

#include "supervisor.h"
#include <stdint.h>

#include "main_master_time.h"

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

static uint32_t supervisor_last_im_alive[SUPERVISOR_THREAD_COUNT] = {0u};

const static uint16_t supervisor_timeouts_conf[SUPERVISOR_THREAD_COUNT] = {
		SUPERVISOR_CONFIG(SUPERVISOR_MAKE_TIMEOUT_ARR)
};

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
 * Called by all supervised threads or library from a place in which the entity
 * could be considered as alive and working OK
 * @param thread_or_library
 */
void supervisor_iam_alive(supervisor_watchlist_t thread_or_library)
{

}

/**
 * Should be called periodically from systick
 * @return non zero if something died
 */
int supervisor_service(void)
{
	int nok = 0;

	// current time sunce bootup
	const uint32_t current_time = main_get_master_time();

	for (int i = 0; i < SUPERVISOR_THREAD_COUNT; i++)
	{
		// number of miliseconds since last time this library / thread has reported
		const int32_t since_last_alive = current_time - supervisor_last_im_alive[i];

		// maximum timeout (in seconds!!!!)
		const uint16_t max_seconds_since = supervisor_timeouts_conf[i];

		if (since_last_alive > (int32_t)(max_seconds_since * 1000))
		{
			nok = 1;
			break;
		}
	}


	return nok;

}
