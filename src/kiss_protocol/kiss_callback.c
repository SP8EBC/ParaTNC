/*
 * kiss_callback.c
 *
 * This file contains callback to all async sudo-UDS services requests. Each
 * callback takes pointer for buffer and response. It returns a size of a
 * response.
 *
 *  Created on: Aug 17, 2022
 *      Author: mateusz
 */

#include <kiss_communication/kiss_communication.h>
#include <kiss_communication/types/kiss_communication_service_ids.h>
#include <kiss_communication/kiss_did.h>
#include <kiss_communication/kiss_nrc_response.h>
#include "main.h"

#include <string.h>
#include <stdio.h>
#include <stored_configuration_nvm/configuration_handler.h>

#define KISS_MAX_CONFIG_PAYLOAD_SIZE	0x80
#define KISS_LAST_ASYNC_MSG				0xFF

uint8_t kiss_async_message_counter = 0;

int32_t kiss_callback_get_running_config(uint8_t* input_frame_from_host, uint16_t input_len, uint8_t* response_buffer, uint16_t buffer_size) {

	#define CALLBACK_GET_RUNNING_CFG_LN	7

	uint32_t conf_size = 0;

	configuration_handler_region_t current_region;

	// set current message to start pooling
	kiss_current_async_message = KISS_RUNNING_CONFIG;

	// the rest of content of an input frame is irrevelant, but we need to send
	// a response telling how long configuration data is
	memset(response_buffer, 0x00, buffer_size);

	// get currently used configuration and its size in flash memory
	current_region = configuration_get_current(&conf_size);

	// reset async message counter
	kiss_async_message_counter = 0;

	// construct a response
	response_buffer[0] = FEND;
	response_buffer[1] = NONSTANDARD;
	response_buffer[2] = CALLBACK_GET_RUNNING_CFG_LN;				// message lenght
	response_buffer[3] = KISS_RUNNING_CONFIG;
	response_buffer[4] = (uint8_t)(current_region & 0xFF);
	if ((conf_size % KISS_MAX_CONFIG_PAYLOAD_SIZE) == 0)
		response_buffer[5] = (uint8_t)(conf_size / KISS_MAX_CONFIG_PAYLOAD_SIZE);
	else
		response_buffer[5] = (uint8_t)(conf_size / KISS_MAX_CONFIG_PAYLOAD_SIZE + 1);
	response_buffer[6] = FEND;

	return CALLBACK_GET_RUNNING_CFG_LN;

}

int16_t kiss_pool_callback_get_running_config(uint8_t * output_buffer, uint16_t buffer_size){

	uint32_t conf_size = 0, offset = 0;

	configuration_handler_region_t current_region;

	uint8_t config_payload_size = 0;

	const uint8_t * config_base_address = 0;

	// terminate if no more packets needs to be send
	if (kiss_async_message_counter == KISS_LAST_ASYNC_MSG) {
		return 0;
	}

	// get currently used configuration and its size in flash memory
	current_region = configuration_get_current(&conf_size);

	// get base address of current config
	config_base_address = (const uint8_t *)configuration_get_address(current_region);

	if (kiss_async_message_counter * KISS_MAX_CONFIG_PAYLOAD_SIZE < conf_size) {
		config_payload_size = KISS_MAX_CONFIG_PAYLOAD_SIZE;

		// offset in source configuration area
		offset = kiss_async_message_counter * KISS_MAX_CONFIG_PAYLOAD_SIZE;

		kiss_async_message_counter++;
	}
	else {
		config_payload_size = conf_size - (kiss_async_message_counter * KISS_MAX_CONFIG_PAYLOAD_SIZE);

		// offset in source configuration area
		offset = kiss_async_message_counter * KISS_MAX_CONFIG_PAYLOAD_SIZE;

		kiss_async_message_counter = KISS_LAST_ASYNC_MSG;
	}

	// a case for 'config_size' being a multiply of 'KISS_MAX_CONFIG_PAYLOAD_SIZE'
	if (config_payload_size == 0) {
		return 0;
	}

	// place KISS header
	output_buffer[0] = FEND;
	output_buffer[1] = NONSTANDARD;
	output_buffer[2] = config_payload_size + 7;
	output_buffer[3] = KISS_RUNNING_CONFIG;
	output_buffer[4] = 0xAB;				// THIS IS A DATA FRAME, not ACK
	output_buffer[5] = kiss_async_message_counter - 1;	// frame sequence number

	memcpy(output_buffer + 6, config_base_address + offset, config_payload_size);

	output_buffer[config_payload_size + 6] = FEND;

	return config_payload_size + 7;
}

