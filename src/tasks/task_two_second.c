/*
 * task_two_second.c
 *
 *  Created on: Aug 5, 2025
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

#include "gsm_comm_state_handler.h"
#include "io.h"
#include "ntp.h"
#include "supervisor.h"
#include "wx_handler.h"
#include "wx_pwr_switch.h"

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
		SUPERVISOR_MONITOR_CLEAR (TASK_TWO_SEC);

		vTaskDelay (xDelay);

		SUPERVISOR_MONITOR_SET_CHECKPOINT (TASK_TWO_SEC, 1);

		xEventGroupClearBits (main_eventgroup_handle_powersave, MAIN_EVENTGROUP_PWRSAVE_TWO_SEC);

		main_get_tasks_stats ();

		SUPERVISOR_MONITOR_SET_CHECKPOINT (TASK_TWO_SEC, 2);

		if (main_config_data_mode->wx != 0) {
			// TODO:
			if (configuration_get_inhibit_wx_pwr_handle () == 0) {
				wx_pwr_switch_periodic_handle ();
			}

			wx_check_force_i2c_reset ();

			SUPERVISOR_MONITOR_SET_CHECKPOINT (TASK_TWO_SEC, 3);
		}
		else {
			// make supervisor happy if weather station mode is not enabled
			supervisor_iam_alive (SUPERVISOR_THREAD_SEND_WX);
		}

		if (rte_main_curret_powersave_mode != PWSAVE_AGGRESV) {
			SUPERVISOR_MONITOR_SET_CHECKPOINT (TASK_TWO_SEC, 4);

			if (configuration_get_power_cycle_vbat_r () == 1 && !main_afsk.sending) {
				io_pool_vbat_r ();
			}
		}
		else {
			io_inhibit_pool_vbat_r ();
		}

		if (io_get_cntrl_vbat_s () == 1) {
			SUPERVISOR_MONITOR_SET_CHECKPOINT (TASK_TWO_SEC, 5);

			max31865_pool ();
		}

		SUPERVISOR_MONITOR_SET_CHECKPOINT (TASK_TWO_SEC, 6);

		// everything here should be inhibited if controller is in aggressive powersaving mode
		// we do not want to mess up with the NTP and the rest of stuff then. Simply send
		// bare minimum -> weather packet
		if (pwr_save_is_currently_in_aggressive() == 0) {
			if (io_get_cntrl_vbat_g () == 1) {
				SUPERVISOR_MONITOR_SET_CHECKPOINT (TASK_TWO_SEC, 7);

				if (main_config_data_mode->gsm == 1 && io_get_cntrl_vbat_g () == 1 &&
					rte_main_woken_up == 0) {
					SUPERVISOR_MONITOR_SET_CHECKPOINT (TASK_TWO_SEC, 8);

					gsm_comm_state_handler (gsm_sim800_engineering_get_is_done (),
											ntp_done,
											rte_main_events_extracted_for_api_stat.zz_total,
											gsm_sim800_gprs_ready);
				}

				// if GSM module is enabled and GPRS communication state is now on API phase
				if ((main_config_data_mode->gsm == 1) &&
					(gsm_comm_state_get_current () == GSM_COMM_API)) {

					// if there are any events remaining to push to API
					if (rte_main_events_extracted_for_api_stat.zz_total > 0) {
						xEventGroupSetBits (main_eventgroup_handle_ntp_and_api_client,
											MAIN_EVENTGROUP_API_NTP_SEND_EVENT_LOG);
					}
				}

				if (gsm_comm_state_get_current () == GSM_COMM_NTP) {
					ntp_done = 1;
				}
			}
		}
		else {
			// this call will cheat the connection state handler that ntp
			// synchronization was done already, and an events queue has been sent
			// to the api. This is to be sure that even in aggressive powersave
			// controller will not waste time, and will instantly reconnect to APRS-IS
			gsm_comm_state_handler(1, 1, 0, 1);
		}

		SUPERVISOR_MONITOR_SET_CHECKPOINT (TASK_TWO_SEC, 9);
		main_reload_internal_wdg ();

		main_two_second_pool_timer = 2000;
		xEventGroupSetBits (main_eventgroup_handle_powersave, MAIN_EVENTGROUP_PWRSAVE_TWO_SEC);

		supervisor_iam_alive (SUPERVISOR_THREAD_TASK_TWO_SEC);

	} // while(1)
	// end of while loop
}
