#ifndef KISSCALLBACK_H_
#define KISSCALLBACK_H_

#include "kiss_communication.h"

#include <stdint.h>

int32_t kiss_callback_get_running_config(uint8_t* input_frame_from_host, uint16_t input_len, uint8_t* response_buffer, uint16_t buffer_size);
int16_t kiss_pool_callback_get_running_config(uint8_t * output_buffer, uint16_t buffer_size );

int32_t kiss_callback_get_version_id(uint8_t* input_frame_from_host, uint16_t input_len, uint8_t* response_buffer, uint16_t buffer_size);

int32_t kiss_callback_erase_startup(uint8_t* input_frame_from_host, uint16_t input_len, uint8_t* response_buffer, uint16_t buffer_size);
int32_t kiss_callback_program_startup(uint8_t* input_frame_from_host, uint16_t input_len, uint8_t* response_buffer, uint16_t buffer_size);

int32_t kiss_callback_read_did(uint8_t* input_frame_from_host, uint16_t input_len, uint8_t* response_buffer, uint16_t buffer_size);

#endif
