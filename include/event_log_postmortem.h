/*
 * event_log_postmortem.h
 *
 *  Created on: Mar 11, 2025
 *      Author: mateusz
 */

#ifndef EVENT_LOG_POSTMORTEM_H_
#define EVENT_LOG_POSTMORTEM_H_

/// ==================================================================================================
///	GLOBAL DEFINITIONS
/// ==================================================================================================

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
 * Check if noinit area contains a stack frame from hardfault. If yes stores it
 * as set of events in
 */
void event_log_postmortem_checknstore_hardfault(void);

/**
 * Checks if noinit are contains a coredump of supervisor array at the moment
 * fail has been detected
 */
void event_log_postmortem_checknstore_supervisor(void);

#endif /* EVENT_LOG_POSTMORTEM_H_ */
