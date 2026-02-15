/*
 * kiss_program_startup.c
 *
 *  Created on: Dec 10, 2025
 *      Author: mateusz
 */

#include <FreeRTOS.h>
#include <task.h>

#include <kiss_communication/kiss_nrc_response.h>

#include "kiss_communication/types/kiss_communication_nrc_t.h"
#include "kiss_communication/types/kiss_communication_service_ids.h"
#include "kiss_communication/types/kiss_communication_transport_t.h"
#include <kiss_communication/diagnostics_services/kiss_program_startup.h>

#include <stored_configuration_nvm/configuration_handler.h>

#include "etc/kiss_configuation.h"

#include "event_log.h"
#include "supervisor.h"

#include "./events_definitions/events_kiss.h"

/**
 * Callback which program configuration data block received from the Host PC. Please bear in mind
 * that the TNC doesn't really take care what it receives and program. It is up to host PC to
 * provide senseful configuration with properly calculated checksum as this isn't recalculated
 * during programming.
 */
int32_t kiss_callback_program_startup (uint8_t *input_frame_from_host, uint16_t input_len,
									   uint8_t *response_buffer, uint16_t buffer_size,
									   kiss_communication_transport_t transport_media)
{

#define PROGRAM_STARTUP_LN 6

	/**
	 * The structure of an input frame goes like that:
	 * FEND, KISS_PROGRAM_STARTUP_CFG, data_PAYLOAD_LN, OFFSET_LSB, OFFSET_MSB, data, data, (...),
	 * FEND
	 *
	 * KISS_PROGRAM_STARTUP_CFG is a frame type, in this case 0x34, but might be also 0x00 for
	 * regular frame data to be sent over the air. data_PAYLOAD_LN is a lenght of data in this
	 * frame, the assumption is  that whole config data segment size will be an even multiply of a
	 * size of single frame.
	 */
	(void)buffer_size;

	// result to be returned to the host PC
	kiss_communication_nrc_t result;

	// offset within input frame where config start begining
	uint8_t *data_ptr = input_frame_from_host + 5;

	// size of data to be programmed into flash memory
	uint8_t data_size = *(input_frame_from_host + 2);

	uint16_t config_block_offset =
		*(input_frame_from_host + 3) | (*(input_frame_from_host + 4) << 8);

	if (transport_media == KISS_TRANSPORT_SERIAL_PORT) {
		taskENTER_CRITICAL ();

		result = configuration_handler_program_startup (data_ptr, data_size, config_block_offset);

		(void)event_log_sync (EVENT_WARNING,
							  EVENT_SRC_KISS,
							  EVENTS_DEFINITIONS_KISS_WARN_FLASHING_STARTUP,
							  result,
							  data_size,
							  config_block_offset,
							  0u,
							  0u,
							  0u);

		// construct a response
		response_buffer[0] = FEND;
		response_buffer[1] = NONSTANDARD;
		response_buffer[2] = PROGRAM_STARTUP_LN; // message lenght

		if (result == NRC_POSITIVE) {
			// construct a response
			response_buffer[3] = KISS_PROGRAM_STARTUP_CFG_RESP;
		}
		else {
			response_buffer[3] = KISS_NEGATIVE_RESPONSE_SERVICE;
		}

		response_buffer[4] = (uint8_t)result;
		response_buffer[5] = FEND;

		taskEXIT_CRITICAL ();

		return PROGRAM_STARTUP_LN;
	}
	else {
		return kiss_nrc_response_fill_security_access_denied (response_buffer);
	}
}
