/*
 * umb_client.c
 *
 *  Created on: 22.02.2020
 *      Author: mateusz
 */

#include <string.h>
#include <umb_master/umb_0x26_status.h>
#include <umb_master/umb_0x23_offline_data.h>
#include <umb_master/umb_master.h>
#include <drivers/serial.h>
#include <rte_wx.h>

#ifdef _UMB_MASTER

#define SOH 0x01
#define STX 0x02
#define ETX 0x03
#define EOT 0x04

#define V10 0x10

#define MASTER_ID 0x01
#define MASTER_CLASS 0xF0

void umb_master_init(umb_context_t* ctx) {
	ctx->current_routine = -1;
	ctx->state = UMB_STATUS_IDLE;
	ctx->nok_error_it = 0;
	ctx->time_of_last_nok = 0xFFFFFFFFu;

	for (int i = 0; i < UMB_CONTEXT_ERR_HISTORY_LN; i++) {
		ctx->nok_error_codes[i] = 0;
	}

}

umb_retval_t umb_parse_serial_buffer_to_frame(uint8_t* serial_buffer, uint16_t buffer_ln, umb_frame_t* frame) {

	uint16_t crc = 0xFFFFu;
	uint16_t crc_from_frame;

	if (serial_buffer[0] != SOH && serial_buffer[1] != V10)
		return UMB_NOT_VALID_FRAME;

	if (serial_buffer[3] != (MASTER_CLASS >> 4) && serial_buffer[2] != MASTER_ID)
		return UMB_TO_ANOTHER_MASTER;

	frame->slave_class 		= serial_buffer[5] >> 4;
	frame->slave_id 		= serial_buffer[4];
	frame->lenght 			= serial_buffer[6] - 2;

	if (serial_buffer[7] != STX)
		return UMB_NOT_VALID_FRAME;

	frame->command_id		= serial_buffer[8];
	frame->protocol_version = serial_buffer[9];

	// checking if payload isn't too big to fit into structure
	if (frame->lenght >= UMB_FRAME_MAX_PAYLOAD_LN)
		return UMB_RECV_FRAME_TOO_LONG;

	// Copying payload of the frame from a serial buffer
	for (int i = 0; (i < frame->lenght && i < buffer_ln); i++) {
		frame->payload[i] = serial_buffer[10 + i];
	}

	// Retrieving the crc value from frame
	crc_from_frame = serial_buffer[frame->lenght + 9] | (serial_buffer[frame->lenght + 10] << 8);

	// recalculating crc from frame content
	for (int j = 0; j <= frame->lenght + 8 + 2; j++) {
		crc = umb_calc_crc(crc, serial_buffer[j]);
	}

	frame->calculated_checksum_lsb = crc & 0xFF;
	frame->calculated_checksum_msb = (crc & 0xFF00) >> 8;

	if (	serial_buffer[frame->lenght + 9 + 2] != frame->calculated_checksum_lsb ||
			serial_buffer[frame->lenght + 10 + 2] != frame->calculated_checksum_msb)
		return UMB_WRONG_CRC;

	return UMB_OK;
}

umb_retval_t umb_parse_frame_to_serial_buffer(uint8_t* serial_buffer, uint16_t buffer_ln, umb_frame_t* frame, uint16_t* target_ln) {

	int i = 0;
	uint16_t crc = 0xFFFFu;

	if (frame->lenght - 2 >= buffer_ln)
		return UMB_FRAME_TOO_LONG_FOR_TX;

	memset(serial_buffer, 0x00, buffer_ln);

	serial_buffer[i++] = SOH;
	serial_buffer[i++] = V10;
	serial_buffer[i++] = _UMB_SLAVE_ID;
	serial_buffer[i++] = _UMB_SLAVE_CLASS << 4;
	serial_buffer[i++] = MASTER_ID;
	serial_buffer[i++] = MASTER_CLASS;
	serial_buffer[i++] = frame->lenght + 2;
	serial_buffer[i++] = STX;
	serial_buffer[i++] = frame->command_id;
	serial_buffer[i++] = V10;

	for (int j = 0; j < frame->lenght; j++) {
		serial_buffer[i++] = frame->payload[j];
	}

	serial_buffer[i++] = ETX;

	for (int j = 0; j < i; j++) {
		crc = umb_calc_crc(crc, serial_buffer[j]);
	}

	serial_buffer[i++] = (uint8_t) crc & 0xFF;
	serial_buffer[i++] = (uint8_t) ((crc & 0xFF00) >> 8);
	serial_buffer[i++] = EOT;

	*target_ln = (uint16_t)i;

	return UMB_OK;
}

