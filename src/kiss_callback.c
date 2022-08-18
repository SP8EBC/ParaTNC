/*
 * kiss_callback.c
 *
 *  Created on: Aug 17, 2022
 *      Author: mateusz
 */

#include "kiss_communication.h"
#include "configuration_handler.h"

int32_t kiss_callback_get_running_config(uint8_t* input_frame_from_host, uint16_t input_len, uint8_t* response_buffer, uint16_t buffer_size) {

	uint32_t conf_size = 0;

	configuration_handler_region_t current_region;

	// set current message to start pooling
	kiss_current_async_message = KISS_RUNNING_CONFIG;

	// the rest of content of an input frame is irrevelant, but we need to send
	// a response telling how long configuration data is
	memset(response_buffer, 0x00, buffer_size);

	// get currently used configuration and its size in flash memory
	current_region = configuration_get_current(&conf_size);

	// construct a response
	response_buffer[0] = FEND;
	response_buffer[1] = KISS_RUNNING_CONFIG;
	response_buffer[2] = (uint8_t)(current_region & 0xFF);
	response_buffer[3] = (uint8_t)(conf_size & 0xFF);
	response_buffer[4] = (uint8_t)((conf_size & 0xFF00) >> 8);
	response_buffer[5] = (uint8_t)((conf_size & 0xFF0000) >> 16);
	response_buffer[6] = (uint8_t)((conf_size & 0xFF000000) >> 24);
	response_buffer[7] = FEND;

	return 8;

}

int16_t kiss_pool_callback_get_running_config(uint8_t * output_buffer, uint16_t buffer_size, uint8_t current_segment){

}
