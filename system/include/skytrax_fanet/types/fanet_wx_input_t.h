/*
 * fanet_wx_input_t.h
 *
 *  Created on: Feb 1, 2025
 *      Author: mateusz
 */

#ifndef SKYTRAX_FANET_TYPES_FANET_WX_INPUT_T_H_
#define SKYTRAX_FANET_TYPES_FANET_WX_INPUT_T_H_

#include <stdint.h>

/// ==================================================================================================
///	GLOBAL MACROS
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL DEFINITIONS
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL TYPEDEFS
/// ==================================================================================================

/**
 * Condensed weather measurements to pass as an input to generate FANET weather
 * frame for. Please note that values must be specifically scaled to integer values.
 * Internally there are rescaled one more time to fit with FANET specification
 */
typedef struct fanet_wx_input_t {
	int16_t temperature;
	uint16_t wind_direction;
	uint16_t wind_average_speed;
	uint16_t wind_gusts;
	int8_t humidity;
	uint16_t qnh;
}fanet_wx_input_t;

/// ==================================================================================================
///	GLOBAL VARIABLES
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

#endif /* SKYTRAX_FANET_TYPES_FANET_WX_INPUT_T_H_ */
