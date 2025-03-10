/*
 * supervisor.h
 *
 *  Created on: Mar 10, 2025
 *      Author: mateusz
 */

#ifndef SUPERVISOR_H_
#define SUPERVISOR_H_

#include "etc/supervisor_config.h"

/// ==================================================================================================
///	GLOBAL DEFINITIONS
/// ==================================================================================================

#define SUPERVISOR_MAKE_ENUM_TYPE(thread, timeout)				\
		SUPERVISOR_THREAD_##thread,								\

/// ==================================================================================================
///	GLOBAL TYPEDEFS
/// ==================================================================================================

typedef enum supervisor_watchlist_t {
	SUPERVISOR_CONFIG(SUPERVISOR_MAKE_ENUM_TYPE)
	SUPERVISOR_THREAD_COUNT
}supervisor_watchlist_t;

/// ==================================================================================================
///	GLOBAL VARIABLES
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

/**
 * Called by all supervised threads or library from a place in which the entity
 * could be considered as alive and working OK
 * @param thread_or_library
 */
void supervisor_iam_alive(supervisor_watchlist_t thread_or_library);

/**
 * Should be called periodically from systick
 * @return non zero if something died
 */
int supervisor_service(void);

#endif /* SUPERVISOR_H_ */
