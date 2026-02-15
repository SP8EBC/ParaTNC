/*
 * kiss_did.c
 *
 *	Implementation of all arrays defining data source for DIDs and function
 *	which generate raw binary data to be returned to a tester
 *
 *  Created on: Jun 27, 2023
 *      Author: mateusz
 */

#include <etc/kiss_configuation.h>
#include <etc/kiss_did_configuration.h>

#include "variant.h"

#include <stm32l4xx.h>
#include <string.h>

/// ==================================================================================================
///	LOCAL DEFINITIONS
/// ==================================================================================================

#define KISS_DID_SIZE_MAPPING_INT8	1
#define KISS_DID_SIZE_MAPPING_INT16 2
#define KISS_DID_SIZE_MAPPING_INT32 3

/// ==================================================================================================
///	LOCAL DATA TYPES
/// ==================================================================================================

//!< Structure to define a DID which return numeric data
typedef struct kiss_did_numeric_definition_t {
	uint16_t identifier;
	void *first_data;
	uint8_t first_data_size;
	void *second_data;
	uint8_t second_data_size;
	void *third_data;
	uint8_t third_data_size;
} kiss_did_numeric_definition_t;

/// ==================================================================================================
///	LOCAL VARIABLES
/// ==================================================================================================

//!< Definition of all DIDs with numeric data
const static kiss_did_numeric_definition_t kiss_did_def[] = {
	DIDS_STRING (DID_NUMERIC_STRING_DEFINITION_EXPANDER)
		DIDS_FLOAT (DID_NUMERIC_FLOAT_DEFINITION_EXPANDER)
			DIDS_NUMERIC (DID_NUMERIC_DEFINITION_EXPANDER)};

//!< Mapping between a result of sizeof operator and a value of sizebyte
const static uint8_t kiss_did_sizeof_to_sizebyte_mapping[5] = {
	0,							 // nothing
	KISS_DID_SIZE_MAPPING_INT8,	 // int8_t	-> 1
	KISS_DID_SIZE_MAPPING_INT16, // int16_t	-> 2
	0,							 // nothing
	KISS_DID_SIZE_MAPPING_INT32	 // int32_t	-> 3
};

/// ==================================================================================================
///	GLOBAL VARIABLES
/// ==================================================================================================

//!< Dummy variable used only as end of definition marker in tables
char did_dummy_data;

/// ==================================================================================================
///	LOCAL FUNCTIONS
/// ==================================================================================================

/**
 * Checks how many variables will be returned by this DID
 * @param definition
 * @return
 */
static uint8_t kiss_did_how_much_data (kiss_did_numeric_definition_t *definition)
{

	int out = 0;

	if (definition != 0) {
		if (definition->first_data != &DID_EMPTY) {
			out++;

			if (definition->second_data != &DID_EMPTY) {
				out++;

				if (definition->third_data != &DID_EMPTY) {
					out++;
				}
				else {
					// zero data size as DID_EMPTY is only a dummy placeholder
					definition->third_data_size = 0;
				}
			}
			else {
				// zero data size as DID_EMPTY is only a dummy placeholder
				definition->third_data_size = 0;
				definition->second_data_size = 0;
			}
		}
	}

	return out;
}

/**
 * Checks if this DID defines data with float
 * @param definition
 * @return
 */
static int kiss_did_validate_is_float (kiss_did_numeric_definition_t *definition)
{
	int out = 0;

	if (definition != 0) {
		if (kiss_did_how_much_data (definition) >= 1) {
			if (definition->first_data_size == 0 && definition->second_data_size == 0 &&
				definition->third_data_size == 0) {
				out = 1;
			}
		}
	}

	return out;
}

/**
 * Check if this DID return string data
 * @param definition
 * @return
 */
static int kiss_did_validate_is_string (kiss_did_numeric_definition_t *definition)
{
	int out = 0;

	if (definition != 0) {
		if (kiss_did_how_much_data (definition) == 3) {
			if (definition->second_data == (void *)0xDEADBEEFu &&
				definition->third_data == (void *)0xDEADBEEFu) {
				out = 1;
			}
		}
	}

	return out;
}

/**
 * Checks if DID configuration is anything valid
 * @param definition
 * @return
 */
