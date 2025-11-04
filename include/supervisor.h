/*
 * supervisor.h
 *
 *  Created on: Mar 10, 2025
 *      Author: mateusz
 */

#ifndef SUPERVISOR_H_
#define SUPERVISOR_H_

#include <stdint.h>

#include "etc/supervisor_config.h"

/// ==================================================================================================
///	GLOBAL DEFINITIONS
/// ==================================================================================================

/**
 * Create single entry of an enum with all monitored
 */
#define SUPERVISOR_MAKE_ENUM_TYPE(thread, timeout)				\
		SUPERVISOR_THREAD_##thread,								\

/**
 * Create an single entry of structure with monitor checkpoints
 */
#define SUPERVISOR_MAKE_CHECKPOINTS_STRUCT(thread, timeout)	\
		uint32_t monitor_##thread;				\

/**
 * set execution monitor mark
 */
#define SUPERVISOR_MONITOR_SET_CHECKPOINT(thread, point)										\
	supervisor_execution_checkpoints.monitor_##thread |= (1 << (uint8_t)(point));		\

/**
 * Reset execution monitor bitmask to zero, should be called
 */
#define SUPERVISOR_MONITOR_CLEAR(thread)					\
	supervisor_execution_checkpoints.monitor_##thread = 0;	\

/// ==================================================================================================
///	GLOBAL TYPEDEFS
/// ==================================================================================================

/**
 * Enum with all supervised tasks
 */
typedef enum supervisor_watchlist_t {
	SUPERVISOR_CONFIG(SUPERVISOR_MAKE_ENUM_TYPE)
	SUPERVISOR_THREAD_COUNT
}supervisor_watchlist_t;

/**
 * Structure of bitmasks, which works as an execution monitor for each task.
 * A bitmask stores an info, which part of flow was executed and which not.
 * In case of supervisor fault the value might be somewhat helpful to debug
 * where the task got stuck.
 */
typedef struct supervisor_tasks_checkpoints_t  {
	SUPERVISOR_CONFIG(SUPERVISOR_MAKE_CHECKPOINTS_STRUCT)
}supervisor_tasks_checkpoints_t;

/// ==================================================================================================
///	GLOBAL VARIABLES
/// ==================================================================================================

/**
 * an instance of the structure with markpoints
 */
extern supervisor_tasks_checkpoints_t supervisor_execution_checkpoints;

/**
 * Array of string representing each supervised task.
 * @note an order of this array resembled the order of supervisor_watchlist_t
 * @warning because of limited area in single log entry, only 10 last characters are used
 */
extern const char * const supervisor_descriptions[];

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

/**
 * Check if noinit area contains valid postmortem supervisor coredump
 * @return
 */
int supervisor_check_have_postmortem(void);

/**
 * Starts task monitoring
 */
void supervisor_start(void);

/**
 * @return non zero if supervisor is started
 */
int supervisor_is_started(void);

#endif /* SUPERVISOR_H_ */
