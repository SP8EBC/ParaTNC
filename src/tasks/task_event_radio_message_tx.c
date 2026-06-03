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

#include "supervisor.h"


void task_event_radio_message_tx (void *param)
{
	(void)param;
	while (1) {

		// wait infinite amount of time for event from a serial port indicating that
		(void)xEventGroupWaitBits (main_eventgroup_handle_radio_message_transmit,
								   MAIN_EVENTGROUP_RADIO_MESSAGE_TXED,
								   pdTRUE,
								   pdTRUE,
								   0xFFFFFFFFu);


		//main_callback_post_tx ();
	}
}
