/*
 * kiss_program_startup.h
 *
 *  Created on: Dec 10, 2025
 *      Author: mateusz
 */

#ifndef KISS_COMMUNICATION_DIAGNOSTICS_SERVICES_KISS_PROGRAM_STARTUP_H_
#define KISS_COMMUNICATION_DIAGNOSTICS_SERVICES_KISS_PROGRAM_STARTUP_H_

#include "./kiss_communication/kiss_communication.h"

#include <stdint.h>

int32_t kiss_callback_program_startup(uint8_t* input_frame_from_host, uint16_t input_len, uint8_t* response_buffer, uint16_t buffer_size, kiss_communication_transport_t transport_media);




#endif /* KISS_COMMUNICATION_DIAGNOSTICS_SERVICES_KISS_PROGRAM_STARTUP_H_ */
