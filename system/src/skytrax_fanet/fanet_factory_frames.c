/*
 * fanet_factory_frames.c
 *
 *  Created on: Feb 2, 2025
 *      Author: mateusz
 */

#include "./skytrax_fanet/fanet_factory_frames.h"
#include "./skytrax_fanet/fanet_internals.h"

#include <math.h>

/// ==================================================================================================
///	LOCAL DEFINITIONS
/// ==================================================================================================

#define doOnlineTracking (1)

#define FANET_FACTORY_FRAMES_TYPE1_SIZE 13 // 11+2
#define FANET_FACTORY_FRAMES_TYPE7_SIZE 7

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
 * Port of int App::serializeTracking(uint8_t*& buffer) from C++ codebase into pure C
 * @param type
 * @param state_vector
 * @param out
 * @return
 */
uint32_t fanet_factory_frames_tracking (fanet_aircraft_t type, fanet_aircraft_stv_t *state_vector,
										fanet_frame_t *out)
{
	uint32_t retval = 0;

	uint8_t *buffer = out->payload;

	/** position
	 * [Byte 0-2]	Position	(Little Endian, 2-Complement)
	 * bit 0-23	Latitude 	(Absolute, see below)
	 * [Byte 3-5]	Position	(Little Endian, 2-Complement)
	 * bit 0-23	Longitude 	(Absolute, see below)
	 */
	fanet_coordinates_absolute (state_vector->latitude, state_vector->longitude, buffer);

	/* altitude set the lower 12bit */
	int alt = FANET_CONSTRAIN (state_vector->altitude, 0, 8190);
	if (alt > 2047)
		((uint16_t *)buffer)[3] = ((alt + 2) / 4) | (1 << 11); // set scale factor
	else
		((uint16_t *)buffer)[3] = alt;

	/* online tracking */
	((uint16_t *)buffer)[3] |= !!doOnlineTracking << 15;
	/* aircraft type */
	((uint16_t *)buffer)[3] |= (type & 0x7) << 12;

	/** Speed
	 * [Byte 8]	Speed		(max 317.5km/h)
	 *	bit 7		Scaling 	1->5x, 0->1x  ===> these 1x and 5x relates to a base of .5km/h
	 *	bit 0-6		Value		in 0.5km/h
	 */
	const int32_t rescaled_speed = (int32_t)round (state_vector->speed * 2.0f);
	int32_t speed2 = FANET_CONSTRAIN (rescaled_speed, 0, 635);
	if (speed2 > 127) // 93.8km/h thankfully not relevant to paraglider.
		buffer[8] = ((speed2 + 2) / 5) | (1 << 7); // set scale factor
	else
		buffer[8] = speed2;

	/** Climb
	 *  [Byte 9]	Climb		(max +/- 31.5m/s, 2-Complement)
	 *	bit 7		Scaling 	1->5x, 0->1x
	 *	bit 0-6		Value		in 0.1m/s
	 */
	const int32_t rescaled_climb = (int32_t)round (state_vector->climb * 10.0f);
	int32_t climb10 = FANET_CONSTRAIN (rescaled_climb, -315, 315);
	if (abs (climb10) > 63) // 6.3m/s
		// TODO: why so complicated? Somebody tried to make a oneliner??
		buffer[9] = ((climb10 + (climb10 >= 0 ? 2 : -2)) / 5) | (1 << 7); // set scale factor
	else
		buffer[9] = climb10 & 0x7F;

	/* Heading */
	const int32_t rescaled_heading = (int)round ((state_vector->heading * 256.0f) / 360.0f);
	buffer[10] = FANET_CONSTRAIN (rescaled_heading, 0, 255);

	/* no optional data */
	// if(std::isnan(turnrate) && qneOffset == 0)
	// 	return APP_TYPE1_SIZE - 2;

	if (state_vector->has_turnrate == 0 && state_vector->qne_offset == 0.0f) {
		out = FANET_FACTORY_FRAMES_TYPE1_SIZE - 2;
	}
	else {
		if (state_vector->has_turnrate == 0) {
			state_vector->turnrate = 0.0f;
		}

		/**
		 * [Byte 11]	Turn rate 	(max +/- 64deg/s, positive is clock wise, 2-Complement)
		 * bit 7		Scaling 	1->4x, 0->1x
	     * bit 0-6		Value 		in 0.25deg/s	
		 */
		const int32_t scaled_turnrate = (int32_t)round (state_vector->turnrate * 4.0f);
		int32_t trOs = FANET_CONSTRAIN (scaled_turnrate, -254, 254);
		if (abs (trOs) >= 63)
			buffer[11] = ((trOs + (trOs >= 0 ? 2 : -2)) / 4) | (1 << 7); // set scale factor
		else
			buffer[11] = trOs & 0x7f;

		/** QNE offset
		 * [optional, if used byte 11 is mandatory as well]
		 * [Byte 12]	QNE offset 	(=QNE-GPS altitude, max +/- 254m, 2-Complement)
		 * bit 7		Scaling 	1->4x, 0->1x
		 * bit 0-6		Value 		in m	
		 */
		if (state_vector->qne_offset != 0.0f) {
			const int32_t scaled_qne_offset = (int32_t)state_vector->qne_offset;
			int qneOs = FANET_CONSTRAIN (scaled_qne_offset, -254, 254);
			if (abs (qneOs) > 63) { // WTF???? TODO: do something with that awkward one-liner!!
				buffer[12] = ((qneOs + (qneOs >= 0 ? 2 : -2)) / 4) | (1 << 7); // set scale factor
			}
			else {
				buffer[12] = qneOs & 0x7f;
			}
			retval = FANET_FACTORY_FRAMES_TYPE1_SIZE;
		}
		else {
			retval = FANET_FACTORY_FRAMES_TYPE1_SIZE - 1;
		}
	}

	return retval;
}

/**
 *
 * @param latitude
 * @param longitude
 * @param online
 * @param out
 */
void fanet_factory_frames_ground (float latitude, float longitude, uint8_t online,
								  fanet_frame_type_t *out)
{
}

/**
 *
 * @param weather_data
 * @param out
 */
void fanet_factory_frames_weather (fanet_wx_input_t *weather_data, fanet_frame_type_t *out)
{
}
