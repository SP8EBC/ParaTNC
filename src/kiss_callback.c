/*
 * kiss_callback.c
 *
 *  Created on: Aug 17, 2022
 *      Author: mateusz
 */

#include "kiss_communication.h"
#include "configuration_handler.h"

#include <string.h>

#define KISS_MAX_CONFIG_PAYLOAD_SIZE	0x80
#define KISS_LAST_ASYNC_MSG				0xFF

uint8_t kiss_async_message_counter = 0;

int32_t kiss_callback_get_running_config(uint8_t* input_frame_from_host, uint16_t input_len, uint8_t* response_buffer, uint16_t buffer_size) {

	#define CALLBACK_GET_RUNNING_CFG_LN	6

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
	response_buffer[1] = CALLBACK_GET_RUNNING_CFG_LN;				// message lenght
	response_buffer[2] = KISS_RUNNING_CONFIG;
	response_buffer[3] = (uint8_t)(current_region & 0xFF);
	if ((conf_size % KISS_MAX_CONFIG_PAYLOAD_SIZE) == 0)
		response_buffer[4] = (uint8_t)(conf_size / KISS_MAX_CONFIG_PAYLOAD_SIZE);
	else
		response_buffer[4] = (uint8_t)(conf_size / KISS_MAX_CONFIG_PAYLOAD_SIZE + 1);
	response_buffer[5] = FEND;

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

	// place KISS header
	output_buffer[0] = FEND;
	output_buffer[1] = config_payload_size + 6;
	output_buffer[2] = KISS_RUNNING_CONFIG;
	output_buffer[3] = 0xAB;				// THIS IS A DATA FRAME, not ACK
	output_buffer[4] = kiss_async_message_counter - 1;	// frame sequence number

	memcpy(output_buffer + 5, config_base_address + offset, config_payload_size);

	output_buffer[config_payload_size + 5] = FEND;

	return config_payload_size + 6;
}
