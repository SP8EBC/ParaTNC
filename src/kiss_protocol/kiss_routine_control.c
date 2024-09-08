/*
 * kiss_diagnostic_routine_control.c
 *
 *  Created on: Sep 8, 2024
 *      Author: mateusz
 */

#include "./kiss_communication/kiss_routine_control.h"
#include "./kiss_communication/types/kiss_xmacro_helpers.h"
#include "./kiss_communication/routines/routine_5254_set_datetime.h"

#include "./etc/kiss_routine_configuration.h"

/// ==================================================================================================
///	LOCAL DEFINITIONS
/// ==================================================================================================

/// ==================================================================================================
///	LOCAL DATA TYPES
/// ==================================================================================================

//!< Definition of single diagnostics routine. It will be hardcoded as const in the flash memory
typedef struct kiss_routine_t {
    uint16_t routine_number;
    kiss_routine_control_start_t start_fn;
    kiss_routine_control_stop_t stop_fn;
    kiss_routine_control_getresult_t get_result_fn;
    uint8_t asynchronous;
} kiss_routine_control_t;

//!< Definition of runtime status for single diagnostic routine
typedef struct kiss_routine_status_t {
    uint16_t routine_number;    //!< Routine ID
    uint8_t running;            //!< if the routine is currently running
} kiss_routine_status_t;

//!< Enum definition used to count all definitions
typedef enum kiss_routine_enum_t {
    KISS_ROUTINES(KISS_DIAGNOSTIC_ROUTINE_ENUM_EXPAND)
    KISS_ROUTINE_NUMBER_COUNT
    
} kiss_routine_enum_t;

/// ==================================================================================================
///	LOCAL VARIABLES
/// ==================================================================================================

//!< Const definitions of all routine diagnostics
static const kiss_routine_control_t kiss_routine_definitions[] = {
    KISS_ROUTINES(KISS_ROUTINE_DEF_EXPAND)
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

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

/**
 * Checks if routine with given id exists and returns its type
 * @param id of routine to check
 * @returns zero if routine doesn't exists or KISS_ROUTINE_CONTROL_ASYNCHRONOUS or 
 *          KISS_ROUTINE_CONTROL_ASYNCHRONOUS_AUTOSTOP or KISS_ROUTINE_CONTROL_SYNCHRO depends on
 *          routine configuration
 */
uint8_t kiss_routine_control_check_routine_id(uint16_t id) {

    uint8_t out = 0;

    for (int i = 0; i < KISS_ROUTINE_NUMBER_COUNT; i++) {
        if (kiss_routine_definitions[i].routine_number == id) {
            out = kiss_routine_definitions[i].asynchronous;
            break;
        }
    }

    return out;
}
