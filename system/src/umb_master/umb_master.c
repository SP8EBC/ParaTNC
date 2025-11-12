/*
 * umb_client.c
 *
 *  Created on: 22.02.2020
 *      Author: mateusz
 */

#include <string.h>
#include <stdio.h>

#include <umb_master/umb_0x26_status.h>
#include <umb_master/umb_0x23_offline_data.h>
#include <umb_master/umb_master.h>
#include <umb_master/umb_channel_pool.h>
#include <rte_wx.h>

#include "event_log.h"
#include "events_definitions/events_umb.h"

#include "integers.h"

/// ==================================================================================================
///	LOCAL DEFINITIONS
/// ==================================================================================================

#define SOH 0x01
#define STX 0x02
#define ETX 0x03
#define EOT 0x04

#define V10 0x10

#define MASTER_ID 0x01
#define MASTER_CLASS 0xF0

#define TEN_MINUTES (1000 * 600)

/// ==================================================================================================
///	LOCAL DATA TYPES
/// ==================================================================================================

/// ==================================================================================================
///	LOCAL VARIABLES
/// ==================================================================================================

/**
 * 0 -> Watchdog hasn't fired yet
 * 1 -> UMB master got stuck once, so the serial port and master itself was reinitialized
 * 2 -> UMB master still has some problems, even after reinitialization. It's time to reset
 */
static uint8_t umb_master_watchdog_state = 0;

static uint32_t umb_master_watchdog_last_fired = 0;

static uint8_t umb_master_qf_error_log_debouncer = 0;

/// ==================================================================================================
///	GLOBAL VARIABLES
/// ==================================================================================================

/// ==================================================================================================
///	LOCAL FUNCTIONS
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

/**
 *
 * @param ctx
 * @param serial_ctx
 * @param config_umb
 */
void umb_master_init(umb_context_t* ctx, srl_context_t* serial_ctx, const config_data_umb_t * const config_umb) {
	ctx->current_routine = UMB_CONTEXT_IDLE_ROUTINE;
	ctx->state = UMB_STATUS_IDLE;
	ctx->nok_error_it = 0;
	ctx->time_of_last_nok = 0xFFFFFFFFu;
	ctx->time_of_last_comms_timeout = 0xFFFFFFFFu;

	ctx->time_of_last_successful_comms = 0;

	ctx->last_fault_channel = 0;

	ctx->serial_context = serial_ctx;

	for (int i = 0; i < UMB_CONTEXT_ERR_HISTORY_LN; i++) {
		ctx->nok_error_codes[i] = 0;
	}

	for (int i = 0; i < UMB_CHANNELS_STORAGE_CAPAC; i++)
		rte_wx_umb_channel_values[i][0] = 0xFFFF;

	ctx->channel_numbers[0] = config_umb->channel_windspeed;

	ctx->channel_numbers[1] = config_umb->channel_wingsusts;

	ctx->channel_numbers[2] = config_umb->channel_winddirection;

	ctx->channel_numbers[3] = config_umb->channel_temperature;

	ctx->channel_numbers[4] = config_umb->channel_qnh;

}

/**
 *
 * @return
 */
uint8_t umb_master_watchdog(umb_context_t* ctx, uint32_t current_master_time)
{
	uint8_t out = 0;

	const uint32_t last_successfull_ago = current_master_time - ctx->time_of_last_successful_comms;

	const uint32_t last_timeout_fail_ago = current_master_time - ctx->time_of_last_comms_timeout;

	const uint32_t last_watchdog_ago = current_master_time - umb_master_watchdog_last_fired;

	// inhibit any watchdog checks if library is initialized but no UMB sensor comms has been
	// done so far
	if (ctx->time_of_last_successful_comms != 0) {
		if (last_successfull_ago > (TEN_MINUTES)) {

			// if there is no correct response from the response for longer than 2 minutes
			umb_master_watchdog_state++;

			// store a moment when a watchdog has been fired
			umb_master_watchdog_last_fired = current_master_time;

			out = umb_master_watchdog_state;
		}
		else if (ctx->time_of_last_comms_timeout != 0xFFFFFFFFu) {
			// inhibit this check when no timeouts was detected so far
			// as default value of 0xFFFFFFFF will mess the value.
			// in ideal world this always will be 0xFFFFFFFF

			if (last_timeout_fail_ago > (TEN_MINUTES) && last_successfull_ago > (TEN_MINUTES)) {
				// if there is no correct response from the response for longer than 2 minutes
				umb_master_watchdog_state++;

				// store a moment when a watchdog has been fired
				umb_master_watchdog_last_fired = current_master_time;

				out = umb_master_watchdog_state;
			}
		}
		else {
			; // no explicit action
		}
	}
	else if (umb_master_watchdog_state > 0) {
		// if no UMB protocol communication has been done so far, but the watchdog fired
		// at least once

		if (last_watchdog_ago > (TEN_MINUTES)) {
			umb_master_watchdog_state++;

			out = umb_master_watchdog_state;

		}
	}

	return out;
}

