/*
 * task_fanet.c
 *
 *  Created on: Nov 1, 2025
 *      Author: mateusz
 */

#include "main.h"
#include "main_freertos_externs.h"
#include "main_getters_for_task.h"

#include "fanet_app.h"

#include <FreeRTOS.h>
#include <task.h>

#include "event_log.h"
#include "events_definitions/events_fanet.h"

static uint8_t fanet_fail_counter = 0;
static uint8_t fanet_success_counter = 0;

void task_fanet (void *parameters)
{
	(void)parameters;
	while (1) {
		// wait infinite amount of time for event from a serial port indicating that
		const EventBits_t bits_on_event = xEventGroupWaitBits (main_eventgroup_handle_fanet,
															   MAIN_EVENTGROUP_FANET_SEND_METEO,
															   pdTRUE,
															   pdTRUE,
															   0xFFFFFFFFu);

		if (bits_on_event == MAIN_EVENTGROUP_FANET_SEND_METEO) {
			xEventGroupClearBits (main_eventgroup_handle_powersave, MAIN_EVENTGROUP_PWRSAVE_FANET);

			const int retval = fanet_test ();

			if (retval != 0) {
				fanet_fail_counter++;
				event_log_sync (EVENT_ERROR,
								EVENT_SRC_FANET,
								EVENTS_FANET_FAIL_TO_SEND_METEO,
								fanet_fail_counter,
								fanet_success_counter,
								0,
								0,
								0xDDCCBBAA,
								retval);
			}
			else {
				fanet_success_counter++;
			}
			xEventGroupSetBits (main_eventgroup_handle_powersave, MAIN_EVENTGROUP_PWRSAVE_FANET);

		}
	}
}
