/*
 * kiss_routine_control.c
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

#include "./kiss_communication/diagnostics_services/kiss_routine_control.h"
#include "./kiss_communication/diagnostics_routines/routine_5254_set_datetime.h"
#include "./kiss_communication/types/kiss_xmacro_helpers.h"

#include "./etc/kiss_routine_configuration.h"

/// ==================================================================================================
///	LOCAL DEFINITIONS
/// ==================================================================================================

/// ==================================================================================================
///	LOCAL DATA TYPES
/// ==================================================================================================

//!< Definition of single diagnostics routine. It will be hardcoded as const in the flash memory
typedef struct kiss_routine_control_t {
	uint16_t routine_number;
	kiss_routine_control_start_t start_fn;
	kiss_routine_control_stop_t stop_fn;
	kiss_routine_control_getresult_t get_result_fn;
	uint8_t asynchronous;
} kiss_routine_control_t;

//!< Definition of runtime status for single diagnostic routine
typedef struct kiss_routine_status_t {
	uint16_t routine_number;  //!< Routine ID
	uint8_t running;		  //!< if the routine is currently running
	uint8_t result_available; //!< if routine is not running now, but a result is available
} kiss_routine_status_t;

//!< Enum definition used to count all definitions
typedef enum kiss_routine_enum_t {
	KISS_ROUTINES (KISS_DIAGNOSTIC_ROUTINE_ENUM_EXPAND) 
    KISS_ROUTINE_NUMBER_COUNT
} kiss_routine_enum_t;

/// ==================================================================================================
///	LOCAL VARIABLES
/// ==================================================================================================

//!< Const definitions of all routine diagnostics
static const kiss_routine_control_t kiss_routine_definitions[] = {
	KISS_ROUTINES (KISS_ROUTINE_DEF_EXPAND)
};

//!< Array of current status structures for all routines defined
static kiss_routine_status_t kiss_routine_status[] = {
    KISS_ROUTINES (KISS_ROUTINE_STATUS_EXPAND)
};

/// ==================================================================================================
///	GLOBAL VARIABLES
/// ==================================================================================================

/// ==================================================================================================
///	LOCAL FUNCTIONS
/// ==================================================================================================

/// ==================================================================================================
///	LOCAL FUNCTIONS
/// ==================================================================================================

static kiss_routine_status_t *kiss_routine_get_status_by_id (uint16_t id)
{
	kiss_routine_status_t *status = 0;

	for (int i = 0; i < KISS_ROUTINE_NUMBER_COUNT; i++) {
		if (kiss_routine_status[i].routine_number == id) {
			status = &kiss_routine_status[i];
			break;
		}
	}

	return status;
}

static kiss_routine_control_t *kiss_routine_get_by_id (uint16_t id)
{
	kiss_routine_control_t *routine = 0;

	for (int i = 0; i < KISS_ROUTINE_NUMBER_COUNT; i++) {
		if (kiss_routine_definitions[i].routine_number == id) {
			routine = &kiss_routine_definitions[i];
			break;
		}
	}

	return routine;
}

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

/**
 * Callback, which must be used by all routines defined as
 * KISS_ROUTINE_CONTROL_ASYNCHRONOUS_AUTOSTOP after the operation is done
 */
void kiss_routine_control_done_callback (uint16_t routine_id)
{
	kiss_routine_status_t *status = kiss_routine_get_status_by_id (routine_id);

	if (status != 0) {
		status->running = 0;
		status->result_available = 1;
	}
}

/**
 * This checks if routine result is available or not. It is required because any value returned
 * from @link{kiss_routine_control_get_result_routine} may be right and correct value. This is
 * not C++ so we don't have std::optional handy
 * @returns zero if result is not available
 */
uint8_t kiss_routine_control_check_result_available (uint16_t id)
{
	const kiss_routine_status_t *status = kiss_routine_get_status_by_id (id);

	if (status != 0) {
		return status->result_available;
	}
	else {
		return 0;
	}
}

/**
 * Checks if routine with given id exists and returns its type
 * @param id of routine to check
 * @returns zero if routine doesn't exists or KISS_ROUTINE_CONTROL_ASYNCHRONOUS or
 *          KISS_ROUTINE_CONTROL_ASYNCHRONOUS_AUTOSTOP or KISS_ROUTINE_CONTROL_SYNCHRO depends on
 *          routine configuration
 */
uint8_t kiss_routine_control_check_routine_id (uint16_t id)
{

	uint8_t out = 0;

	kiss_routine_control_t *routine = kiss_routine_get_by_id (id);

	if (routine != 0) {
		out = routine->asynchronous;
	}

	return out;
}

/**
 * Start given diagnostic routine by using a function hardcoded in @link{kiss_routine_definitions}
 * @param id of routine to start
 * @param wparaw shorter parameter to a routine
 * @param lparam longer parameter to a routine
 * @returns KISS_ROUTINE_RETVAL_SUCCESSFULLY_STARTED if parameters are OK and routine is started
 *          KISS_ROUTINE_RETVAL_WRONG_PARAMS_VALUES routine is not started
 *
 */
uint8_t kiss_routine_control_start_routine (uint16_t id, uint16_t wparam, uint32_t lparam)
{
    // this should be always amended either here or within start_fn
    uint8_t out = KISS_ROUTINE_RETVAL_GENERAL_CATASTROPHIC_ERROR;

    const kiss_routine_control_t *routine = kiss_routine_get_by_id (id);
	kiss_routine_status_t *status = kiss_routine_get_status_by_id (id);

    if (routine != 0 && status != 0) {
        if (status->running == 0) {
            // this shall return one of codes mentioned in the doxygen comment 
            out = routine->start_fn(lparam, wparam);

            // set running flag only for asynchronous flag
            if (routine->asynchronous == KISS_ROUTINE_CONTROL_ASYNCHRONOUS ||
                routine->asynchronous == KISS_ROUTINE_CONTROL_ASYNCHRONOUS_AUTOSTOP) 
                {
                    status->running = 1;
                }
        }
        else {
            out = KISS_ROUTINE_RETVAL_ALREADY_STARTED;
        }

    }

    return out;
}

/**
 * Stops routine by using a function hardcoded in @link{kiss_routine_definitions}
 * @param id of routine to stop
 */
uint8_t kiss_routine_control_stop_routine (uint16_t id)
{
    uint8_t out = KISS_ROUTINE_RETVAL_GENERAL_CATASTROPHIC_ERROR;

    const kiss_routine_control_t *routine = kiss_routine_get_by_id (id);
	kiss_routine_status_t *status = kiss_routine_get_status_by_id (id);

    if (routine != 0 && status != 0) {
        if (routine->asynchronous == KISS_ROUTINE_CONTROL_SYNCHRO) 
        {  
            out = KISS_ROUTINE_RETVAL_STOP_FOR_SYNCHRO_ROUTINE;
        }   
        else
        {
            if (status->running == 0) {
                out = KISS_ROUTINE_RETVAL_STOP_FOR_NOT_RUNNING;
            }
            else {
                routine->stop_fn();
                out = KISS_ROUTINE_RETVAL_SUCCESS;
            }
        }  
    }

    return out;
}

/**
 * Return a result of a routine
 * @param id of routine to get result for
 */
uint16_t kiss_routine_control_get_result_routine (uint16_t id)
{
    uint16_t out = KISS_ROUTINE_RETVAL_GENERAL_CATASTROPHIC_ERROR;

    const kiss_routine_control_t *routine = kiss_routine_get_by_id (id);

    if (routine != 0) {
        out = routine->get_result_fn();
    }

    return out;
}   