static int kiss_did_validate (kiss_did_numeric_definition_t *definition, uint8_t *amount)
{

	int out = 0;

	uint8_t amount_of_data = 0;

	// check if pointer is valid
	if (definition != 0) {

		// check how many variables will be returned by this DID
		amount_of_data = kiss_did_how_much_data (definition);

		// if first DID is defined
		if (amount_of_data > 0) {

			// check if DID data size an address is correct
			if ((definition->first_data_size == sizeof (int8_t) ||
				 definition->first_data_size == sizeof (int16_t) ||
				 definition->first_data_size == sizeof (int32_t)) &&
				variant_validate_is_within_ram (definition->first_data)) {

				// valid
				out = 1;

				// if second did is also defined
				if (amount_of_data > 1) {
					// check if DID data size is correct
					if ((definition->first_data_size == sizeof (int8_t) ||
						 definition->first_data_size == sizeof (int16_t) ||
						 definition->first_data_size == sizeof (int32_t)) &&
						variant_validate_is_within_ram (definition->second_data)) {

						// valid
						out = 1;

						// if third DID source is also defined
						if (amount_of_data > 2) {

							// check third DID source data size
							if ((definition->first_data_size == sizeof (int8_t) ||
								 definition->first_data_size == sizeof (int16_t) ||
								 definition->first_data_size == sizeof (int32_t)) &&
								variant_validate_is_within_ram (definition->third_data)) {

								// valid
								out = 1;
							}
							else {
								out = 0;
							}
						}
						else {
							; // third data source doesn't need to be defined
						}
					}
					else {
						// not valid, zero output and do nothing more
						out = 0;
					}
				}
				else {
					; // second did source doesn't need to be defined
					  // so do nothing here
				}
			}
			else {
				out = 0;
			}
		}
		else {
			; // at least one DID must be defined
			  // keep out set to zero
		}
	}

	// special case for float DIDs
	if (out == 0) {
		if (kiss_did_validate_is_float (definition) != 0) {
			out = 1;
		}
	}

	if (amount != 0) {
		*amount = amount_of_data;
	}

	return out;
}

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

/**
 * Creates a response for DID request into specified buffer. Please take into account
 * that this function cannot use full size of this buffer. The exact content of the
 * response is: FEND, NONSTANDARD, RESPONSE_LN, KISS_READ_DID_RESP, DID LSB, DID MSB,
 * data, FEND.
 * So in practice payload data may use only buffer_ln - 7 bytes of data. By default
 * an output buffer used to return a response has 20 bytes in size
 *
 * @param identifier
 * @param output_buffer pointer to buffer where response will be generated into
 * @param buffer_ln	a size of this buffer, please be aware of note about available buffer!
 * @return Zero if DID hasn't been found in definitions, otherwise total data lenght
 */
