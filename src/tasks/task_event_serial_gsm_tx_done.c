/*
 * task_event_serial_gsm_tx_done.c
 *
 *  Created on: Sep 1, 2025
 *      Author: mateusz
 */

#include <FreeRTOS.h>
#include <event_groups.h>
#include <stdbool.h>
#include <stdint.h>
#include <task.h>

#include "main.h"
#include "main_freertos_externs.h"

#include "gsm/sim800c.h"

void task_event_gsm_tx_done (void *param)
{
	(void)param;

	// another pointer to serial kiss context. with shorter name for convenience ;)
	srl_context_t *ctx = main_gsm_srl_ctx_ptr;

	while (1) {
		// wait infinite amount of time for event from a serial port indicating that
		const EventBits_t bits_on_event = xEventGroupWaitBits (main_eventgroup_handle_serial_gsm,
															   MAIN_EVENTGROUP_SERIAL_GSM_TX_DONE,
															   pdTRUE,
															   pdTRUE,
															   0xFFFFFFFFu);

		// check if the event was really generated
		if (bits_on_event == MAIN_EVENTGROUP_SERIAL_GSM_TX_DONE) {
			gsm_sim800_tx_done_event_handler(ctx, &main_gsm_state);

		}
	}
}
