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
const kiss_did_numeric_definition_t kiss_did_def[] = {
		DIDS_NUMERIC(DID_NUMERIC_DEFINITION_EXPANDER)
};

/**
 * Creates a response for DID request into specified buffer. Please take into account
 * that this function cannot use full size of this buffer. The exact content of the
 * response is: FEND, NONSTANDARD, KISS_READ_DID_RESP, DID LSB, DID MSB, data, FEND.
 * So in practice payload data may use only buffer_ln - 6 bytes of data.
 *
 * @param identifier
 * @param output_buffer pointer to buffer where response will be generated into
 * @param buffer_ln	a size of this buffer, please be aware of note about available buffer!
 */
void kiss_did_response(uint16_t identifier, uint8_t * output_buffer, uint16_t buffer_ln) {

	// iterate through DID definition and check if this id exist there
}
