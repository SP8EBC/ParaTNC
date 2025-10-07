/*
 * task_event_serial_kiss_tx_done.c
 *
 *  Created on: Sep 28, 2025
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
#include "kiss_communication/kiss_callback.h"
#include "kiss_communication/types/kiss_communication_transport_t.h"

#include "supervisor.h"

void task_event_kiss_tx_done (void *param)
{
	(void)param;

	// another pointer to serial kiss context. with shorter name for convenience ;)
	srl_context_t *ctx = main_kiss_srl_ctx_ptr;

	while (1) {
		const EventBits_t bits_on_event = xEventGroupWaitBits (main_eventgroup_handle_serial_kiss,
															   MAIN_EVENTGROUP_SERIAL_KISS_TX_DONE,
															   pdTRUE,
															   pdTRUE,
															   0xFFFFFFFFu);
		// check if the event was really generated
		if (bits_on_event == MAIN_EVENTGROUP_SERIAL_KISS_TX_DONE) {
			// running config
			if (kiss_current_async_message != 0xFF) {
				srl_start_tx(ctx, kiss_async_pooler(ctx->srl_tx_buf_pointer, ctx->srl_tx_buf_ln));
			}
		}

		supervisor_iam_alive(SUPERVISOR_THREAD_EVENT_SRL_KISS_TX_DONE);
	}
}
