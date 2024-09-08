/*
 * config_data_ptsensor.h
 *
 *  Created on: Sep 8, 2024
 *      Author: mateusz
 */

#ifndef STORED_CONFIGURATION_NVM_CONFIG_DATA_PTSENSOR_H_
#define STORED_CONFIGURATION_NVM_CONFIG_DATA_PTSENSOR_H_

/// ==================================================================================================
///	GLOBAL MACROS
/// ==================================================================================================

#define CONFIG_DATA_PTSENSOR_ASSEMBLE_CONFIG(sensor, resistor)	((sensor & 0x3) | ((resistor & 0x3F) << 2))

/// ==================================================================================================
///	GLOBAL DEFINITIONS
/// ==================================================================================================

#define CONFIG_DATA_PTSENSOR_RREF_430		(0u)
#define CONFIG_DATA_PTSENSOR_RREF_432		(1u)
#define CONFIG_DATA_PTSENSOR_RREF_442		(2u)
#define CONFIG_DATA_PTSENSOR_RREF_470		(3u)
#define CONFIG_DATA_PTSENSOR_RREF_499		(4u)
#define CONFIG_DATA_PTSENSOR_RREF_510		(5u)
#define CONFIG_DATA_PTSENSOR_RREF_560		(6u)
#define CONFIG_DATA_PTSENSOR_RREF_620		(7u)
#define CONFIG_DATA_PTSENSOR_RREF_680		(8u)
#define CONFIG_DATA_PTSENSOR_RREF_768		(9u)
#define CONFIG_DATA_PTSENSOR_RREF_1000		(10u)
#define CONFIG_DATA_PTSENSOR_RREF_1100		(11u)
#define CONFIG_DATA_PTSENSOR_RREF_1200		(12u)
#define CONFIG_DATA_PTSENSOR_RREF_1300		(13u)
#define CONFIG_DATA_PTSENSOR_RREF_1400		(14u)
#define CONFIG_DATA_PTSENSOR_RREF_1500		(15u)
#define CONFIG_DATA_PTSENSOR_RREF_1600		(16u)
#define CONFIG_DATA_PTSENSOR_RREF_1740		(17u)
#define CONFIG_DATA_PTSENSOR_RREF_1800		(18u)
#define CONFIG_DATA_PTSENSOR_RREF_1910		(19u)
#define CONFIG_DATA_PTSENSOR_RREF_2000		(20u)
#define CONFIG_DATA_PTSENSOR_RREF_2100		(21u)
#define CONFIG_DATA_PTSENSOR_RREF_2400		(22u)
#define CONFIG_DATA_PTSENSOR_RREF_2700		(23u)
#define CONFIG_DATA_PTSENSOR_RREF_3000		(24u)
#define CONFIG_DATA_PTSENSOR_RREF_3090		(25u)
#define CONFIG_DATA_PTSENSOR_RREF_3400		(26u)
#define CONFIG_DATA_PTSENSOR_RREF_3900		(27u)
#define CONFIG_DATA_PTSENSOR_RREF_4300		(28u)
#define CONFIG_DATA_PTSENSOR_RREF_4700		(29u)
#define CONFIG_DATA_PTSENSOR_RREF_4990		(30u)

#define CONFIG_DATA_PTSENSOR_3WIRE			(3u)
#define CONFIG_DATA_PTSENSOR_4_OR_2WIRE		(1u)

/// ==================================================================================================
///	GLOBAL TYPEDEFS
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL VARIABLES
/// ==================================================================================================


/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================


#endif /* STORED_CONFIGURATION_NVM_CONFIG_DATA_PTSENSOR_H_ */
