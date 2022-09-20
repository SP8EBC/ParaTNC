/*
 * kiss_callback.c
 *
 *  Created on: Aug 17, 2022
 *      Author: mateusz
 */

#include "kiss_communication.h"
#include "configuration_handler.h"
#include "main.h"

#include <string.h>
#include <stdio.h>

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
	config_payload_size = snprintf((char *)response_buffer + 3, buffer_size, "METEO-%s-%s", SW_VER, SW_KISS_PROTO);
#else
	config_payload_size = snprintf((char *)response_buffer + 3, buffer_size, "TNC-%s-%s", SW_VER, SW_KISS_PROTO);
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

	configuration_erase_startup_t result = configuration_handler_erase_startup();

	// construct a response
	response_buffer[0] = FEND;
	response_buffer[1] = NONSTANDARD;
	response_buffer[2] = ERASE_STARTUP_LN;				// message lenght
	response_buffer[3] = KISS_ERASE_STARTUP_CFG_RESP;
	response_buffer[4] = result;
	response_buffer[5] = FEND;

	return ERASE_STARTUP_LN;
}

/**
 * Callback which program configuration data block received from the Host PC. Please bear in mind that the TNC doesn't really take care
 * what it receices and program. It is up to host PC to provide senseful configuration with properly calculated checksum as this isn't
 * recalculated ruing programming.
 */
int32_t kiss_callback_program_startup(uint8_t* input_frame_from_host, uint16_t input_len, uint8_t* response_buffer, uint16_t buffer_size) {

#define PROGRAM_STARTUP_LN	6

	/**
	 * The structure of input frame goes like that:
	 * FEND, LN, KISS_PROGRAM_STARTUP_CFG, OFFSET, data, data, (...), FEND
	 *
	 * LN is a lenght of complete frame, so data size is LN - 5 (two FENDs, LN itself, OFFSET and KISS_PROGRAM_STARTUP_CFG)
	 * OFFSET is an offset calculated from the begining of configuration block. Host PC doesn't know anything about TNC memory layout
	 */

	// result to be returned to the host PC
	configuration_erase_startup_t result;

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
	response_buffer[3] = KISS_PROGRAM_STARTUP_CFG_RESP;
	response_buffer[4] = result;
	response_buffer[5] = FEND;

	return PROGRAM_STARTUP_LN;
}