/**
 *
 * @param serial_buffer
 * @param buffer_ln
 * @param frame
 * @return
 */
umb_retval_t umb_parse_serial_buffer_to_frame(uint8_t* serial_buffer, uint16_t buffer_ln, umb_frame_t* frame) {

	uint16_t crc = 0xFFFFu;

	umb_retval_t out = UMB_UNINITIALIZED;

	if (serial_buffer[0] != SOH && serial_buffer[1] != V10) {
		return UMB_NOT_VALID_FRAME;
	}

	if (serial_buffer[3] != (MASTER_CLASS >> 4) && serial_buffer[2] != MASTER_ID) {
		return UMB_TO_ANOTHER_MASTER;
	}

	frame->slave_class 		= serial_buffer[5] >> 4;
	frame->slave_id 		= serial_buffer[4];
	frame->lenght 			= serial_buffer[6] - 2;

	if (serial_buffer[7] != STX) {
		const uint32_t lparam1 = integers_dword_from_arr(serial_buffer, 0);
		const uint32_t lparam2 = integers_dword_from_arr(serial_buffer, 4);
		const uint16_t wparam1 = integers_word_from_arr(serial_buffer, 8);
		const uint16_t wparam2 = integers_word_from_arr(serial_buffer, 10);

		out = UMB_NOT_VALID_FRAME;

		event_log_sync(
				  EVENT_WARNING,
				  EVENT_SRC_UMB,
				  EVENTS_UMB_WARN_RECEIVED_FRAME_MALFORMED,
				  0, serial_buffer[7],
				  wparam1, wparam2,
				  lparam1, lparam2);
	}
	else {

		frame->command_id		= serial_buffer[8];
		frame->protocol_version = serial_buffer[9];

		// checking if payload isn't too big to fit into structure
		if (frame->lenght >= UMB_FRAME_MAX_PAYLOAD_LN) {
			const uint32_t lparam1 = integers_dword_from_arr(serial_buffer, 0);
			const uint32_t lparam2 = integers_dword_from_arr(serial_buffer, 4);
			const uint16_t wparam1 = integers_word_from_arr(serial_buffer, 8);
			const uint16_t wparam2 = integers_word_from_arr(serial_buffer, 10);

			out = UMB_RECV_FRAME_TOO_LONG;

			event_log_sync(
					  EVENT_WARNING,
					  EVENT_SRC_UMB,
					  EVENTS_UMB_WARN_RECEIVED_FRAME_TOO_LONG,
					  frame->lenght, 0,
					  lparam1, lparam2,
					  wparam1, wparam2);
		}
		else {
			// Copying payload of the frame from a serial buffer
			for (int i = 0; (i < frame->lenght && i < buffer_ln); i++) {
				frame->payload[i] = serial_buffer[10 + i];
			}

			// recalculating crc from frame content
			for (int j = 0; j <= frame->lenght + 8 + 2; j++) {
				crc = umb_calc_crc(crc, serial_buffer[j]);
			}

			frame->calculated_checksum_lsb = crc & 0xFF;
			frame->calculated_checksum_msb = (crc & 0xFF00) >> 8;

			if (	serial_buffer[frame->lenght + 9 + 2] != frame->calculated_checksum_lsb ||
					serial_buffer[frame->lenght + 10 + 2] != frame->calculated_checksum_msb) {
				out = UMB_WRONG_CRC;

				const uint16_t crc_from_frame = (serial_buffer[frame->lenght + 10 + 2] << 8) |
												(serial_buffer[frame->lenght + 9 + 2]);

				event_log_sync(
						  EVENT_WARNING,
						  EVENT_SRC_UMB,
						  EVENTS_UMB_WARN_CRC_FAILED_IN_RECEIVED_FRAME,
						  0, 0,
						  crc, crc_from_frame,
						  0, 0);

			}
			else {
				out = UMB_OK;
			}
		}
	}

	return out;
}

