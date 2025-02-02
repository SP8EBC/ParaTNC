/*
 * fanet_mac_address_t.h
 *
 *  Created on: Jan 30, 2025
 *      Author: mateusz
 */

#ifndef SKYTRAX_FANET_FANET_MAC_ADDRESS_T_H_
#define SKYTRAX_FANET_FANET_MAC_ADDRESS_T_H_

#include <stdint.h>

/// ==================================================================================================
///	GLOBAL MACROS
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL DEFINITIONS
/// ==================================================================================================

#define FANET_MAC_ADDRESS_MANUFACTR_RESERVED		(0x00)
#define FANET_MAC_ADDRESS_MANUFACTR_SKYTRAXX		(0x01)
#define FANET_MAC_ADDRESS_MANUFACTR_BITBROKER_EU	(0x02)
#define FANET_MAC_ADDRESS_MANUFACTR_AIRWHERE		(0x04)
#define FANET_MAC_ADDRESS_MANUFACTR_WINDLINE		(0x05)
#define FANET_MAC_ADDRESS_MANUFACTR_BURNAIR_CH		(0x06)
#define FANET_MAC_ADDRESS_MANUFACTR_SOFTRF			(0x07)
#define FANET_MAC_ADDRESS_MANUFACTR_GXAIRCOM		(0x08)
#define FANET_MAC_ADDRESS_MANUFACTR_AIRTRIBUNE		(0x09)
#define FANET_MAC_ADDRESS_MANUFACTR_FLARM			(0x0A)
#define FANET_MAC_ADDRESS_MANUFACTR_FLYBEEPER		(0x0B)
#define FANET_MAC_ADDRESS_MANUFACTR_ALFAPILOT		(0x10)
#define FANET_MAC_ADDRESS_MANUFACTR_FANETPLUS		(0x11)
#define FANET_MAC_ADDRESS_MANUFACTR_XCTRACER		(0x20)
#define FANET_MAC_ADDRESS_MANUFACTR_CLOUDBUDDY		(0xCB)
#define FANET_MAC_ADDRESS_MANUFACTR_OGN_TRACKER		(0xE0)
#define FANET_MAC_ADDRESS_MANUFACTR_4AVIATION		(0xE4)
#define FANET_MAC_ADDRESS_MANUFACTR_VARIOUS			(0xFA)

/// ==================================================================================================
///	GLOBAL TYPEDEFS
/// ==================================================================================================

typedef struct fanet_mac_adress_t {

	int16_t manufacturer;	//!< zero for broadcast
	int16_t id;				//!< zero for broadcast

}fanet_mac_adress_t;

/// ==================================================================================================
///	GLOBAL VARIABLES
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================


#endif /* SKYTRAX_FANET_FANET_MAC_ADDRESS_T_H_ */