uint8_t kiss_did_response (uint16_t identifier, uint8_t *output_buffer, uint16_t buffer_ln)
{

	uint8_t out = 0;

	// iterator to go through DID definition
	int i = 0;

	/**
	 * Explanation how size_byte works. DID can return three kinds of data:
	 * a string, integer data (one, two or three) with different width,
	 * float (also one, two or three). size_byte is used to distinguish
	 * between these three formats. It works like that
	 *
	 * 0y - If most significant bit is set to zero, size_byte will
	 * 		signalize string DID as ASCII characters from basic ASIC
	 * 		table are from range 0 to 127
	 * 10 - If most significant bit is set to one AND next significant
	 * 		bit is set to zero this DID returns integer data. In such
	 * 		case three groups of two bits controls a size of data according
	 * 		to 'kiss_did_sizeof_to_sizebyte_mapping'. If a group of two bits
	 * 		is set to 0 it means that DID consist less than 3 variables
	 * 11 - This means that a DID return float data. One did cannot mix
	 * 		integer and floating point data and float has always 32 bit wide.
	 * 		The rest of size_bute doesn't specify any size. Less significant
	 * 		part consist number how many variables are returned.
	 */

	// first byte of a DID response which defines size of each field
	uint8_t size_byte = 0u;

	uint8_t number_of_data_source = 0u;

	// data of a did found in configuration
	kiss_did_numeric_definition_t found = {.identifier = 0xFFFFu};

	// iterate through DID definition and check if this id exist there
	do {
		// check if current position is the DID we're looking for
		if (kiss_did_def[i].identifier == identifier) {
			// if yes copy this data and exit the loop
			found = kiss_did_def[i];

			break;
		}
		else {
			// if no, go to the next DID definition
			i++;
		}
	} while (kiss_did_def[i].identifier != 0xFFFFU);

	// check is valid
	const int is_valid = kiss_did_validate (&found, &number_of_data_source);

	// check if this is string did
	const int is_string = kiss_did_validate_is_string (&found);

	// check if this is float number did
	const int is_float = kiss_did_validate_is_float (&found);

	// if something has been found and it is valid
	if (found.identifier != 0xFFFFu && is_valid == 1 && is_float == 0) {

		// put DID itself at the begining of a response
		output_buffer[0] = (identifier & 0xFF);
		output_buffer[1] = (identifier & 0xFF00) >> 8;

		// set the sign bit in size_byte to distinguish that from ASCII did.
		// ASCII characters are from range 0 - 127 so they never have that
		// set to one
		size_byte |= 0x80u;

		// append a size of first data source
		size_byte |= kiss_did_sizeof_to_sizebyte_mapping[found.first_data_size];

		// append a size of second data source
		size_byte |= (kiss_did_sizeof_to_sizebyte_mapping[found.second_data_size] << 2);

		// append a size of third data source
		size_byte |= (kiss_did_sizeof_to_sizebyte_mapping[found.third_data_size] << 4);

		output_buffer[2] = size_byte;

		// move after DID value and size_byte
		output_buffer += 3;

		// room for size byte
		out++;

		// and for DID value itself
		out += 2;

		// append first data source
		memcpy (output_buffer, found.first_data, found.first_data_size);

		// move forward a poiner to response buffer
		output_buffer += found.first_data_size;

		// room for first value returned by DID
		out += found.first_data_size;

		if (number_of_data_source > 1) {
			// append second data source
			memcpy (output_buffer, found.second_data, found.second_data_size);

			// move forward a poiner to response buffer
			output_buffer += found.second_data_size;

			out += found.second_data_size;
		}

		if (number_of_data_source > 2) {
			// append third data source
			memcpy (output_buffer, found.third_data, found.third_data_size);

			// move forward a poitner to response buffer
			output_buffer += found.third_data_size;

			out += found.third_data_size;
		}
	}
	else if (found.identifier != 0xFFFFu && is_valid == 1 && is_float == 1) {

		// put DID itself at the begining of a response
		output_buffer[0] = (identifier & 0xFF);
		output_buffer[1] = (identifier & 0xFF00) >> 8;

		// set two most significant bits to one to signalize that it is
		size_byte |= 0xC0u;

		size_byte |= number_of_data_source;

		output_buffer[2] = size_byte;

		// move after DID value and size_byte
		output_buffer += 3;

		// room for size byte stored in output buffer
		out++;

		// room for DID number in output buffer
		out += 2;

		// append first data source
		memcpy (output_buffer, found.first_data, sizeof (float));

		// move forward a poiner to response buffer
		output_buffer += sizeof (float);

		out += sizeof (float);

		if (number_of_data_source > 1) {
			// append second data source
			memcpy (output_buffer, found.second_data, sizeof (float));

			// move forward a poiner to response buffer
			output_buffer += sizeof (float);

			out += sizeof (float);
		}

		if (number_of_data_source > 2) {
			// append third data source
			memcpy (output_buffer, found.third_data, sizeof (float));

			// move forward a poitner to response buffer
			output_buffer += sizeof (float);

			out += sizeof (float);
		}
	}
	else if (found.identifier != 0xFFFFu && is_string == 1) {

		// put DID itself at the begining of a response
		output_buffer[0] = (identifier & 0xFF);
		output_buffer[1] = (identifier & 0xFF00) >> 8;

		// move after DID value and size_byte
		output_buffer += 2;

		// if this is a string DID
		const void *str = (void *)found.first_data;

		const size_t str_len = found.first_data_size;

		memset (output_buffer, 0x00, buffer_ln - 2);

		if (str_len - 2 > buffer_ln) {
			memcpy (output_buffer, str, buffer_ln - 2);

			out = buffer_ln - 2;
		}
		else {
			memcpy (output_buffer, str, str_len);

			out = str_len;
		}

		// include DID number itself
		out += 2;
	}
	else {
		out = 0;
	}

	return out;
}
