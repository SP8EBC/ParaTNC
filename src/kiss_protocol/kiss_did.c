/*
 * kiss_did.c
 *
 *	Implementation of all arrays defining data source for DIDs and function
 *	which generate raw binary data to be returned to a tester
 *
 *  Created on: Jun 27, 2023
 *      Author: mateusz
 */

#include <etc/kiss_did_configuration.h>
#include <etc/kiss_configuation.h>

#ifdef STM32F10X_MD_VL
#include <stm32f10x.h>
#define SRAM1_SIZE_MAX	(0x00002000UL) /*!< maximum SRAM1 size (up to 96 KBytes) */
#endif

#ifdef STM32L471xx
#include <stm32l4xx.h>
#endif

//!< Dummy variable used only as end of definition marker in tables
char did_dummy_data;

//!< Structure to define a DID which return numeric data
typedef struct kiss_did_numeric_definition_t {
	uint16_t identifier;
	void* first_data;
	uint8_t first_data_size;
	void* second_data;
	uint8_t second_data_size;
	void* third_data;
	uint8_t third_data_size;
}kiss_did_numeric_definition_t;

//!< Definition of all DIDs with numeric data
const static kiss_did_numeric_definition_t kiss_did_def[] = {
		DIDS_NUMERIC(DID_NUMERIC_DEFINITION_EXPANDER)
};

//!< Mapping between a result of sizeof operator and a value of sizebyte
const static uint8_t kiss_did_sizeof_to_sizebyte_mapping[5] = {
		0,// nothing
		1,	// int8_t
		2,	// int16_t
		0,	// nothing
		3	// int32_t
};

/**
 * Checks how many variables will be returned by this DID
 * @param definition
 * @return
 */
static uint8_t kiss_did_how_much_data(kiss_did_numeric_definition_t * definition) {

	int out = 0;

	if (definition != 0) {
		if (definition->first_data != 0x00) {
			out++;

			if (definition->second_data != 0x00) {
				out++;

				if (definition->third_data != 0x00) {
					out++;
				}
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
static int kiss_did_validate(kiss_did_numeric_definition_t * definition, uint8_t * amount) {

	int out = 0;

	uint8_t amount_of_data = 0;

	// check if pointer is valid
	if (definition != 0) {

		// check how many variables will be returned by this DID
		amount_of_data = kiss_did_how_much_data(definition);

		// if first DID is defined
		if (amount_of_data > 0) {

			// check if DID data size an address is correct
			if 	((definition->first_data_size == 0 ||
				definition->first_data_size == 1 ||
				definition->first_data_size == 4) &&
						(uint32_t)definition->first_data > SRAM_BASE &&
						(uint32_t)definition->first_data < SRAM_BASE + SRAM1_SIZE_MAX) {

				// valid
				out = 1;

				// if second did is also defined
				if (amount_of_data > 1) {
					// check if DID data size is correct
					if 	((definition->second_data_size == 0 ||
						definition->second_data_size == 1 ||
						definition->second_data_size == 4) &&
							(uint32_t)definition->second_data > SRAM_BASE &&
							(uint32_t)definition->second_data < SRAM_BASE + SRAM1_SIZE_MAX) {

						// valid
						out = 1;

						// if third DID source is also defined
						if (amount_of_data > 2) {

							// check third DID source data size
							if 	((definition->third_data_size == 0 ||
								definition->third_data_size == 1 ||
								definition->third_data_size == 4)   &&
									(uint32_t)definition->third_data > SRAM_BASE &&
									(uint32_t)definition->third_data < SRAM_BASE + SRAM1_SIZE_MAX) {

								// valid
								out = 1;
							}
							else {
								out = 0;
							}
						}
						else {
							;	// third data source doesn't need to be defined
						}
					}
					else {
						// not valid, zero output and do nothing more
						out = 0;
					}
				}
				else {
					;	// second did source doesn't need to be defined
						// so do nothing here
				}
			}
			else {
				out = 0;
			}
		}
		else {
			;	// at least one DID must be defined
				// keep out set to zero
		}

	}

	if (amount != 0) {
		*amount = amount_of_data;
	}

	return out;
}

/**
 * Creates a response for DID request into specified buffer. Please take into account
 * that this function cannot use full size of this buffer. The exact content of the
 * response is: FEND, NONSTANDARD, KISS_READ_DID_RESP, DID LSB, DID MSB, data, FEND.
 * So in practice payload data may use only buffer_ln - 6 bytes of data.
 *
 * @param identifier
 * @param output_buffer pointer to buffer where response will be generated into
 * @param buffer_ln	a size of this buffer, please be aware of note about available buffer!
 * @return Zero if DID hasn't been found in definitions, otherwise total data lenght
 */
uint8_t kiss_did_response(uint16_t identifier, uint8_t * output_buffer, uint16_t buffer_ln) {

	uint8_t out = 0;

	// iterator to go through DID definition
	int i = 0;

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
	const int is_valid = kiss_did_validate(&found, &number_of_data_source);

	// if something has been found and it is valid
	if (found.identifier != 0xFFFFu && is_valid == 1) {

		// set the sign bit in size_byte to distinguish that from ASCII did.
		// ASCII characters are from range 0 - 127 so they never have that
		// set to one
		size_byte |= 0x80u;

		// append a size of first data source
		size_byte |= kiss_did_sizeof_to_sizebyte_mapping[found.first_data_size];

		// append a size of second data source
		size_byte |= (kiss_did_sizeof_to_sizebyte_mapping[found.first_data_size] << 2);

		// append a size of third data source
		size_byte |= (kiss_did_sizeof_to_sizebyte_mapping[found.first_data_size] << 4);

		output_buffer[0] = size_byte;

		output_buffer++;

		//append first data source
		memcpy(output_buffer, found.first_data, found.first_data_size);

		//move forward a poiner to response buffer
		output_buffer += found.first_data_size;

		out += found.first_data_size;

		if (number_of_data_source > 1) {
			//append second data source
			memcpy(output_buffer, found.second_data, found.second_data_size);

			//move forward a poiner to response buffer
			output_buffer += found.second_data_size;

			out += found.second_data_size;
		}

		if (number_of_data_source > 2) {
			//append third data source
			memcpy(output_buffer, found.third_data, found.third_data_size);

			//move forward a poitner to response buffer
			output_buffer += found.third_data_size;

			out += found.third_data_size;
		}

		// also include size_byte in this calculation
		out++;

	}

	return out;

}
