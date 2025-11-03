/*
 * task_event_radio_message.c
 *
 *  Created on: Sep 30, 2025
 *      Author: mateusz
 */

#include <FreeRTOS.h>
#include <event_groups.h>
#include <stdbool.h>
#include <stdint.h>
#include <task.h>

#include "main.h"
#include "main_freertos_externs.h"

#include "aprsis.h"
#include "digi.h"
#include "supervisor.h"

#include "drivers/serial.h"
#include "kiss_communication/kiss_communication.h"

#include <string.h>

void task_event_radio_message (void *param)
{
	(void)param;

	// another pointer to serial kiss context. with shorter name for convenience ;)
	srl_context_t *ctx = main_kiss_srl_ctx_ptr;

	while (1) {
		SUPERVISOR_MONITOR_CLEAR(EVENT_NEW_RF);

		// wait infinite amount of time for event from a serial port indicating that
		(void)xEventGroupWaitBits (main_eventgroup_handle_radio_message,
								   MAIN_EVENTGROUP_RADIO_MESSAGE_RXED,
								   pdTRUE,
								   pdTRUE,
								   0xFFFFFFFFu);

		SUPERVISOR_MONITOR_SET_CHECKPOINT(EVENT_NEW_RF, 1);

		// if serial port is currently not busy on transmission
		if (ctx->srl_tx_state != SRL_TXING) {
			memset (ctx->srl_tx_buf_pointer, 0x00, ctx->srl_tx_buf_ln);

			if (main_kiss_enabled == 1) {
				// convert message to kiss format and send it to host
				srl_start_tx (main_kiss_srl_ctx_ptr,
							  kiss_send_ax25_to_host (ax25_rxed_frame.raw_data,
													  (ax25_rxed_frame.raw_msg_len - 2),
													  main_kiss_srl_ctx_ptr->srl_tx_buf_pointer,
													  main_kiss_srl_ctx_ptr->srl_tx_buf_ln));
			}
		}

		SUPERVISOR_MONITOR_SET_CHECKPOINT(EVENT_NEW_RF, 2);

		main_ax25.dcd = false;

		// check this frame against other frame in visvous buffer waiting to be transmitted
		digi_check_with_viscous (&ax25_rxed_frame);

		// check if this packet needs to be repeated (digipeated) and do it if it is necessary
		digi_process (&ax25_rxed_frame, main_config_data_basic, main_config_data_mode);

		SUPERVISOR_MONITOR_SET_CHECKPOINT(EVENT_NEW_RF, 3);

		ax25_new_msg_rx_flag = 0;
		rx10m++;
#ifdef PARAMETEO
		rte_main_rx_total++;

		// if aprsis is logged
		if (aprsis_connected == 1 && gsm_sim800_tcpip_tx_busy () == 0) {
			aprsis_igate_to_aprsis (&ax25_rxed_frame, (const char *)&main_callsign_with_ssid);
		}

#endif
		SUPERVISOR_MONITOR_SET_CHECKPOINT(EVENT_NEW_RF, 4);

		supervisor_iam_alive(SUPERVISOR_THREAD_EVENT_NEW_RF);

	}	// while(1)
}
