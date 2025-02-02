/*
 * fanet_serialization.h
 *
 *	Serialization and deserialization
 *
 *  Created on: Jan 31, 2025
 *      Author: mateusz
 */

#ifndef SKYTRAX_FANET_FANET_SERIALIZATION_H_
#define SKYTRAX_FANET_FANET_SERIALIZATION_H_

#include "./skytrax_fanet/types/fanet_frame_t.h"

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
 * Serializes FANET frame into buffer of bytes, which then can be directly used to transmit
 * through SX1262 or similar radio IC.
 * @param input
 * @param output
 * @param output_size
 * @return positive value is a length of the output data after successful serialization, negative
 * value means an error
 */
int16_t fanet_serialize (fanet_frame_t *input, uint8_t *output, uint8_t output_size);

/**
 *
 * @param input
 * @param input_size
 * @param output
 * @return
 */
int32_t fanet_deserialize(uint8_t * input, uint8_t input_size, fanet_frame_t * output);

#endif /* SKYTRAX_FANET_FANET_SERIALIZATION_H_ */
