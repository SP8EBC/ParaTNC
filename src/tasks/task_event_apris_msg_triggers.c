/*
 * task_event_apris_msg_triggers.c
 *
 *  Created on: Sep 17, 2025
 *      Author: mateusz
 */

#include <FreeRTOS.h>
#include <event_groups.h>
#include <stdbool.h>
#include <stdint.h>
#include <task.h>

#include "main.h"
#include "main_freertos_externs.h"

#include "rte_main.h"

#include "system_stm32l4xx.h"

#include "kiss_communication/kiss_communication.h"
#include "kiss_communication/kiss_communication_aprsmsg.h"
#include "kiss_communication/kiss_communication_defs.h"
#include "kiss_communication/kiss_nrc_response.h"

#include "aprsis.h"
#include "supervisor.h"

#include "event_log.h"

#define RECEIVED_MESSAGE		 (&rte_main_received_message)
#define FROM_MESSAGE			 rte_main_kiss_from_message
#define FROM_MESSAGE_LN			 rte_main_kiss_from_message_ln
#define RESPONSE_MESSAGE		 rte_main_kiss_response_message
#define MESSAGE_FOR_TRANSMITTING (&rte_main_message_for_transmitting)
#define OWN_APRS_MESSAGE		 main_own_aprs_msg
#define CALLSIGN_WITH_SSID		 main_callsign_with_ssid

