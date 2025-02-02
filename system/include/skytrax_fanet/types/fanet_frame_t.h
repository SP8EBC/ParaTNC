/*
 * fanet_frame_t.h
 *
 *  Created on: Jan 30, 2025
 *      Author: mateusz
 */

#ifndef SKYTRAX_FANET_FANET_FRAME_T_H_
#define SKYTRAX_FANET_FANET_FRAME_T_H_

#include "./skytrax_fanet/types/fanet_mac_address_t.h"
#include "./skytrax_fanet/types/fanet_ack_t.h"

/// ==================================================================================================
///	GLOBAL MACROS
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL DEFINITIONS
/// ==================================================================================================

#define FANET_FRAME_PAYLOAD_MAX_SIZE		(128u)

/// ==================================================================================================
///	GLOBAL TYPEDEFS
/// ==================================================================================================

/**
 *
 */
typedef enum fanet_frame_type_t {
	FANET_FRAME_ACK 		= 0x0,     	/**< FANET_FRAME_ACK */
	FANET_FRAME_TRACKING	= 0x1,  	/**< FANET_FRAME_TRACKING */
	FANET_FRAME_NAME		= 0x2,     	/**< FANET_FRAME_NAME */
	FANET_FRAME_MESSAGE		= 0x3,  	/**< FANET_FRAME_MESSAGE */
	FANET_FRAME_SERVICE		= 0x4,  	/**< FANET_FRAME_SERVICE */
	FANET_FRAME_LANDMARK	= 0x5,  	/**< FANET_FRAME_LANDMARK */
	FANET_FRAME_REMOTECONF	= 0x6,		/**< FANET_FRAME_REMOTECONF */
	FANET_FRAME_GNDTRACKING	= 0x7		/**< FANET_FRAME_GNDTRACKING */
}fanet_frame_type_t;

/**
 * Main type describing a deserialized content of FANET frame
 */
typedef struct fanet_frame_t {

	fanet_mac_adress_t source;
	fanet_mac_adress_t destination;

	fanet_ack_t ack_requested;
	uint8_t forward;

	uint32_t signature;

	fanet_frame_type_t type;
	uint8_t payload_length;
	uint8_t payload[FANET_FRAME_PAYLOAD_MAX_SIZE];

	int16_t num_tx;
	uint32_t next_tx;

	int16_t rssi;

}fanet_frame_t;

/// ==================================================================================================
///	GLOBAL VARIABLES
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================


#endif /* SKYTRAX_FANET_FANET_FRAME_T_H_ */
