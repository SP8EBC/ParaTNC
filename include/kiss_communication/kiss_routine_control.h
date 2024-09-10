/*
 * kiss_routine_control.h
 *
 * Starts, stops and returns a result of diagnostic routines. Diagnostic routine is always started
 * only on diagnostic request and regardless if it is synchonous, asynchronous or asynchronous
 * with automatic stop, it doesn't persist across restart. If the controller is restarted while any
 * diagnostic routine is running, the routine is not restarted automatically. Each routine has small
 * state machine associated with it. Return codes from start, stop and get result corresponds to
 * positive return code or negative return code returned to a tester as a result of the request.
 * Positive / negative response in a response returned by diagnostics relays also on an internal
 * state of given routine.
 *
 * NRC_REQUEST_OUT_OF_RANGE -   will be returned always for unknown routine_id, this is handled
 *                              outside this file by @link{kiss_callback_routine_control}
 *
 * NRC_REQUEST_SEQUENCE_ERROR - will be returned if stop request will be sent for routine which
 *                              is not running, or starting async routine which is running now. 
 *                              it is also returned for get result request for routine which either 
 *                              hasn't been started yet, or it has been started and currently running.
 *
 * NRC_INCORRECT_MESSAGE_LENGTH_OR_FORMAT - will be returned if parameters values are wrong, so
 *                                          routine refuses to start
 * 
 * NRC_SUBFUNCTION_NOT_SUPPORTED - will be returned for any stop request for synchronous routine
 *
 *  Created on: Sep 8, 2024
 *      Author: mateusz
 */

#ifndef KISS_COMMUNICATION_KISS_ROUTINE_CONTROL_H_
#define KISS_COMMUNICATION_KISS_ROUTINE_CONTROL_H_

#include <stdint.h>

/// ==================================================================================================
///	GLOBAL MACROS
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL DEFINITIONS
/// ==================================================================================================

//!< Diagnostic routine runs asynchronously but it doesn't stops on its own
#define KISS_ROUTINE_CONTROL_ASYNCHRONOUS           (1u)

//!< Routine runs asynchronously and it stops automatically 
#define KISS_ROUTINE_CONTROL_ASYNCHRONOUS_AUTOSTOP  (2u)

//!< Routine has blocking I/O and ends automatically after starting
#define KISS_ROUTINE_CONTROL_SYNCHRO                (3u)

//!< Value of subfunction in diagnostic frame to start routine
#define KISS_ROUTINE_CONTROL_SUBFUNC_START                  (1u)

//!< Value of subfunction in diagnostic frame to stop routine
//!< Valid only for KISS_ROUTINE_CONTROL_ASYNCHRONOUS_AUTOSTOP 
//!< or KISS_ROUTINE_CONTROL_ASYNCHRONOUS. Synchronous routine
//!< will always stop itself
#define KISS_ROUTINE_CONTROL_SUBFUNC_STOP                   (2u)

//!< Value of subfunction in diagnostic frame to request for returning 
#define KISS_ROUTINE_CONTROL_SUBFUNC_RESULT                 (3u)

//!< General success
#define KISS_ROUTINE_RETVAL_SUCCESS                          (0u)

//!< Values of all parameters have been validated by a routine and it decides to start
#define KISS_ROUTINE_RETVAL_SUCCESSFULLY_STARTED             (1u)

#define KISS_ROUTINE_RETVAL_NOT_FOUND                        (2u)

//!< May be returned by start function if incorrect parameters was submitted in a request
#define KISS_ROUTINE_RETVAL_WRONG_PARAMS_VALUES              (3u)

//!< A request for starting a routine was sent while this routine is already started
#define KISS_ROUTINE_RETVAL_ALREADY_STARTED                  (4u)

//!< Synchronous routines cannot be stopped by diagnostics request. they simply starts, blocks
//!< io and ends on theirs now. Then a positive response to routine start request is send back
#define KISS_ROUTINE_RETVAL_STOP_FOR_SYNCHRO_ROUTINE         (5u)

//!< Stop request has been sent but the routine is not running 
#define KISS_ROUTINE_RETVAL_STOP_FOR_NOT_RUNNING             (6u)

#define KISS_ROUTINE_RETVAL_GENERAL_CATASTROPHIC_ERROR       (255u)

/// ==================================================================================================
///	GLOBAL TYPEDEFS
/// ==================================================================================================

//!< Routine control start function 
typedef uint8_t(*kiss_routine_control_start_t)(uint32_t lparam, uint16_t wparam);

//!< Routine control stop function 
typedef void(*kiss_routine_control_stop_t)(void);

//!< Routine control get result function
typedef uint16_t(*kiss_routine_control_getresult_t)(void);

/// ==================================================================================================
///	GLOBAL VARIABLES
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

/**
 * Callback, which must be used by all routines defined as KISS_ROUTINE_CONTROL_ASYNCHRONOUS_AUTOSTOP
 * after the operation is done 
 */
void kiss_routine_control_done_callback(uint16_t routine_id);

/**
 * Checks if routine with given id exists and returns it's type
 * @param id of routine to check
 * @returns zero if routine doesn't exists or KISS_ROUTINE_CONTROL_ASYNCHRONOUS or 
 *          KISS_ROUTINE_CONTROL_ASYNCHRONOUS_AUTOSTOP or KISS_ROUTINE_CONTROL_SYNCHRO depends on
 *          routine configuration
 */
uint8_t kiss_routine_control_check_routine_id(uint16_t id);

/**
 * This checks if routine result is available or not. It is required because any value returned 
 * from @link{kiss_routine_control_get_result_routine} may be right and correct value. This is 
 * not C++ so we don't have std::optional handy
 * @returns zero if result is not available 
 */
uint8_t kiss_routine_control_check_result_available(uint16_t id);

/**
 * Start given diagnostic routine by using a function hardcoded in @link{kiss_routine_definitions}
 * @param id of routine to start
 * @param wparaw shorter parameter to a routine
 * @param lparam longer parameter to a routine
 * @returns KISS_ROUTINE_RETVAL_SUCCESSFULLY_STARTED if parameters are OK and routine is started
 *          KISS_ROUTINE_RETVAL_WRONG_PARAMS_VALUES routine is not started
 *  
 */
uint8_t kiss_routine_control_start_routine(uint16_t id, uint16_t wparam, uint32_t lparam);

/**
 * Stops routine by using a function hardcoded in @link{kiss_routine_definitions}
 * @param id of routine to stop
 */
uint8_t kiss_routine_control_stop_routine(uint16_t id);

/**
 * Return a result of a routine
 * @param id of routine to get result for
 */
uint16_t kiss_routine_control_get_result_routine(uint16_t id);

#endif /* KISS_COMMUNICATION_KISS_ROUTINE_CONTROL_H_ */
