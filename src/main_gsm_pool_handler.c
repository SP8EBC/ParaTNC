/*
 * main_gsm_pool_handler.c
 *
 *  Created on: Jul 17, 2025
 *      Author: mateusz
 */

#if defined(STM32L471xx)

#include "main_gsm_pool_handler.h"

#include "main.h"
#include "rte_main.h"
#include "kiss_communication/kiss_communication.h"
#include "kiss_communication/kiss_communication_defs.h"
#include "kiss_communication/kiss_nrc_response.h"
#include "kiss_communication/types/kiss_communication_transport_t.h"
#include "kiss_communication/kiss_communication_aprsmsg.h"

#include "gsm/sim800c.h"

#include "aprsis.h"
#include "event_log.h"

extern int system_is_rtc_ok(void);

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
				) {

	int32_t i = 0;

	// if data has been received
	if (gsm_srl_ctx_ptr->srl_rx_state == SRL_RX_DONE || gsm_srl_ctx_ptr->srl_rx_state == SRL_RX_ERROR) {

		// receive callback for communicatio with the modem
		gsm_sim800_rx_done_event_handler(gsm_srl_ctx_ptr, gsm_state);
	}

	if (gsm_srl_ctx_ptr->srl_tx_state == SRL_TX_IDLE) {
		gsm_sim800_tx_done_event_handler(gsm_srl_ctx_ptr, gsm_state);
	}

	// if message ACK has been scheduled
	if (*trigger_message_ack == 1) {
		// if TCP/IP connection is not busy and received message comes from APRS-IS
		if ((received_message->source == MESSAGE_SOURCE_APRSIS || received_message->source == MESSAGE_SOURCE_APRSIS_HEXCNTR)
				&& gsm_sim800_tcpip_tx_busy() == 0) {

			// clear ACK request
			*trigger_message_ack = 0;

			// create and send ACK for this message
			aprsis_send_ack_for_message(received_message);

			received_message->source = MESSAGE_SOURCE_UNINITIALIZED;

			// decode message, do it after ACK is scheduled to be sure about right sequence
			const kiss_communication_transport_t type = kiss_communication_aprsmsg_check_type(received_message->content, MESSAGE_MAX_LENGHT);

			// decode HEXSTRING
			if (type == KISS_TRANSPORT_HEXSTRING) {
				*from_message_ln = kiss_communication_aprsmsg_decode_hexstring(received_message->content, received_message->content_ln, from_message + 1, MAIN_KISS_FROM_MESSAGE_LEN - 1);
			}
			else {
				// zero message lenght if message cannot be decoded for some reason
				*from_message_ln = 0;
			}

			// if KISS request has been parsed from APRS message
			if (*from_message_ln != 0) {
				// put artificial FEND at the begining of a buffer to make it compatible with 'kiss_parse_received'
				*(from_message) = FEND;

#define KISS_RESPONSE_MESSAGE_LN i

				// parse KISS request
				KISS_RESPONSE_MESSAGE_LN = kiss_parse_received(from_message, *from_message_ln, NULL, NULL, response_message, MAIN_KISS_FROM_MESSAGE_LEN, type);

				// if a response was generated
				if (KISS_RESPONSE_MESSAGE_LN > 0) {
					// check if a beginning and an end of generated response contains FEND.
					if ((*(response_message) == FEND) && (*(response_message + 1) == NONSTANDARD) && (*(response_message + KISS_RESPONSE_MESSAGE_LN - 1) == FEND)) {
						// if yes encode the response w/o them
						message_for_transmitting->content_ln = kiss_communication_aprsmsg_encode_hexstring(response_message + 2, KISS_RESPONSE_MESSAGE_LN - 3, message_for_transmitting->content, MESSAGE_MAX_LENGHT);
					}
					else {
						// if response doesn't contain FEND at the begining and the end just
						message_for_transmitting->content_ln = kiss_communication_aprsmsg_encode_hexstring(response_message, KISS_RESPONSE_MESSAGE_LN, message_for_transmitting->content, MESSAGE_MAX_LENGHT);
					}

					// put source and destination callsign
					main_set_ax25_my_callsign(&message_for_transmitting->from);
					main_copy_ax25_call(&message_for_transmitting->to, &received_message->from);

					message_for_transmitting->source = MESSAGE_SOURCE_APRSIS;

					// response is done, trigger sending it
					*trigger_send_message = 1;
				}
				else if (KISS_RESPONSE_MESSAGE_LN == KISS_COMM_RESULT_UNKNOWN_DIAG_SERV) {
					// assemble a response with NRC 'unknown diagnostics service'
					KISS_RESPONSE_MESSAGE_LN = kiss_nrc_response_fill_unknown_service(response_message);

					message_for_transmitting->content_ln = kiss_communication_aprsmsg_encode_hexstring(response_message + 2, KISS_RESPONSE_MESSAGE_LN - 3, message_for_transmitting->content, MESSAGE_MAX_LENGHT);

					// put source and destination callsign
					main_set_ax25_my_callsign(&message_for_transmitting->from);
					main_copy_ax25_call(&message_for_transmitting->to, &received_message->from);

					message_for_transmitting->source = MESSAGE_SOURCE_APRSIS;

					// response is done, trigger sending it
					*trigger_send_message = 1;
				}

			}

		}
		else if (received_message->source == MESSAGE_SOURCE_RADIO) {

		}

	}

	if (gsm_sim800_tcpip_tx_busy() == 0) {
		if (*trigger_send_message == 1 && message_for_transmitting->source == MESSAGE_SOURCE_APRSIS) {
			*trigger_send_message = 0;

			KISS_RESPONSE_MESSAGE_LN = message_encode(message_for_transmitting, (uint8_t*)own_aprs_msg, OWN_APRS_MSG_LN, MESSAGE_SOURCE_APRSIS);

			aprsis_send_any_string_buffer(own_aprs_msg, KISS_RESPONSE_MESSAGE_LN);
		}
		else if (*trigger_gsm_status == 1) {
			*trigger_gsm_status = 0;

			aprsis_send_gsm_status((const char *)callsign_with_ssid);
		}
		// if GSM status message is triggered and GSM module is not busy transmitting something else
		else if (*trigger_gsm_aprsis_counters_packet == 1) {
			*trigger_gsm_aprsis_counters_packet = 0;

			aprsis_send_server_comm_counters((const char *)callsign_with_ssid);
		}
		// if loginstring packet (APRS status packet with loginstring received from a server)
		// is triggered and GSM module is not busy
		else if (*trigger_gsm_loginstring_packet == 1) {
			*trigger_gsm_loginstring_packet = 0;

			aprsis_send_loginstring((const char *)callsign_with_ssid, system_is_rtc_ok(), rte_main_battery_voltage);
		}
		// if telemetry packet is triggerend
		else if (*trigger_gsm_telemetry_values == 1) {
			*trigger_gsm_telemetry_values = 0;

			aprsis_send_telemetry(1u, (const char *)callsign_with_ssid);
		}
		else if (*trigger_gsm_telemetry_descriptions == 1) {

			// check if this ought to be first telemetry description in sequence
			if (*telemetry_description == TELEMETRY_NOTHING) {
				// if yes check if victron telemetry is enabled
				if (config_data_mode->victron != 0) {
					// set the first packet accordingly
					*telemetry_description = TELEMETRY_PV_PARM;
				}
				else if (config_data_mode->digi_viscous != 0) {
					*telemetry_description = TELEMETRY_VISCOUS_PARAM;
				}
				else {
					*telemetry_description = TELEMETRY_NORMAL_PARAM;
				}
			}

			// assemble and sent a telemetry description packet
			*telemetry_description = aprsis_send_description_telemetry(1u, *telemetry_description, config_data_basic, config_data_mode, (const char *)callsign_with_ssid);

			// if there is nothing to send
			if (*telemetry_description == TELEMETRY_NOTHING) {
				*trigger_gsm_telemetry_descriptions = 0;
			}
		}
		// trigger value contain how many entries shall be sent
		else if (*trigger_gsm_event_log > 0 && aprsis_get_aprsis_logged() == 1) {

			// set a pointer to even in exposed form which will be sent now
			const event_log_exposed_t * current_exposed_event = &exposed_events[(*trigger_gsm_event_log) - 1];

			// create APRS status content itself
			const uint16_t str_size = event_exposed_to_string(current_exposed_event, own_aprs_msg, OWN_APRS_MSG_LN);

			(*trigger_gsm_event_log)--;

			if (str_size > 0) {
				aprsis_send_any_status((const char *)callsign_with_ssid, own_aprs_msg ,str_size);
			}
		}

	}
}
#endif


