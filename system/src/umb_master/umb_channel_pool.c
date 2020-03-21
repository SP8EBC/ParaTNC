/*
 * umb_pool.c
 *
 *  Created on: 21.03.2020
 *      Author: mateusz
 */

#include <umb_master/umb_channel_pool.h>
#include "station_config.h"

void umb_channel_pool(umb_frame_t *frame, umb_context_t *ctx) {
#if defined(_UMB_MASTER)
	if (ctx->current_channel >  UMB_CHANNELS_STORAGE_CAPAC)
		ctx->current_channel = 0;

	uint16_t curr_chn = ctx->channel_numbers[ctx->current_channel];

	if (curr_chn != 0xFFFFu) {
		umb_0x23_offline_data_request(frame, ctx, curr_chn);
	}

	ctx->current_channel++;
#endif
}