uint16_t umb_calc_crc(uint16_t crc_buff, uint8_t input) {
	uint8_t i;
	uint16_t x16;
	for	(i=0; i<8; i++)
	{
		// XOR current D0 and next input bit to determine x16 value
		if		( (crc_buff & 0x0001) ^ (input & 0x01) )
			x16 = 0x8408;
		else
			x16 = 0x0000;
		// shift crc buffer
		crc_buff = crc_buff >> 1;
		// XOR in the x16 value
		crc_buff ^= x16;
		// shift input for next iteration
		input = input >> 1;
	}
	return (crc_buff);
}

/**
 * This function is called in main 'for' loop to check if there
 * is anything to do regarding UMB
 */
umb_retval_t umb_pooling_handler(umb_context_t* ctx, umb_call_reason_t r) {

	uint16_t temp = 0;
	umb_retval_t main_umb_retval = UMB_UNINITIALIZED;

	switch(ctx->state) {
		case UMB_STATUS_IDLE:
			break;
		case UMB_STATUS_READY_TO_SEND: {
			// Check if serial port is idle and can be used in this moment to transmit something
			if (r == REASON_TRANSMIT_IDLE) {
				// parsing UMB frame into serial buffer
				umb_parse_frame_to_serial_buffer(srl_tx_buffer, TX_BUFFER_LN, &rte_wx_umb, &temp);

				// starting data transfer
				srl_start_tx(temp);

				// switching state
				ctx->state = UMB_STATUS_SENDING_REQUEST_TO_SLAVE;
			}
			break;
		}
		// when routine request generator created the frame which now waits to be send
		case UMB_STATUS_SENDING_REQUEST_TO_SLAVE: {

			if (r == REASON_TRANSMIT_IDLE) {
				// transmission is done and now receive must be triggered
				srl_receive_data(8, SOH, 0x00, 0, 6, 12);

				ctx->state = UMB_STATUS_WAITING_FOR_RESPONSE;
			}
			//else if (srl_tx_state == SRL_TXING) {
			//	return UMB_BUSY;
			//}
			else {
				;
			}
			break;
		}
		case UMB_STATUS_WAITING_FOR_RESPONSE: {
			if (r == REASON_TRANSMIT_IDLE) {

			}
			else if (r == REASON_RECEIVE_IDLE) {
				// deprcode the UMB frame from a content of serial buffer
				main_umb_retval = umb_parse_serial_buffer_to_frame(srl_get_rx_buffer(), srl_get_num_bytes_rxed(), &rte_wx_umb);

				// if data was decoded successfully
				if (main_umb_retval == UMB_OK) {
					// call a master callback to look what was received
					main_umb_retval = umb_master_callback(&rte_wx_umb, ctx);

					if (main_umb_retval == UMB_OK)
						ctx->state = UMB_STATUS_RESPONSE_AVALIABLE;
					else
						ctx->state = UMB_STATUS_ERROR;
				}


			}
			break;
		}
		case UMB_STATUS_RESPONSE_AVALIABLE: {
			//umb_master_callback()
			break;
		}
		case UMB_STATUS_ERROR: {
			break;
		}
		case UMB_STATUS_ERROR_WRONG_RID_IN_RESPONSE: {
			break;
		}
	}

	return UMB_OK;
}

/**
 * This function is called globally after receiving any UMB data
 */
umb_retval_t umb_master_callback(umb_frame_t* frame, umb_context_t* ctx) {

	// check if this is a response to routine which was queried recently
	if (frame->command_id != ctx->current_routine) {
		ctx->state = UMB_STATUS_ERROR_WRONG_RID_IN_RESPONSE;

		return UMB_GENERAL_ERROR;
	}

	// looking for a callback to this response
	switch (frame->command_id) {
		case 0x23: {
			umb_0x23_offline_data_callback(frame, ctx);
			break;
		}
		case 0x26: {
			umb_0x26_status_callback(frame, ctx);
			break;
		}
	}

	return UMB_OK;
}

#endif