void task_event_aprsis_msg_trigger (void *param)
{
	(void)param;

	int i = 0;

	while (1) {
		// wait infinite amount of time for event from a serial port indicating that
		const EventBits_t bits_on_event = xEventGroupWaitBits (main_eventgroup_handle_aprs_trigger,
															   MAIN_EVENTGROUP_APRSIS_TRIG,
															   pdFALSE,
															   pdFALSE,
															   0xFFFFFFFFu);

		xEventGroupClearBits (main_eventgroup_handle_powersave,
							  MAIN_EVENTGROUP_PWRSAVE_EV_APRS_TRIG);

		// check if the event was really generated
		if (bits_on_event == MAIN_EVENTGROUP_APRSIS_TRIG_MESSAGE_ACK) {
			xEventGroupClearBits (main_eventgroup_handle_aprs_trigger,
								  MAIN_EVENTGROUP_APRSIS_TRIG_MESSAGE_ACK);

			// if TCP/IP connection is not busy and received message comes from APRS-IS
			if ((RECEIVED_MESSAGE->source == MESSAGE_SOURCE_APRSIS ||
				 RECEIVED_MESSAGE->source == MESSAGE_SOURCE_APRSIS_HEXCNTR) &&
				gsm_sim800_tcpip_tx_busy () == 0) {

				// clear ACK request
				//*trigger_message_ack = 0;

				// create and send ACK for this message
				aprsis_send_ack_for_message (RECEIVED_MESSAGE);

				RECEIVED_MESSAGE->source = MESSAGE_SOURCE_UNINITIALIZED;

				// decode message, do it after ACK is scheduled to be sure about right sequence
				const kiss_communication_transport_t type =
					kiss_communication_aprsmsg_check_type (RECEIVED_MESSAGE->content,
														   MESSAGE_MAX_LENGHT);

				// decode HEXSTRING
				if (type == KISS_TRANSPORT_HEXSTRING) {
					FROM_MESSAGE_LN = kiss_communication_aprsmsg_decode_hexstring (
						RECEIVED_MESSAGE->content,
						RECEIVED_MESSAGE->content_ln,
						FROM_MESSAGE + 1,
						MAIN_KISS_FROM_MESSAGE_LEN - 1);
				}
				else {
					// zero message lenght if message cannot be decoded for some reason
					FROM_MESSAGE_LN = 0;
				}

				// if KISS request has been parsed from APRS message
				if (FROM_MESSAGE_LN != 0) {
					// put artificial FEND at the begining of a buffer to make it compatible with
					// 'kiss_parse_received'
					FROM_MESSAGE[0] = FEND;

#define KISS_RESPONSE_MESSAGE_LN i

					// parse KISS request
					KISS_RESPONSE_MESSAGE_LN = kiss_parse_received (FROM_MESSAGE,
																	FROM_MESSAGE_LN,
																	NULL,
																	NULL,
																	RESPONSE_MESSAGE,
																	MAIN_KISS_FROM_MESSAGE_LEN,
																	type);

					// if a response was generated
					if (KISS_RESPONSE_MESSAGE_LN > 0) {
						// check if a beginning and an end of generated response contains FEND.
						if ((RESPONSE_MESSAGE[0] == FEND) && (RESPONSE_MESSAGE[1] == NONSTANDARD) &&
							(RESPONSE_MESSAGE[KISS_RESPONSE_MESSAGE_LN - 1] == FEND)) {
							// if yes encode the response w/o them
							MESSAGE_FOR_TRANSMITTING->content_ln =
								kiss_communication_aprsmsg_encode_hexstring (
									RESPONSE_MESSAGE + 2,
									KISS_RESPONSE_MESSAGE_LN - 3,
									MESSAGE_FOR_TRANSMITTING->content,
									MESSAGE_MAX_LENGHT);
						}
						else {
							// if response doesn't contain FEND at the begining and the end just
							MESSAGE_FOR_TRANSMITTING->content_ln =
								kiss_communication_aprsmsg_encode_hexstring (
									RESPONSE_MESSAGE,
									KISS_RESPONSE_MESSAGE_LN,
									MESSAGE_FOR_TRANSMITTING->content,
									MESSAGE_MAX_LENGHT);
						}

						// put source and destination callsign
						main_set_ax25_my_callsign (&(MESSAGE_FOR_TRANSMITTING->from));
						main_copy_ax25_call (&(MESSAGE_FOR_TRANSMITTING->to),
											 &(RECEIVED_MESSAGE->from));

						MESSAGE_FOR_TRANSMITTING->source = MESSAGE_SOURCE_APRSIS;

						// response is done, trigger sending it
						xEventGroupSetBits (main_eventgroup_handle_aprs_trigger,
											MAIN_EVENTGROUP_APRSIS_TRIG_SEND_MESSAGE);
					}
					else if (KISS_RESPONSE_MESSAGE_LN == KISS_COMM_RESULT_UNKNOWN_DIAG_SERV) {
						// assemble a response with NRC 'unknown diagnostics service'
						KISS_RESPONSE_MESSAGE_LN =
							kiss_nrc_response_fill_unknown_service (RESPONSE_MESSAGE);

						MESSAGE_FOR_TRANSMITTING->content_ln =
							kiss_communication_aprsmsg_encode_hexstring (
								RESPONSE_MESSAGE + 2,
								KISS_RESPONSE_MESSAGE_LN - 3,
								MESSAGE_FOR_TRANSMITTING->content,
								MESSAGE_MAX_LENGHT);

						// put source and destination callsign
						main_set_ax25_my_callsign (&MESSAGE_FOR_TRANSMITTING->from);
						main_copy_ax25_call (&MESSAGE_FOR_TRANSMITTING->to,
											 &RECEIVED_MESSAGE->from);

						MESSAGE_FOR_TRANSMITTING->source = MESSAGE_SOURCE_APRSIS;

						// response is done, trigger sending it
						//*trigger_send_message = 1;
						xEventGroupSetBits (main_eventgroup_handle_aprs_trigger,
											MAIN_EVENTGROUP_APRSIS_TRIG_SEND_MESSAGE);
					}
				}
			}
			else if (RECEIVED_MESSAGE->source == MESSAGE_SOURCE_RADIO) {
			}
		}
		else if (bits_on_event == MAIN_EVENTGROUP_APRSIS_TRIG_SEND_MESSAGE) {
			KISS_RESPONSE_MESSAGE_LN = message_encode (MESSAGE_FOR_TRANSMITTING,
													   (uint8_t *)OWN_APRS_MESSAGE,
													   OWN_APRS_MSG_LN,
													   MESSAGE_SOURCE_APRSIS);

			aprsis_send_any_string_buffer (OWN_APRS_MESSAGE, KISS_RESPONSE_MESSAGE_LN);

			xEventGroupClearBits (main_eventgroup_handle_aprs_trigger,
								  MAIN_EVENTGROUP_APRSIS_TRIG_SEND_MESSAGE);
		}
		else if (bits_on_event == MAIN_EVENTGROUP_APRSIS_TRIG_GSM_STATUS) {
			aprsis_send_gsm_status ((const char *)CALLSIGN_WITH_SSID); // done

			xEventGroupClearBits (main_eventgroup_handle_aprs_trigger,
								  MAIN_EVENTGROUP_APRSIS_TRIG_GSM_STATUS);
		}
		else if (bits_on_event == MAIN_EVENTGROUP_APRSIS_TRIG_APRSIS_COUNTERS) {
			aprsis_send_server_comm_counters ((const char *)CALLSIGN_WITH_SSID);

			xEventGroupClearBits (main_eventgroup_handle_aprs_trigger,
								  MAIN_EVENTGROUP_APRSIS_TRIG_APRSIS_COUNTERS);
		}
		else if (bits_on_event == MAIN_EVENTGROUP_APRSIS_TRIG_APRSIS_LOGINSTRING) {
			aprsis_send_loginstring ((const char *)CALLSIGN_WITH_SSID,
									 system_is_rtc_ok (),
									 rte_main_battery_voltage);

			packet_tx_set_trigger_tcp_weather();

			xEventGroupClearBits (main_eventgroup_handle_aprs_trigger,
								  MAIN_EVENTGROUP_APRSIS_TRIG_APRSIS_LOGINSTRING);
		}
		else if (bits_on_event == MAIN_EVENTGROUP_APRSIS_TRIG_TELEMETRY_VALUES) {
			aprsis_send_telemetry (1u, (const char *)CALLSIGN_WITH_SSID);

			xEventGroupClearBits (main_eventgroup_handle_aprs_trigger,
								  MAIN_EVENTGROUP_APRSIS_TRIG_TELEMETRY_VALUES);
		}
		else if (bits_on_event == MAIN_EVENTGROUP_APRSIS_TRIG_EVENTS) {
			if (rte_main_trigger_gsm_event_log > 0 && aprsis_get_aprsis_logged () == 1) {

				// set a pointer to even in exposed form which will be sent now
				const event_log_exposed_t *current_exposed_event =
					&rte_main_exposed_events[(rte_main_trigger_gsm_event_log)-1];

				// create APRS status content itself
				const uint16_t str_size = event_exposed_to_string (current_exposed_event,
																   OWN_APRS_MESSAGE,
																   OWN_APRS_MSG_LN);

				rte_main_trigger_gsm_event_log--;

				if (str_size > 0) {
					aprsis_send_any_status ((const char *)CALLSIGN_WITH_SSID,
											OWN_APRS_MESSAGE,
											str_size);
				}

				if (rte_main_trigger_gsm_event_log > 0) {
					xEventGroupSetBits (main_eventgroup_handle_aprs_trigger,
										MAIN_EVENTGROUP_APRSIS_TRIG_EVENTS);
				}
				else {
					xEventGroupClearBits (main_eventgroup_handle_aprs_trigger,
										  MAIN_EVENTGROUP_APRSIS_TRIG_EVENTS);
				}
			}
		}
		else {
			xEventGroupClearBits (main_eventgroup_handle_aprs_trigger, bits_on_event);
		}
		// supervisor_iam_alive(SUPERVISOR_THREAD_EVENT_APRSIS_MSG_TRIG);

		xEventGroupSetBits (main_eventgroup_handle_powersave, MAIN_EVENTGROUP_PWRSAVE_EV_APRS_TRIG);

	} // while (1)
}
