/*
 * fanet_aircraft_stv_t.h
 *
 * State vector of an aircraft.
 *
 *  Created on: Feb 1, 2025
 *      Author: mateusz
 */

#ifndef SKYTRAX_FANET_TYPES_FANET_AIRCRAFT_STV_T_H_
#define SKYTRAX_FANET_TYPES_FANET_AIRCRAFT_STV_T_H_


/// ==================================================================================================
///	GLOBAL MACROS
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL DEFINITIONS
/// ==================================================================================================

/**
 * 
 */
typedef enum fanet_aircraft_state_t {
	FANET_AIRCRAFT_STATUS_OTHER = 0,
	FANET_AIRCRAFT_STATUS_HIKING = 1,
	FANET_AIRCRAFT_STATUS_VEHICLE = 2,
	FANET_AIRCRAFT_STATUS_BIKE = 3,
	FANET_AIRCRAFT_STATUS_BOOT = 4,
	FANET_AIRCRAFT_STATUS_NEED_RIDE = 8,

	FANET_AIRCRAFT_STATUS_NEED_TECHNICAL_ASSISTANCE = 12,
	FANET_AIRCRAFT_STATUS_NEED_MEFICAL_HELP = 13,
	FANET_AIRCRAFT_STATUS_DISTRESS_CALL = 14,
	FANET_AIRCRAFT_STATUS_DISTRESS_CALL_AUTO = 15,				//max number
}fanet_aircraft_state_t;

/**
 * State vector of an aircraft to generate FANET frame for
 */
typedef struct fanet_aircraft_stv_t {
	fanet_aircraft_state_t state;
	float latitude;
	float longitude;
	float altitude;
	float speed;
	float climb;
	float heading;
	float turnrate;
	float qne_offset;
	int has_turnrate;
}fanet_aircraft_stv_t;

/// ==================================================================================================
///	GLOBAL TYPEDEFS
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL VARIABLES
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================


#endif /* SKYTRAX_FANET_TYPES_FANET_AIRCRAFT_STV_T_H_ */
