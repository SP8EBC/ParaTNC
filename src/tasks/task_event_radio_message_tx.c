/*
 * task_event_radio_message_tx.c
 *
 *  Created on: May 28, 2026
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

void task_event_radio_message_tx (void *param)
{
	(void)param;
	while (1) {
		SUPERVISOR_MONITOR_CLEAR (EVENT_NEW_RF);

		// wait infinite amount of time for event from a serial port indicating that
		(void)xEventGroupWaitBits (main_eventgroup_handle_radio_message_transmit,
								   MAIN_EVENTGROUP_RADIO_MESSAGE_TXED,
								   pdTRUE,
								   pdTRUE,
								   0xFFFFFFFFu);
		main_callback_post_tx ();
	}
}
