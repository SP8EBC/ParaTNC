/*
 * fanet_internals.c
 *
 *  Created on: Feb 2, 2025
 *      Author: mateusz
 */

#include "./skytrax_fanet/fanet_internals.h"


/// ==================================================================================================
///	LOCAL DEFINITIONS
/// ==================================================================================================

/// ==================================================================================================
///	LOCAL DATA TYPES
/// ==================================================================================================

/// ==================================================================================================
///	LOCAL VARIABLES
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL VARIABLES
/// ==================================================================================================

/// ==================================================================================================
///	LOCAL FUNCTIONS
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

/**
 * Direct port of Frame::coord2payload_compressed from C++ codebase
 * @param deg
 * @return
 */
uint16_t fanet_coordinates_compressed (float deg)
{
	const float deg_round = round (deg);
	const int deg_round_int = (int)deg_round;
	uint8_t deg_odd = (deg_round_int & 1);
	const float decimal = deg - deg_round;
	const int dec_int = FANET_CONSTRAIN ((int)(decimal * 32767), -16383, 16383);

	return ((dec_int & 0x7FFF) | (!!deg_odd << 15));
}

/**
 * Direct port of Frame::coord2payload_absolut from C++ codebase
 * @param lat
 * @param lon
 * @param buf
 */
void fanet_coordinates_absolute (float lat, float lon, uint8_t *buf)
{
	int32_t lat_i = round (lat * 93206.0f);
	int32_t lon_i = round (lon * 46603.0f);

	buf[0] = ((uint8_t *)&lat_i)[0];
	buf[1] = ((uint8_t *)&lat_i)[1];
	buf[2] = ((uint8_t *)&lat_i)[2];

	buf[3] = ((uint8_t *)&lon_i)[0];
	buf[4] = ((uint8_t *)&lon_i)[1];
	buf[5] = ((uint8_t *)&lon_i)[2];
}

