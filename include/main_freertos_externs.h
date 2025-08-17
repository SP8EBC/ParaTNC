/*
 * main_freertos_externs.h
 *
 *  Created on: Aug 17, 2025
 *      Author: mateusz
 */

#ifndef MAIN_FREERTOS_EXTERNS_H_
#define MAIN_FREERTOS_EXTERNS_H_

#include <FreeRTOS.h>
#include <event_groups.h>

/// ==================================================================================================
///	GLOBAL DEFINITIONS
/// ==================================================================================================

#define MAIN_EVENTGROUP_PWRSAVE_ONE_MIN			(1 << 0)
#define MAIN_EVENTGROUP_PWRSAVE_ONE_SEC			(1 << 1)
#define MAIN_EVENTGROUP_PWRSAVE_TWO_SEC			(1 << 2)
#define MAIN_EVENTGROUP_PWRSAVE_FOU_SEC			(1 << 3)
#define MAIN_EVENTGROUP_PWRSAVE_TEN_SEC			(1 << 4)

#define MAIN_EVENTGROUP_WAIT_FOR	(									\
									MAIN_EVENTGROUP_PWRSAVE_ONE_SEC |	\
									MAIN_EVENTGROUP_PWRSAVE_TWO_SEC |	\
									MAIN_EVENTGROUP_PWRSAVE_TEN_SEC		\
									)

/*
 *									MAIN_EVENTGROUP_PWRSAVE_FOU_SEC |	\
 *									MAIN_EVENTGROUP_PWRSAVE_ONE_MIN |	\
 */

/// ==================================================================================================
///	GLOBAL TYPEDEFS
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL VARIABLES
/// ==================================================================================================

//! Declare a variable to hold the handle of the created event group.
extern EventGroupHandle_t main_eventgroup_handle_powersave;

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================


#endif /* MAIN_FREERTOS_EXTERNS_H_ */
