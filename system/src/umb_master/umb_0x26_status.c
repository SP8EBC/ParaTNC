/*
 * umb_status_request.c
 *
 *  Created on: 23.02.2020
 *      Author: mateusz
 */

#include "../umb_master/umb_master.h"
#include "../umb_master/umb_0x26_status.h"
#include "station_config.h"

#include <string.h>

#ifdef _UMB_MASTER

umb_retval_t umb_0x26_status_request(umb_frame_t* frame, umb_context_t* ctx) {

	if (umb_context.state != UMB_STATUS_IDLE && umb_context.state != UMB_STATUS_ERROR) {
		return UMB_BUSY;
	}

	frame->command_id = 0x26;
	frame->slave_class = _UMB_SLAVE_CLASS;
	frame->slave_id = _UMB_SLAVE_ID;
	frame->lenght = 0;

	memset(frame->payload, 0x00, UMB_FRAME_MAX_PAYLOAD_LN);

	ctx->state = UMB_STATUS_SENDING_REQUEST_TO_SLAVE;

	return UMB_OK;

}

umb_retval_t umb_0x26_status_callback(umb_frame_t* frame, umb_context_t* ctx) {


	ctx->state = UMB_STATUS_IDLE;

	return UMB_OK;
}
#endif

