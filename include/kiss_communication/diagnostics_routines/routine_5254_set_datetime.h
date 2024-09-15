#ifndef FECBA2A0_59AB_4F1A_8C66_10091DC348D3
#define FECBA2A0_59AB_4F1A_8C66_10091DC348D3

#include "./kiss_communication/diagnostics_services/kiss_routine_control.h"

/// ==================================================================================================
///	GLOBAL MACROS
/// ==================================================================================================

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
 * Sets RTC date and time to values provided by diagnostcs call
 * @param lparam date in binary coded decimal 0xYYYYMMDD
 * @param wparam time in binary coded decimal 0xHHMM
 */
void routine_5254_start(uint32_t lparam, uint16_t wparam);

// this doesn't have stop function, because it is synchronous

/**
 * Returns zero if RTC date and time has been updated successfully, or non zero in case of error
 */
uint16_t routine_5254_get_result(void);

#endif /* FECBA2A0_59AB_4F1A_8C66_10091DC348D3 */
