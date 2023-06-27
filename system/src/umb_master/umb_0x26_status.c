/*
 * umb_status_request.c
 *
 *  Created on: 23.02.2020
 *      Author: mateusz
 */

#include <configuration_nvm/config_data.h>
#include "../umb_master/umb_master.h"
#include "../umb_master/umb_0x26_status.h"
#include "main.h"

#include <string.h>

umb_retval_t umb_0x26_status_request(umb_frame_t* frame, umb_context_t* ctx, const config_data_umb_t * const config_umb) {

	if (ctx->state != UMB_STATUS_IDLE && ctx->state != UMB_STATUS_ERROR) {
		return UMB_BUSY;
	}

	frame->command_id = 0x26;
	frame->slave_class = (uint8_t)(config_umb->slave_class & 0xFF);
	frame->slave_id = (uint8_t)(config_umb->slave_id & 0xFF);
	frame->lenght = 0;

	memset(frame->payload, 0x00, UMB_FRAME_MAX_PAYLOAD_LN);

	ctx->state = UMB_STATUS_READY_TO_SEND;
	ctx->current_routine = 0x26;

	return UMB_OK;

}

umb_retval_t umb_0x26_status_callback(umb_frame_t* frame, umb_context_t* ctx) {

	// return value
	umb_retval_t output = UMB_OK;

	// first byte of the response is always 0x00 then one or more error code is transmitted
	uint8_t error_codes_amount = frame->lenght - 1;

	uint8_t first_errc = frame->payload[1];

	// error code set to 0x00 means just that device operates normally and there are no errors
	if (first_errc == 0x00) {
		// so just do nothing and then return from the function
		;
	}
	else {
		// copy error codes to context
		for (int i = 0; i < error_codes_amount && i < UMB_CONTEXT_ERR_HISTORY_LN; i++) {

			// copy current error code to the buffer
			ctx->nok_error_codes[ctx->nok_error_it] = frame->payload[1 + i];

			// move the iterator through the buffer
			ctx->nok_error_it++;

			// check if and end of the buffer is reach here
			ctx->nok_error_it %= UMB_CONTEXT_ERR_HISTORY_LN;
		}

		// storing the time when last error code will be stored
		ctx->time_of_last_nok = main_get_master_time();

		// trigger the status message with a content of fault store
		ctx->trigger_status_msg = 1;
	}

	ctx->state = UMB_STATUS_IDLE;

	return output;
}

