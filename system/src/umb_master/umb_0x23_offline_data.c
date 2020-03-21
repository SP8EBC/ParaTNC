/*
 * umb_0x23_offline_data.c
 *
 *  Created on: 20.03.2020
 *      Author: mateusz
 */

#include "../umb_master/umb_master.h"
#include "../umb_master/umb_0x23_offline_data.h"
#include "station_config.h"
#include "rte_wx.h"

#define UNSIGNED_CHAR 	0x10
#define SIGNED_CHAR		0x11
#define UNSIGNED_SHORT	0x12
#define SIGNED_SHORT	0x13
#define UNSIGNED_LONG	0x14
#define SIGNED_LONG		0x15
#define FLOAT			0x16

#define UNSIGNED_CHAR_LN	1
#define SIGNED_CHAR_LN		1
#define UNSIGNED_SHORT_LN	2
#define SIGNED_SHORT_LN		2
#define	UNSIGNED_LONG_LN	4
#define SIGNED_LONG_LN		4
#define FLOAT_LN			4


umb_retval_t umb_0x23_offline_data_request(umb_frame_t* frame, umb_context_t* ctx, uint16_t channel_number) {

	if (ctx->state != UMB_STATUS_IDLE && ctx->state != UMB_STATUS_ERROR) {
		return UMB_BUSY;
	}

	frame->command_id = 0x23;
	frame->slave_class = _UMB_SLAVE_CLASS;
	frame->slave_id = _UMB_SLAVE_ID;
	frame->lenght = 2;

	// channels are little endian 16 bit unsigned integer
	frame->payload[0] = channel_number & 0xFF;
	frame->payload[1] = (channel_number & 0xFF00) >> 8;

	return UMB_OK;
}

umb_retval_t umb_0x23_offline_data_callback(umb_frame_t* frame, umb_context_t* ctx) {

	// return value
	umb_retval_t output = UMB_OK;

	// status from sensor
	uint8_t status = frame->payload[0];

	// value type
	uint8_t type = 0;

	// channel from response
	uint16_t channel = 0;

	// raw value from sensor
	uint32_t raw_value = 0;

	// signed value to be written in RTE
	int32_t value_for_rte = 0;

	// temporary float point buffer to handle this type
	float temp = 0.0f;

	// check if status is OK
	if (status == 0x00) {
		// fetch the channel number
		channel = frame->payload[1] | (frame->payload[2] << 8);

		// fetch the value type
		type = frame->payload[3];

		// fetch the raw value into temporary storage
		switch (type) {
			case UNSIGNED_CHAR:
			case SIGNED_CHAR: {
				raw_value = frame->payload[4];
				break;
			}
			case UNSIGNED_SHORT:
			case SIGNED_SHORT: {
				raw_value = frame->payload[4] | (frame->payload[5] << 8);
				break;
			}
			case UNSIGNED_LONG:
			case SIGNED_LONG: {
				raw_value = frame->payload[4] | (frame->payload[5] << 8) | (frame->payload[6] << 16) | (frame->payload[7] << 24);
				break;
			}
			case FLOAT: {
				raw_value = frame->payload[4] | (frame->payload[5] << 8) | (frame->payload[6] << 16) | (frame->payload[7] << 24);
				break;
			}
			default: {
				ctx->state = UMB_STATUS_ERROR;

				output = UMB_GENERAL_ERROR;

				return output;
			}
		}

		// convert a raw value to a value to be stored in the rte as an integer multiplicity of .1
		switch (type) {
			case UNSIGNED_CHAR:
			case UNSIGNED_SHORT:
			case UNSIGNED_LONG:
			case SIGNED_LONG: {
				// multiply this by the factor of ten
				value_for_rte = raw_value * 10;
				break;
			}
			// for signed types do a conversion to intermediate signed variable to take care about
			// negative numbers
			case SIGNED_CHAR: {
				value_for_rte = (int32_t)(((int8_t)(raw_value & 0xFF)) * 10);
				break;
			}
			case SIGNED_SHORT: {
				value_for_rte = (int32_t)(((int16_t)(raw_value & 0xFFFF)) * 10);
				break;
			}
			case FLOAT: {
				temp = *(float*)&raw_value;
				value_for_rte = (int32_t)(temp * 10.0f);
				break;
			}
			// there is no default case here as if a type was any different from supported the function would
			// quit before, in previous switch
		}

		// limit the value as rte stores only 16 bit signed integers
		if (value_for_rte > 32767) {
			ctx->state = UMB_STATUS_ERROR;

			return UMB_VALUE_OUT_OF_RANGE;
		}
		else if (value_for_rte < (int32_t)-32767) {
			ctx->state = UMB_STATUS_ERROR;

			return UMB_VALUE_OUT_OF_RANGE;
		}

		for (int i = 0; i < UMB_CHANNELS_STORAGE_CAPAC; i++) {
			// look for free slot in channel values storage
			if(	rte_wx_umb_channel_values[i][0] == (int16_t)0xFFFF ||
				rte_wx_umb_channel_values[i][0] == (int16_t)channel	)
			{
				// if found store the value and then break the loop execution
				rte_wx_umb_channel_values[i][0] = channel;
				rte_wx_umb_channel_values[i][1] = (int16_t)value_for_rte;
				break;
			}
		}

	}
	else {
		// if not stop further processing
		ctx->state = UMB_STATUS_ERROR;

		output = UMB_NOK_STATUS_GOT_WITH_RESP;
	}

	return output;

}
