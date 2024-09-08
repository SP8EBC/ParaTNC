/*
 * kiss_routine_control.h
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

/// ==================================================================================================
///	GLOBAL TYPEDEFS
/// ==================================================================================================

//!< Routine control start function 
typedef void(*kiss_routine_control_start_t)(uint32_t lparam, uint16_t wparam);

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
 * Checks if routine with given id exists and returns it's type
 * @param id of routine to check
 * @returns zero if routine doesn't exists or KISS_ROUTINE_CONTROL_ASYNCHRONOUS or 
 *          KISS_ROUTINE_CONTROL_ASYNCHRONOUS_AUTOSTOP or KISS_ROUTINE_CONTROL_SYNCHRO depends on
 *          routine configuration
 */
uint8_t kiss_routine_control_check_routine_id(uint16_t id);

/**
 * 
 */
uint8_t kiss_routine_control_start_routine(uint16_t id);

#endif /* KISS_COMMUNICATION_KISS_ROUTINE_CONTROL_H_ */
