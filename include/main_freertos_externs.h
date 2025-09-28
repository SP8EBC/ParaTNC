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
// clang-format off
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

#define MAIN_EVENTGROUP_SERIAL_KISS_RX_DONE		(1 << 0)
#define MAIN_EVENTGROUP_SERIAL_GSM_RX_DONE		(1 << 1)
#define MAIN_EVENTGROUP_SERIAL_GSM_TX_DONE		(1 << 2)
#define MAIN_EVENTGROUP_SERIAL_KISS_TX_DONE		(1 << 3)

#define MAIN_EVENTGROUP_APRSIS_TRIG_MESSAGE_ACK			(1 << 0)
#define MAIN_EVENTGROUP_APRSIS_TRIG_SEND_MESSAGE		(1 << 1)
#define MAIN_EVENTGROUP_APRSIS_TRIG_GSM_STATUS			(1 << 2)
#define MAIN_EVENTGROUP_APRSIS_TRIG_APRSIS_COUNTERS		(1 << 3)
#define MAIN_EVENTGROUP_APRSIS_TRIG_APRSIS_LOGINSTRING	(1 << 4)
#define MAIN_EVENTGROUP_APRSIS_TRIG_TELEMETRY_VALUES	(1 << 5)
#define MAIN_EVENTGROUP_APRSIS_TRIG_TELEMETRY_DESCR		(1 << 6)
#define MAIN_EVENTGROUP_APRSIS_TRIG_EVENTS				(1 << 7)

#define MAIN_EVENTGROUP_APRSIS_TRIG  	MAIN_EVENTGROUP_APRSIS_TRIG_MESSAGE_ACK 		|	\
										MAIN_EVENTGROUP_APRSIS_TRIG_SEND_MESSAGE 		| 	\
										MAIN_EVENTGROUP_APRSIS_TRIG_GSM_STATUS			|	\
										MAIN_EVENTGROUP_APRSIS_TRIG_APRSIS_COUNTERS 	|	\
										MAIN_EVENTGROUP_APRSIS_TRIG_APRSIS_LOGINSTRING 	| 	\
										MAIN_EVENTGROUP_APRSIS_TRIG_TELEMETRY_VALUES	|	\
										MAIN_EVENTGROUP_APRSIS_TRIG_EVENTS

// clang-format on
/// ==================================================================================================
///	GLOBAL TYPEDEFS
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL VARIABLES
/// ==================================================================================================

//! Event group synchronizing powersave task not to run while another tasks are running.
extern EventGroupHandle_t main_eventgroup_handle_powersave;

extern EventGroupHandle_t main_eventgroup_handle_serial_kiss;

//! Event group blocking GSM 800 driver code until rx or tx from/to GSM modem is complete.
extern EventGroupHandle_t main_eventgroup_handle_serial_gsm;

//! Event group blocking GSM 800 driver code until rx or tx from/to GSM modem is complete.
extern EventGroupHandle_t main_eventgroup_handle_aprs_trigger;

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

#endif /* MAIN_FREERTOS_EXTERNS_H_ */
