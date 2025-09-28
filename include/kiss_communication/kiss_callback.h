#ifndef KISSCALLBACK_H_
#define KISSCALLBACK_H_

#include "./kiss_communication/kiss_communication.h"


#include <stdint.h>

extern uint8_t kiss_current_async_message;

int32_t kiss_callback_get_running_config(uint8_t* input_frame_from_host, uint16_t input_len, uint8_t* response_buffer, uint16_t buffer_size, kiss_communication_transport_t transport_media);
int16_t kiss_pool_callback_get_running_config(uint8_t * output_buffer, uint16_t buffer_size );

int32_t kiss_callback_get_version_id(uint8_t* input_frame_from_host, uint16_t input_len, uint8_t* response_buffer, uint16_t buffer_size, kiss_communication_transport_t transport_media);

int32_t kiss_callback_erase_startup(uint8_t* input_frame_from_host, uint16_t input_len, uint8_t* response_buffer, uint16_t buffer_size, kiss_communication_transport_t transport_media);
int32_t kiss_callback_program_startup(uint8_t* input_frame_from_host, uint16_t input_len, uint8_t* response_buffer, uint16_t buffer_size, kiss_communication_transport_t transport_media);

int32_t kiss_callback_read_did(uint8_t* input_frame_from_host, uint16_t input_len, uint8_t* response_buffer, uint16_t buffer_size, kiss_communication_transport_t transport_media);

int32_t kiss_callback_read_memory_by_addr(uint8_t* input_frame_from_host, uint16_t input_len, uint8_t* response_buffer, uint16_t buffer_size, kiss_communication_transport_t transport_media);

int32_t kiss_callback_reset(uint8_t* input_frame_from_host, uint16_t input_len, uint8_t* response_buffer, uint16_t buffer_size, kiss_communication_transport_t transport_media);

int32_t kiss_callback_routine_control(uint8_t* input_frame_from_host, uint16_t input_len, uint8_t* response_buffer, uint16_t buffer_size, kiss_communication_transport_t transport_media);


#endif
