/*
 * fanet_factory_frames.h
 *
 *  Created on: Feb 1, 2025
 *      Author: mateusz
 */

#ifndef SKYTRAX_FANET_FANET_FACTORY_FRAMES_H_
#define SKYTRAX_FANET_FANET_FACTORY_FRAMES_H_

#include "./skytrax_fanet/types/fanet_aircraft_stv_t.h"
#include "./skytrax_fanet/types/fanet_aircraft_t.h"
#include "./skytrax_fanet/types/fanet_frame_t.h"
#include "./skytrax_fanet/types/fanet_wx_input_t.h"

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
 * Port of int App::serializeTracking(uint8_t*& buffer)
 * @param type
 * @param state_vector
 * @param out
 * @return
 */
uint32_t fanet_factory_frames_tracking (fanet_aircraft_t type, fanet_aircraft_stv_t *state_vector,
									fanet_frame_t *out);

/**
 *
 * @param latitude
 * @param longitude
 * @param online
 * @param out
 */
void fanet_factory_frames_ground (float latitude, float longitude, uint8_t online,
								  fanet_frame_type_t *out);

/**
 *
 * @param weather_data
 * @param out
 */
void fanet_factory_frames_weather (fanet_wx_input_t *weather_data, fanet_frame_type_t *out);

#endif /* SKYTRAX_FANET_FANET_FACTORY_FRAMES_H_ */
