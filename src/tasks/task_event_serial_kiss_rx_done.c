/*
 * task_event_serial_kiss_rx_done.c
 *
 *  Created on: Aug 18, 2025
 *      Author: mateusz
 */

#include <FreeRTOS.h>
#include <event_groups.h>
#include <stdbool.h>
#include <stdint.h>
#include <task.h>

#include "main.h"
#include "main_freertos_externs.h"

#include "drivers/serial.h"
#include "etc/kiss_configuation.h"

#include "kiss_communication/kiss_communication.h"
#include "kiss_communication/types/kiss_communication_transport_t.h"

#include "supervisor.h"

static uint8_t task_event_kiss_buffer[KISS_CONFIG_DIAGNOSTIC_BUFFER_LN];

void task_event_kiss_rx_done (void *param)
{

	(void)param;

	// another pointer to serial kiss context. with shorter name for convenience ;)
	srl_context_t *ctx = main_kiss_srl_ctx_ptr;

	while (1) {
		SUPERVISOR_MONITOR_CLEAR(EVENT_SRL_KISS_RX_DONE);

		// wait infinite amount of time for event from a serial port indicating that
		const EventBits_t bits_on_event = xEventGroupWaitBits (main_eventgroup_handle_serial_kiss,
															   MAIN_EVENTGROUP_SERIAL_KISS_RX_DONE,
															   pdTRUE,
															   pdTRUE,
															   0xFFFFFFFFu);

		SUPERVISOR_MONITOR_SET_CHECKPOINT(EVENT_SRL_KISS_RX_DONE, 1);

		// check if the event was really generated
		if (bits_on_event == MAIN_EVENTGROUP_SERIAL_KISS_RX_DONE) {
			SUPERVISOR_MONITOR_SET_CHECKPOINT(EVENT_SRL_KISS_RX_DONE, 2);

			// check if KISS communication with host-PC is enabled
			if (main_kiss_enabled == 1) {

				// if there were an error during receiving frame from host, restart rxing once again
				if (ctx->srl_rx_state == SRL_RX_ERROR) {
					SUPERVISOR_MONITOR_SET_CHECKPOINT(EVENT_SRL_KISS_RX_DONE, 3);

					srl_receive_data_kiss_protocol (main_kiss_srl_ctx_ptr, 120);
				}

				else if (ctx->srl_rx_state == SRL_RX_DONE) {
					SUPERVISOR_MONITOR_SET_CHECKPOINT(EVENT_SRL_KISS_RX_DONE, 4);

					// parse i ncoming data and then transmit on radio freq
					const int ln = kiss_parse_received (srl_get_rx_buffer (ctx),
														srl_get_num_bytes_rxed (ctx),
														&main_ax25,
														&main_afsk,
														task_event_kiss_buffer,
														KISS_CONFIG_DIAGNOSTIC_BUFFER_LN,
														KISS_TRANSPORT_SERIAL_PORT);

					SUPERVISOR_MONITOR_SET_CHECKPOINT(EVENT_SRL_KISS_RX_DONE, 5);
					if (ln == 0) {
						kiss10m++; // increase kiss messages counter
					}
					else if (ln > 0) {
						SUPERVISOR_MONITOR_SET_CHECKPOINT(EVENT_SRL_KISS_RX_DONE, 6);
						// if a response (ACK) to this KISS frame shall be sent

						// wait for any pending transmission to complete
						srl_wait_for_tx_completion (main_kiss_srl_ctx_ptr);

						SUPERVISOR_MONITOR_SET_CHECKPOINT(EVENT_SRL_KISS_RX_DONE, 7);

						srl_send_data (main_kiss_srl_ctx_ptr,
									   task_event_kiss_buffer,
									   SRL_MODE_DEFLN,
									   ln,
									   SRL_INTERNAL);

						SUPERVISOR_MONITOR_SET_CHECKPOINT(EVENT_SRL_KISS_RX_DONE, 8);
					}

					// restart KISS receiving to be ready for next frame
					srl_receive_data_kiss_protocol (main_kiss_srl_ctx_ptr, 120);
				}

				SUPERVISOR_MONITOR_SET_CHECKPOINT(EVENT_SRL_KISS_RX_DONE, 9);
			}
		}
		supervisor_iam_alive(SUPERVISOR_THREAD_EVENT_SRL_KISS_RX_DONE);
	}	// while(1)
}
