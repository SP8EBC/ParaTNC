/*
 * main_gsm_pool_handler.h
 *
 *  Created on: Jul 17, 2025
 *      Author: mateusz
 */

#ifndef MAIN_GSM_POOL_HANDLER_H_
#define MAIN_GSM_POOL_HANDLER_H_

#include <stdint.h>

#include "stored_configuration_nvm/config_data.h"
#include "drivers/serial.h"

#include "gsm/sim800_state_t.h"
#include "event_log_t.h"

#include "telemetry.h"
#include "message.h"

/**
 * Piece of code refactored from 'main' function responsible for pooling
 * @param gsm_srl_ctx_ptr
 * @param gsm_state
 * @param trigger_message_ack
 * @param received_message
 * @param from_message
 * @param from_message_ln
 * @param response_message
 * @param message_for_transmitting
 * @param trigger_send_message
 * @param trigger_gsm_status
 * @param trigger_gsm_aprsis_counters_packet
 * @param trigger_gsm_loginstring_packet
 * @param trigger_gsm_telemetry_values
 * @param trigger_gsm_telemetry_descriptions
 * @param telemetry_description
 * @param config_data_mode
 * @param config_data_basic
 * @param callsign_with_ssid
 * @param own_aprs_msg
 */
void main_gsm_pool_handler(
				srl_context_t				*const gsm_srl_ctx_ptr,				// 1
				gsm_sim800_state_t			*const gsm_state,					// 2
				uint8_t						*const trigger_message_ack,			// 3
				message_t					*const received_message,			// 4
				uint8_t						*const from_message,				// 5
				uint8_t						*const from_message_ln,				// 6
				uint8_t						*const response_message,			// 7
				message_t					*const message_for_transmitting,	// 8
				uint8_t						*const trigger_send_message,		// 9
				uint8_t						*const trigger_gsm_status,			// 10
				uint8_t						*const trigger_gsm_aprsis_counters_packet,		// 11
				uint8_t						*const trigger_gsm_loginstring_packet,			// 12
				uint8_t						*const trigger_gsm_telemetry_values,			// 13
				uint8_t						*const trigger_gsm_telemetry_descriptions,		// 14
				telemetry_description_t		*const telemetry_description,		// 15
				const config_data_mode_t	*const config_data_mode,			// 16
				const config_data_basic_t 	*const config_data_basic,			// 17
				const char					*const callsign_with_ssid,			// 18
				char						*const own_aprs_msg,				// 19
				uint8_t						*trigger_gsm_event_log,		// 20
				const event_log_exposed_t   *const exposed_events		// 21
				);


#endif /* MAIN_GSM_POOL_HANDLER_H_ */
