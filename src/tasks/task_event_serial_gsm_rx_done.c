/*
 * task_event_serial_gsm_rx_done.c
 *
 *  Created on: Aug 22, 2025
 *      Author: mateusz
 */

#include <FreeRTOS.h>
#include <event_groups.h>
#include <stdbool.h>
#include <stdint.h>
#include <task.h>

#include "main.h"
#include "main_freertos_externs.h"
#include "supervisor.h"

#include "gsm/sim800c.h"

void task_event_gsm_rx_done (void *param)
{
	(void)param;

	// another pointer to serial kiss context. with shorter name for convenience ;)
	srl_context_t *ctx = main_gsm_srl_ctx_ptr;

	while (1) {
		SUPERVISOR_MONITOR_CLEAR(EVENT_SRL_GSM_RX_DONE);

		// wait infinite amount of time for event from a serial port indicating that
		const EventBits_t bits_on_event = xEventGroupWaitBits (main_eventgroup_handle_serial_gsm,
															   MAIN_EVENTGROUP_SERIAL_GSM_RX_DONE,
															   pdTRUE,
															   pdTRUE,
															   0xFFFFFFFFu);

		SUPERVISOR_MONITOR_SET_CHECKPOINT(EVENT_SRL_GSM_RX_DONE, 1);

		// check if the event was really generated
		if (bits_on_event == MAIN_EVENTGROUP_SERIAL_GSM_RX_DONE) {
			SUPERVISOR_MONITOR_SET_CHECKPOINT(EVENT_SRL_GSM_RX_DONE, 2);

			// receive callback for communicatio with the modem
			gsm_sim800_rx_done_event_handler(ctx, &main_gsm_state);
		}
		supervisor_iam_alive(SUPERVISOR_THREAD_EVENT_SRL_GSM_RX_DONE);

		SUPERVISOR_MONITOR_SET_CHECKPOINT(EVENT_SRL_GSM_RX_DONE, 3);

	}
}
