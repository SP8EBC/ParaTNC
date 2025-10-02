/*
 * task_two_second.c
 *
 *  Created on: Aug 5, 2025
 *      Author: mateusz
 */

#include <FreeRTOS.h>
#include <stdbool.h>
#include <stdint.h>
#include <task.h>
#include <event_groups.h>

#include "main.h"
#include "main_freertos_externs.h"
#include "rte_main.h"

#include "gsm_comm_state_handler.h"
#include "io.h"
#include "ntp.h"
#include "wx_handler.h"
#include "wx_pwr_switch.h"
#include "supervisor.h"

#include "api/api.h"
#include "drivers/max31865.h"
#include "gsm/sim800c_engineering.h"
#include "gsm/sim800c_gprs.h"
#include "http_client/http_client.h"

void task_two_second (void *parameters)
{
	(void)parameters;

	/* Block for 2000ms. */
	const TickType_t xDelay = 2000 / portTICK_PERIOD_MS;

	while (1) {
		vTaskDelay (xDelay);

		xEventGroupClearBits(main_eventgroup_handle_powersave, MAIN_EVENTGROUP_PWRSAVE_TWO_SEC);

		supervisor_iam_alive(SUPERVISOR_THREAD_TASK_TWO_SEC);

		if (main_config_data_mode->wx != 0) {
			// TODO:
			if (configuration_get_inhibit_wx_pwr_handle () == 0) {
				wx_pwr_switch_periodic_handle ();
			}

			wx_check_force_i2c_reset ();
		}

#ifdef PARAMETEO
		if (rte_main_curret_powersave_mode != PWSAVE_AGGRESV) {
			if (configuration_get_power_cycle_vbat_r () == 1 && !main_afsk.sending) {
				io_pool_vbat_r ();
			}
		}
		else {
			io_inhibit_pool_vbat_r ();
		}
#endif

#ifdef PARAMETEO

		if (io_get_cntrl_vbat_s () == 1) {
			max31865_pool ();
		}

		if (io_get_cntrl_vbat_g () == 1) {
			if (main_config_data_mode->gsm == 1 && io_get_cntrl_vbat_g () == 1 &&
				rte_main_woken_up == 0) {
				gsm_comm_state_handler (gsm_sim800_engineering_get_is_done (), ntp_done,
										rte_main_events_extracted_for_api_stat.zz_total,
										gsm_sim800_gprs_ready);
			}

			// if GSM module is enabled and GPRS communication state is now on API phase
			if ((main_config_data_mode->gsm == 1) &&
				(gsm_comm_state_get_current () == GSM_COMM_API)) {

				// if there are any events remaining to push to API
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
			}

			if (gsm_comm_state_get_current () == GSM_COMM_NTP) {
				ntp_get_sync ();
			}
		}
#endif
		main_reload_internal_wdg ();

		main_two_second_pool_timer = 2000;
		xEventGroupSetBits(main_eventgroup_handle_powersave, MAIN_EVENTGROUP_PWRSAVE_TWO_SEC);

	}
	// end of while loop
}
