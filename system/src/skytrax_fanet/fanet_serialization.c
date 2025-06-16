#include "./skytrax_fanet/fanet_serialization.h"
#include "./skytrax_fanet/fanet_internals.h"
#include <math.h>

/// ==================================================================================================
///	LOCAL DEFINITIONS
/// ==================================================================================================

/*
 * Frame
 */
#define FANET_SERIALIZE_MIN_HEADER_LENGTH (4)
#define FANET_SERIALIZE_ADDR_LENGTH		  (3)
#define FANET_SERIALIZE_SIGNATURE_LENGTH  (4)

/* Header Byte */
#define FANET_SERIALIZE_HEADER_EXTHEADER_BIT (7)
#define FANET_SERIALIZE_HEADER_FORWARD_BIT	 (6)
#define FANET_SERIALIZE_HEADER_TYPE_MASK	 (0x3F)

/* Extended Header Byte */
#define FANET_SERIALIZE_EXTHEADER_ACK_BIT1		7
#define FANET_SERIALIZE_EXTHEADER_ACK_BIT0		6
#define FANET_SERIALIZE_EXTHEADER_UNICAST_BIT	5
#define FANET_SERIALIZE_EXTHEADER_SIGNATURE_BIT 4

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
 * Serializes FANET frame into buffer of bytes, which then can be directly used to transmit
 * through SX1262 or similar radio IC.
 * @param input
 * @param output
 * @param output_size
 * @return positive value is a length of the output data after successful serialization, negative
 * value means an error
 */
int16_t fanet_serialize (fanet_frame_t *input, uint8_t *output, uint8_t output_size)
{
	const fanet_mac_adress_t *src = &input->source;
	const fanet_mac_adress_t *dest = &input->destination;
	const uint8_t masked_frame_type = input->type & FANET_SERIALIZE_HEADER_TYPE_MASK;
	const uint8_t extheader_bit = !!(input->ack_requested || dest->id != 0 || dest->manufacturer != 0 || input->signature != 0);
	const uint8_t forward_bit = !!input->forward;


	// comparison is always false due to limited range of data type [-Wtype-limits]
	if (src->id == 0 || src->id == 0xFFFF || src->manufacturer == 0 || src->manufacturer >= 0xFE) {
		return -2;
	}

	int16_t blength = FANET_SERIALIZE_MIN_HEADER_LENGTH + input->payload_length;

	/* extended header? */
	if ((input->ack_requested > FANET_ACK_NOACK) || input->destination.id != 0 ||
		input->destination.manufacturer != 0 || input->signature != 0)
		blength++;

	/* none broadcast frame */
	if (dest->id != 0 || dest->manufacturer != 0)
		blength += FANET_SERIALIZE_ADDR_LENGTH;

	/* signature */
	if (input->signature != 0)
		blength += FANET_SERIALIZE_SIGNATURE_LENGTH;

	/* frame to long */
	if (blength > output_size)
		return -1;

	/* get memory */
	int idx = 0;

	// clang-format off
	/* header */			// not sure why there is double negation here. This is something copied directly from C++ codebase
	output[idx++] = 		extheader_bit <<FANET_SERIALIZE_HEADER_EXTHEADER_BIT |
							forward_bit << FANET_SERIALIZE_HEADER_FORWARD_BIT |
							masked_frame_type;
	output[idx++] = src->manufacturer & 0x000000FF;
	output[idx++] = src->id & 0x000000FF;
	output[idx++] = (src->id >> 8) & 0x000000FF;
	// clang-format on

	/* extended header */
	if (extheader_bit) {
		output[idx++] = (input->ack_requested & 3) << FANET_SERIALIZE_EXTHEADER_ACK_BIT0 |
						!!(dest->id != 0 || dest->manufacturer != 0)
							<< FANET_SERIALIZE_EXTHEADER_UNICAST_BIT |
						!!input->signature << FANET_SERIALIZE_EXTHEADER_SIGNATURE_BIT;
	}

	/* extheader and unicast -> add destination addr */
	if (extheader_bit &&
		(output[4] & 1 << FANET_SERIALIZE_EXTHEADER_UNICAST_BIT)) {
		output[idx++] = dest->manufacturer & 0x000000FF;
		output[idx++] = dest->id & 0x000000FF;
		output[idx++] = (dest->id >> 8) & 0x000000FF;
	}

	/* extheader and signature -> add signature */
	if ((output[0] & 1 << 7) && (output[4] & 1 << 4)) {
		output[idx++] = input->signature & 0x000000FF;
		output[idx++] = (input->signature >> 8) & 0x000000FF;
		output[idx++] = (input->signature >> 16) & 0x000000FF;
		output[idx++] = (input->signature >> 24) & 0x000000FF;
	}

	/* fill payload */
	for (int i = 0; (i < input->payload_length) && (idx < blength); i++)
		output[idx++] = input->payload[i];

	return blength;
}

/**
 * Deserializes raw data received from radio 
 * @param input pointer to buffer with binary data received by radio module
 * @param input_size amount of raw data received from FANET 
 * @param output
 * @return
 */
int32_t fanet_deserialize (uint8_t *input, uint8_t input_size, fanet_frame_t *output)
{
	fanet_mac_adress_t *src = &output->source;
	fanet_mac_adress_t *dest = &output->destination;

	int payload_start = FANET_SERIALIZE_MIN_HEADER_LENGTH;

	/* header */
	output->forward = !!(input[0] & (1 << FANET_SERIALIZE_HEADER_FORWARD_BIT));
	output->type = input[0] & FANET_SERIALIZE_HEADER_TYPE_MASK;
	src->manufacturer = input[1];
	src->id = input[2] | (input[3] << 8);

	/* extended header */
	if (input[0] & 1 << FANET_SERIALIZE_HEADER_EXTHEADER_BIT) {
		payload_start++;

		/* ack type */
		output->ack_requested = (input[4] >> FANET_SERIALIZE_EXTHEADER_ACK_BIT0) & 3;

		/* unicast */
		if (input[4] & (1 << FANET_SERIALIZE_EXTHEADER_UNICAST_BIT)) {
			dest->manufacturer = input[5];
			dest->id = input[6] | (input[7] << 8);

			payload_start += FANET_SERIALIZE_ADDR_LENGTH;
		}

		/* signature */
		if (input[4] & (1 << FANET_SERIALIZE_EXTHEADER_SIGNATURE_BIT)) {
			output->signature = input[payload_start] | (input[payload_start + 1] << 8) |
						(input[payload_start + 2] << 16) | (input[payload_start + 3] << 24);
			payload_start += FANET_SERIALIZE_SIGNATURE_LENGTH;
		}
	}

	/* payload */
	output->payload_length = input_size - payload_start;
	if (output->payload_length > 0) {
		//payload = new uint8_t[payload_length];
		memcpy (output->payload, &input[payload_start], output->payload_length);
	}

	return 0;
}
