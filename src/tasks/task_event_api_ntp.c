/*
 * task_event_api_ntp.c
 *
 *  Created on: Oct 6, 2025
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

#include "http_client/http_client.h"
#include "api/api.h"

void task_event_api_ntp (void *param)
{
	(void)param;

	while (1) {
		// wait infinite amount of time for event from a serial port indicating that
		const EventBits_t bits_on_event =
			xEventGroupWaitBits (main_eventgroup_handle_ntp_and_api_client,
								 MAIN_EVENTGROUP_API_NTP,
								 pdFALSE,
								 pdFALSE,
								 0xFFFFFFFFu);

		if ((bits_on_event & MAIN_EVENTGROUP_API_NTP_SEND_EVENT_LOG) != 0) {

			// additional check against data races
			if (rte_main_events_extracted_for_api_stat.zz_total > 0) {
				const uint8_t current_event_index =
					rte_main_events_extracted_for_api_stat.zz_total - 1;

				// send current event
				const uint8_t api_connection_result =
					api_send_json_event (&rte_main_exposed_events[current_event_index]);

				// if TCP connection is established and data is currently sent asynchronously
				if (api_connection_result == HTTP_CLIENT_OK) {
					// end decrement remaining number of events
					rte_main_events_extracted_for_api_stat.zz_total--;
				}
				else {
					// for sake of simplicity break on first connection error
					rte_main_events_extracted_for_api_stat.zz_total = 0;
				}
			}
			xEventGroupClearBits (main_eventgroup_handle_ntp_and_api_client,
								  MAIN_EVENTGROUP_API_NTP_SEND_EVENT_LOG);
		}

		if ((bits_on_event & MAIN_EVENTGROUP_API_NTP_SYNC_TIME) != 0) {
			xEventGroupClearBits (main_eventgroup_handle_ntp_and_api_client,
								  MAIN_EVENTGROUP_API_NTP_SYNC_TIME);
		}
	}	// while(1)
}