/**
 *
 * @param serial_buffer
 * @param buffer_ln
 * @param frame
 * @param target_ln
 * @param config_umb
 * @return
 */
umb_retval_t umb_parse_frame_to_serial_buffer(uint8_t* serial_buffer, uint16_t buffer_ln, umb_frame_t* frame, uint16_t* target_ln, const config_data_umb_t * const config_umb) {

	int i = 0;
	uint16_t crc = 0xFFFFu;

	if (frame->lenght - 2 >= buffer_ln)
		return UMB_FRAME_TOO_LONG_FOR_TX;

	memset(serial_buffer, 0x00, buffer_ln);

	serial_buffer[i++] = SOH;
	serial_buffer[i++] = V10;
	serial_buffer[i++] = (uint8_t)(config_umb->slave_id & 0xFF); //_UMB_SLAVE_ID;
	serial_buffer[i++] = (uint8_t)(config_umb->slave_class & 0xFF) << 4;//_UMB_SLAVE_CLASS << 4;
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

/**
 *
 * @param crc_buff
 * @param input
 * @return
 */
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
umb_retval_t umb_pooling_handler(umb_context_t* ctx, umb_call_reason_t r, uint32_t master_time, const config_data_umb_t * const config_umb) {

	uint16_t temp = 0;
	umb_retval_t main_umb_retval = UMB_UNINITIALIZED;

	switch(ctx->state) {
		case UMB_STATUS_IDLE:
			break;
		case UMB_STATUS_READY_TO_SEND: {
			// Check if serial port is idle and can be used in this moment to transmit something
			if (r == REASON_TRANSMIT_IDLE) {
				// parsing UMB frame into serial buffer
				umb_parse_frame_to_serial_buffer(ctx->serial_context->srl_tx_buf_pointer, ctx->serial_context->srl_tx_buf_ln, &rte_wx_umb, &temp, config_umb);

				// starting data transfer
				srl_start_tx(ctx->serial_context, temp);

				// switching state
				ctx->state = UMB_STATUS_SENDING_REQUEST_TO_SLAVE;
			}
			break;
		}
		// when routine request generator created the frame which now waits to be send
		case UMB_STATUS_SENDING_REQUEST_TO_SLAVE: {

			if (r == REASON_TRANSMIT_IDLE) {
				// transmission is done and now receive must be triggered
				srl_receive_data(ctx->serial_context, 8, SOH, 0x00, 0, 6, 12);

				// enable timeout in case that sensor won't send any reponse
				srl_switch_timeout(ctx->serial_context, 1, 2345);
				srl_switch_timeout_for_waiting(ctx->serial_context, 1);

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
				main_umb_retval = umb_parse_serial_buffer_to_frame(
																	srl_get_rx_buffer(ctx->serial_context),
																	srl_get_num_bytes_rxed(ctx->serial_context),
																	&rte_wx_umb);

				// if data was decoded successfully
				if (main_umb_retval == UMB_OK) {
					// call a master callback to look what was received
					main_umb_retval = umb_master_callback(&rte_wx_umb, ctx);

					if (main_umb_retval == UMB_OK) {
						ctx->time_of_last_successful_comms = master_time;

						ctx->state = UMB_STATUS_RESPONSE_AVALIABLE;
					}
					else
						ctx->state = UMB_STATUS_ERROR;
				}


			}
			else if (r == REASON_RECEIVE_ERROR) {
				ctx->state = UMB_STATUS_ERROR;

				ctx->time_of_last_comms_timeout = master_time;

				uint8_t * serial_buffer = srl_get_rx_buffer(ctx->serial_context);

				const uint32_t lparam1 = integers_dword_from_arr(serial_buffer, 0);
				const uint32_t lparam2 = integers_dword_from_arr(serial_buffer, 4);
				const uint16_t wparam1 = integers_word_from_arr(serial_buffer, 8);
				const uint16_t wparam2 = integers_word_from_arr(serial_buffer, 10);

				event_log_sync(
						  EVENT_ERROR,
						  EVENT_SRC_UMB,
						  EVENTS_UMB_ERROR_RECEIVING,
						  ctx->serial_context->srl_rx_error_reason, (uint8_t)(ctx->serial_context->srl_rx_bytes_req & 0xFFu),
						  lparam1, lparam2,
						  wparam1, wparam2);

			}
			break;
		}
		case UMB_STATUS_RESPONSE_AVALIABLE: {
			if (r == REASON_RECEIVE_IDLE) {
				ctx->state = UMB_STATUS_IDLE;

			}
			break;
		}
		case UMB_STATUS_ERROR: {
			ctx->state = UMB_STATUS_IDLE;
			ctx->current_routine = UMB_CONTEXT_IDLE_ROUTINE;

			break;
		}
		case UMB_STATUS_ERROR_WRONG_RID_IN_RESPONSE: {
			ctx->state = UMB_STATUS_IDLE;
			ctx->current_routine = UMB_CONTEXT_IDLE_ROUTINE;

			break;
		}
	}

	return UMB_OK;
}

/**
 * This function is called globally after receiving any UMB data
 */
umb_retval_t umb_master_callback(umb_frame_t* frame, umb_context_t* ctx) {

	umb_retval_t ret_fron_callback = UMB_UNINITIALIZED;

	// check if this is a response to routine which was queried recently
	if (frame->command_id != ctx->current_routine) {
		ctx->state = UMB_STATUS_ERROR_WRONG_RID_IN_RESPONSE;

		event_log_sync(
				  EVENT_ERROR,
				  EVENT_SRC_UMB,
				  EVENTS_UMB_ERROR_UNEXPECTED_ROUTINE_ID,
				  frame->command_id, ctx->current_routine,
				  0, 0,
				  0, 0);

		return UMB_GENERAL_ERROR;
	}
	else {
		ctx->current_routine = UMB_CONTEXT_IDLE_ROUTINE;
	}

	// looking for a callback to this response
	switch (frame->command_id) {
		case 0x23: {
			ret_fron_callback = umb_0x23_offline_data_callback(frame, ctx);

			if (ret_fron_callback == UMB_OK) {
				rte_wx_update_last_measuremenet_timers(ctx->current_channel);
			}

			break;
		}
		case 0x26: {
			ret_fron_callback = umb_0x26_status_callback(frame, ctx);
			break;
		}
	}

	return UMB_OK;
}

/**
 *
 * @param ctx
 * @param master_time
 * @return
 */
umb_qf_t umb_get_current_qf(umb_context_t* ctx, uint32_t master_time) {

	umb_qf_t out = UMB_QF_UNKNOWN;

	// initialization value - no error has been received from power up
	if (ctx->time_of_last_nok == 0xFFFFFFFFu) {
		out = UMB_QF_FULL;
	}
	// if the last error status was received more (sooner) than 10 minutes ago
	else if (master_time - ctx->time_of_last_nok >= TEN_MINUTES) {
		out = UMB_QF_FULL;
	}
	// if the last error has been received later than 10 minutes ago
	else {
		out =  UMB_QF_DEGRADED;

	}

	// if there were no timeouts so far
	if (ctx->time_of_last_comms_timeout == 0xFFFFFFFFu) {
		;
	}
	// if the time of last timeout during communication was later than 10 minutes ago
	else if (master_time - ctx->time_of_last_comms_timeout < TEN_MINUTES) {
//		if (ctx->time_of_last_successful_comms > ctx->time_of_last_comms_timeout) {
//			;
//		}
//		else {
			out =  UMB_QF_DEGRADED;
//		}
	}

	// if the last successfull communication with the sensor was 10 minutes ago or sooner
	if (master_time - ctx->time_of_last_successful_comms > TEN_MINUTES) {
		out = UMB_QF_NOT_AVALIABLE;

		umb_master_qf_error_log_debouncer++;

		if (	(umb_master_qf_error_log_debouncer < 0xF) ||
				(umb_master_qf_error_log_debouncer % 0xF))
		{
			event_log_sync(
					  EVENT_ERROR,
					  EVENT_SRC_UMB,
					  EVENTS_UMB_ERROR_QF_NOT_AVAILABLE,
					  umb_master_qf_error_log_debouncer, 0,
					  0, 0,
					  ctx->time_of_last_comms_timeout, 0);
		}
	}
	else {
		umb_master_qf_error_log_debouncer = 0;
	}

	return out;
}

/**
 *
 * @param ctx
 * @param out_buffer
 * @param buffer_size
 * @param status_string_ln
 * @param master_time
 */
void umb_construct_status_str(umb_context_t* ctx, char* out_buffer, uint16_t buffer_size, uint16_t* status_string_ln, uint32_t master_time) {

	uint32_t string_ln;
	uint32_t sprintf_out = 0;
	char local[11];

	// local copy of time_of_last_nok
	uint32_t local_tln = ctx->time_of_last_nok;

	// local copy of time_of_last_comms_timeput
	uint32_t local_tlct = ctx->time_of_last_comms_timeout;

	// check if there is a reason to print the status
	if (ctx->trigger_status_msg == 0) {
		*status_string_ln = 0;

		return;
	}

	// clear the flag to omit looping
	ctx->trigger_status_msg = 0;

	// check if there is enought size in buffer
	if (buffer_size < 64) {
		*status_string_ln = 0;

		return;
	}

	memset(local, 0x00, 11);

	// swap initialization values to zero
	if (local_tln == 0xFFFFFFFFu)
		local_tln = 0;

	if (local_tlct == 0xFFFFFFFFu)
		local_tlct = 0;

	// clearing target buffer
	for (int i = 0; i < buffer_size; i++)
		out_buffer[i] = '\0';

	string_ln = snprintf(out_buffer, buffer_size, ">UMB Status: [TIME= 0x%x, TLN= 0x%x, TLCT= 0x%x, LFC= %d ERRS= [",
																			(int)master_time,
																			(int)local_tln,
																			(int)local_tlct,
																			(int)ctx->last_fault_channel);
	for (int i = 0; i < UMB_CONTEXT_ERR_HISTORY_LN; i++ ) {
		// print the string representation of the error code into the buffer
		sprintf_out = snprintf(local, 11, "0x%02x, ", ctx->nok_error_codes[i]);

		if (sprintf_out + string_ln < buffer_size) {
			// rewrite the string value into the main output
			strcat(out_buffer, local);

			string_ln += sprintf_out;
		}
	}

	out_buffer[string_ln - 2] = ']';
	out_buffer[string_ln - 1] = ']';

	// setting target string lenght
	*status_string_ln = string_ln;
}

/**
 *
 * @param ctx
 */
void umb_clear_error_history(umb_context_t* ctx) {
	ctx->last_fault_channel = 0;

	for (int i = 0; i < UMB_CONTEXT_ERR_HISTORY_LN; i++) {
		ctx->nok_error_codes[i] = 0;
	}

	ctx->trigger_status_msg = 0;
}

/**
 *
 * @param config_umb
 * @return
 */
uint16_t umb_get_windspeed(const config_data_umb_t * const config_umb) {

	uint16_t out = 0;

	for (int i = 0; i < UMB_CHANNELS_STORAGE_CAPAC; i++) {
		if (rte_wx_umb_channel_values[i][0] == (int16_t)config_umb->channel_windspeed) {
			out = (uint16_t)rte_wx_umb_channel_values[i][1];
			break;
		}
	}

	return out;
}

/**
 *
 * @param config_umb
 * @return
 */
uint16_t umb_get_windgusts(const config_data_umb_t * const config_umb) {
	uint16_t out = 0;

	for (int i = 0; i < UMB_CHANNELS_STORAGE_CAPAC; i++) {
		if (rte_wx_umb_channel_values[i][0] == (int16_t)config_umb->channel_wingsusts) {
			out = (uint16_t)rte_wx_umb_channel_values[i][1];
			break;
		}
	}

	return out;
}

/**
 *
 * @param config_umb
 * @return
 */
int16_t umb_get_winddirection(const config_data_umb_t * const config_umb) {
	int16_t out = 0;

	for (int i = 0; i < UMB_CHANNELS_STORAGE_CAPAC; i++) {
		if (rte_wx_umb_channel_values[i][0] == (int16_t)config_umb->channel_winddirection) {
			out = (int16_t)rte_wx_umb_channel_values[i][1];
			break;
		}
	}
	out /= 10;
	return out;
}

/**
 *
 * @param config_umb
 * @return
 */
float umb_get_temperature(const config_data_umb_t * const config_umb) {
	float out = 0.0f;

	for (int i = 0; i < UMB_CHANNELS_STORAGE_CAPAC; i++) {
		if (rte_wx_umb_channel_values[i][0] == (int16_t)config_umb->channel_temperature) {
			out = (float)rte_wx_umb_channel_values[i][1] * 0.1f;
			break;
		}
	}

	return out;
}

/**
 *
 * @param config_umb
 * @return
 */
float umb_get_qnh(const config_data_umb_t * const config_umb) {
	float out = 0;

	for (int i = 0; i < UMB_CHANNELS_STORAGE_CAPAC; i++) {
		if (rte_wx_umb_channel_values[i][0] == (int16_t)config_umb->channel_qnh) {
			out = (float)rte_wx_umb_channel_values[i][1] * 0.1f;
			break;
		}
	}

	return out;
}

