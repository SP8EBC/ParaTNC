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

#define FANET_FACTORY_FRAMES_SERVICE_INTERNET_GW	(1 << 7)
#define FANET_FACTORY_FRAMES_SERVICE_TEMPERATURE	(1 << 6)
#define FANET_FACTORY_FRAMES_SERVICE_WIND			(1 << 5)
#define FANET_FACTORY_FRAMES_SERVICE_HUMIDITY		(1 << 4)
#define FANET_FACTORY_FRAMES_SERVICE_BARO			(1 << 3)
#define FANET_FACTORY_FRAMES_SERVICE_EXTHEADER		(1 << 0)

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
		retval = FANET_FACTORY_FRAMES_TYPE1_SIZE - 2;
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
		fanet_frame_t *out)
{
}

/**
 *
 * @param weather_data
 * @param out
 */
uint8_t fanet_factory_frames_weather (float latitude, float longitude, fanet_wx_input_t *weather_data, fanet_frame_t *out)
{

	uint8_t *buffer = out->payload;

	/**
	 * [Byte 0]	Header	(additional payload will be added in order 6 to 1, followed by Extended Header payload 7 to 0 once defined)
	 * 	bit 7		Internet Gateway (no additional payload required, other than a position)
	 * 	bit 6		Temperature (+1byte in 0.5 degree, 2-Complement)
	 * 	bit 5		Wind (+3byte: 1byte Heading in 360/256 degree, 1byte speed and 1byte gusts in 0.2km/h (each: bit 7 scale 5x or 1x, bit 0-6))
 	 * 	bit 4		Humidity (+1byte: in 0.4% (%rh*10/4))
	 * 	bit 3		Barometric pressure normailized (+2byte: in 10Pa, offset by 430hPa, unsigned little endian (hPa-430)*10)
	 * 	bit 2		Support for Remote Configuration (Advertisement)
	 * 	bit 1		State of Charge  (+1byte lower 4 bits: 0x00 = 0%, 0x01 = 6.666%, .. 0x0F = 100%)
	 * 	bit 0		Extended Header (+1byte directly after byte 0)
	 * 	The following is only mandatory if no additional data will be added. Broadcasting only the gateway/remote-cfg flag doesn't require pos information.
	 */
	// it seems like extended header is mandatory
	buffer[0] = FANET_FACTORY_FRAMES_SERVICE_TEMPERATURE |
			FANET_FACTORY_FRAMES_SERVICE_WIND |
			FANET_FACTORY_FRAMES_SERVICE_HUMIDITY |
			FANET_FACTORY_FRAMES_SERVICE_BARO /*|
			FANET_FACTORY_FRAMES_SERVICE_EXTHEADER*/;

	/**
	 * Extended Header:
	 *		7-6bit 		ACK:
	 *			0: none (default)
	 *			1: requested
	 *			2: requested (via forward, if received via forward (received forward bit = 0). must
	 * be used if forward is set) 			
	 * 			3: reserved

	*		5bit		Cast:
	*			0: Broadcast (default)
	*			1: Unicast (adds destination address (8+16bit)) (shall only be forwarded if dest
	* addr in cache and no 'better' retransmission received)
	* 		4bit 		Signature (if 1, add 4byte)
	* 		3bit		Geo-based Forwarded	(prevent any further geo-based forwarding, can be
	* ignored by any none-forwarding instances)
	*		2-0bit 		Reserved	(ideas: indicate multicast interest add 16bit addr, emergency)
	*
	*
	*/
	//buffer[1] = 0;

	/**
	  *	Byte 1-3 or Byte 2-4]	Position	(Little Endian, 2-Complement)
	  *	bit 0-23	Latitude	(Absolute, see below)
	  *	[Byte 4-6 or Byte 5-7]	Position	(Little Endian, 2-Complement)
	  *	bit 0-23	Longitude   (Absolute, see below)
	  */
	fanet_coordinates_absolute (latitude, longitude, &buffer[1]);

	/**
	 * Temperature (+1byte in 0.5 degree, 2-Complement)
	 */
	int16_t scaled_temperature = weather_data->temperature;

	// convert temperature from 0.1 degree resolution to 0.5 resolution
	scaled_temperature /= 5;

	// check maximum/minimum
	if (scaled_temperature > INT8_MAX) {
		buffer[7] = INT8_MAX;
	}
	else if (scaled_temperature < INT8_MIN) {
		buffer[7] = INT8_MIN;
	}
	else {
		// store rescaled temperature in output buffer
		buffer[7] = scaled_temperature;
	}

	/**
	 * Wind (+3byte: 1byte Heading in 360/256 degree, 1byte speed and 1byte gusts in 0.2km/h (each: bit 7 scale 5x or 1x, bit 0-6))
	 */
	const float winddirection_scaling = 256.0f / 360.0f;
	buffer[8] = (uint8_t)(winddirection_scaling * (float)weather_data->wind_direction);

	const float windspeed_scaling = 1.8f;
	const float high_windpseed_scaling = 0.36f;

	// windspeed
	if (weather_data->wind_average_speed < 71)
	{
		buffer[9] = (uint8_t)(windspeed_scaling * (float)weather_data->wind_average_speed);

	}
	else
	{
		buffer[9] = (uint8_t)(high_windpseed_scaling * (float)weather_data->wind_average_speed);
		buffer[9] |= (1 << 7);
	}

	// windgusts
	if (weather_data->wind_gusts < 71)
	{
		buffer[9] = (uint8_t)(windspeed_scaling * (float)weather_data->wind_gusts);

	}
	else
	{
		buffer[9] = (uint8_t)(high_windpseed_scaling * (float)weather_data->wind_gusts);
		buffer[9] |= (1 << 7);
	}

	/**
	 * Humidity (+1byte: in 0.4% (%rh*10/4))
	 */
	const float humidity_scaling = 2.5f;
	buffer[10] = (uint8_t)(humidity_scaling * (float)weather_data->humidity);

	/**
	 * Barometric pressure normailized (+2byte: in 10Pa, offset by 430hPa, unsigned little endian (hPa-430)*10)
	 */
	uint16_t scaled_pressure = (uint8_t)(weather_data->qnh - 4300ul);
	if (weather_data->qnh > 4700u)
	{
		buffer[11] = (scaled_pressure & 0xFFu);
		buffer[12] = (scaled_pressure & 0xFF00u) >> 8;
	}
	else
	{
		buffer[11] = 0u;
		buffer[12] = 0u;

	}

	/**
	 * Extended Header:
	 *	[Byte 4 (if Extended Header bit is set)]
	 *	7-6bit 		ACK:
	 *				0: none (default)
	 *				1: requested
	 *				2: requested (via forward, if received via forward (received forward bit = 0). must be used if forward is set)
	 *				3: reserved
	 *	5bit		Cast:
	 *				0: Broadcast (default)
	 *				1: Unicast (adds destination address (8+16bit)) (shall only be forwarded if dest addr in cache and no 'better' retransmission received)
	 *	4bit 		Signature (if 1, add 4byte)
	 *	3bit		Geo-based Forwarded	(prevent any further geo-based forwarding, can be ignored by any none-forwarding instances)
	 *	2-0bit 		Reserved	(ideas: indicate multicast interest add 16bit addr, emergency)
	 */
	buffer[13] = 0;

	out->payload_length = 14;

	return 14;
}