int32_t kiss_callback_get_version_id(uint8_t* input_frame_from_host, uint16_t input_len, uint8_t* response_buffer, uint16_t buffer_size) {

	uint8_t config_payload_size = 0;

#ifdef PARAMETEO
	config_payload_size = snprintf((char *)response_buffer + 4, buffer_size, "METEO-%s-%s", SW_VER, SW_KISS_PROTO);
#else
	config_payload_size = snprintf((char *)response_buffer + 4, buffer_size, "TNC-%s-%s", SW_VER, SW_KISS_PROTO);
#endif

	// construct a response
	response_buffer[0] = FEND;
	response_buffer[1] = NONSTANDARD;
	response_buffer[2] = config_payload_size + 5;				// message lenght
	response_buffer[3] = KISS_VERSION_AND_ID;
	// string here
	response_buffer[config_payload_size + 4] = FEND;

	return config_payload_size + 5;

}

int32_t kiss_callback_erase_startup(uint8_t* input_frame_from_host, uint16_t input_len, uint8_t* response_buffer, uint16_t buffer_size) {

#define ERASE_STARTUP_LN	6

	kiss_communication_nrc_t result = configuration_handler_erase_startup();

	response_buffer[0] = FEND;
	response_buffer[1] = NONSTANDARD;
	response_buffer[2] = ERASE_STARTUP_LN;				// message lenght

	if (result == NRC_POSITIVE) {
		// construct a response
		response_buffer[3] = KISS_ERASE_STARTUP_CFG_RESP;
	}
	else {
		response_buffer[3] = KISS_NEGATIVE_RESPONSE_SERVICE;

	}

	response_buffer[4] = (uint8_t)result;
	response_buffer[5] = FEND;

	return ERASE_STARTUP_LN;

}

/**
 * Callback which program configuration data block received from the Host PC. Please bear in mind that the TNC doesn't really take care
 * what it receives and program. It is up to host PC to provide senseful configuration with properly calculated checksum as this isn't
 * recalculated during programming.
 */
int32_t kiss_callback_program_startup(uint8_t* input_frame_from_host, uint16_t input_len, uint8_t* response_buffer, uint16_t buffer_size) {

#define PROGRAM_STARTUP_LN	6

	/**
	 * The structure of an input frame goes like that:
	 * FEND, KISS_PROGRAM_STARTUP_CFG, data_PAYLOAD_LN, OFFSET_LSB, OFFSET_MSB, data, data, (...), FEND
	 *
	 * KISS_PROGRAM_STARTUP_CFG is a frame type, in this case 0x34, but might be also 0x00 for regular
	 * frame data to be sent over the air. data_PAYLOAD_LN is a lenght of data in this frame, the
	 * assumption is  that whole config data segment size will be an even multiply of a size
	 * of single frame.
	 */

	// result to be returned to the host PC
	kiss_communication_nrc_t result;

	// offset within input frame where config start begining
	uint8_t * data_ptr = input_frame_from_host + 5;

	// size of data to be programmed into flash memory
	uint8_t data_size =  *(input_frame_from_host + 2);

	uint16_t config_block_offset = *(input_frame_from_host + 3) | (*(input_frame_from_host + 4) << 8);

	result = configuration_handler_program_startup(data_ptr, data_size, config_block_offset);

	// construct a response
	response_buffer[0] = FEND;
	response_buffer[1] = NONSTANDARD;
	response_buffer[2] = PROGRAM_STARTUP_LN;				// message lenght

	if (result == NRC_POSITIVE) {
		// construct a response
		response_buffer[3] = KISS_PROGRAM_STARTUP_CFG_RESP;
	}
	else {
		response_buffer[3] = KISS_NEGATIVE_RESPONSE_SERVICE;

	}

	response_buffer[4] = (uint8_t)result;
	response_buffer[5] = FEND;

	return PROGRAM_STARTUP_LN;
}

int32_t kiss_callback_read_did(uint8_t* input_frame_from_host, uint16_t input_len, uint8_t* response_buffer, uint16_t buffer_size) {

	/**
	 * Response frame structure
	 *
	 * FEND, NONSTANDARD, RESPONSE_SIZE, KISS_READ_DID_RESP, DID_lsb, DID_msb, size_byte, DATA (...), FEND
	 */

	int32_t out = 0;

	memset(response_buffer, 0x00, buffer_size);

	// identifier
	uint16_t did = *(input_frame_from_host + 2) | (*(input_frame_from_host + 3) << 8);

	// construct DID response to an output buffer
	const uint8_t response_size = kiss_did_response(did, response_buffer + 4, buffer_size - 5);

	// check if DID has been found and everyting is OK with it.
	if (response_size > 0) {
		// if response is correct fill the buffer with the rest of stuff
		response_buffer[0] = FEND;
		response_buffer[1] = NONSTANDARD;
		response_buffer[2] = response_size + 5;				// message lenght
		response_buffer[3] = KISS_READ_DID_RESP;

		response_buffer[response_size + 4] = FEND;

		out = response_size + 5;
	}
	else {
		out = kiss_nrc_response_fill_request_out_of_range(response_buffer);
	}

	return out;
}
