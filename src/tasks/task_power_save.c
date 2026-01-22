/*
 * task_power_save.c
 *
 *  Created on: Aug 16, 2025
 *      Author: mateusz
 */

#include <FreeRTOS.h>
#include <stdbool.h>
#include <stdint.h>
#include <task.h>

#include "main.h"
#include "main_freertos_externs.h"
#include "rte_main.h"
#include "supervisor.h"

#include "adc.h"
#include "backup_registers.h"
#include "io.h"
#include "packet_tx_handler.h"
#include "pwr_save.h"

volatile EventBits_t last_waiting_state = 0;

void task_power_save (void *parameters)
{
	(void)parameters;

	/* Block for 10000ms -> ten seconds */
	const TickType_t xDelay = 10000 / portTICK_PERIOD_MS;
	const TickType_t xTimeout = 22000 / portTICK_PERIOD_MS;

	int main_continue_loop = 0;

	while (1) {
		SUPERVISOR_MONITOR_CLEAR (TASK_POWERSAV);

		vTaskDelay (xDelay);

		SUPERVISOR_MONITOR_SET_CHECKPOINT (TASK_POWERSAV, 1);

		last_waiting_state = xEventGroupWaitBits (main_eventgroup_handle_powersave,
												  MAIN_EVENTGROUP_WAIT_FOR,
												  pdTRUE,
												  pdTRUE,
												  xTimeout);

		SUPERVISOR_MONITOR_SET_CHECKPOINT (TASK_POWERSAV, 2);

		supervisor_iam_alive (SUPERVISOR_THREAD_TASK_POWERSAV);

		// set these bits once again. they are controlled by events, which might
		// or might not be triggered between each powersave "loops"
		xEventGroupSetBits (main_eventgroup_handle_powersave,
							MAIN_EVENTGROUP_PWRSAVE_EV_SRL_KISS_RX);
		xEventGroupSetBits (main_eventgroup_handle_powersave,
							MAIN_EVENTGROUP_PWRSAVE_EV_SRL_KISS_TX);
		xEventGroupSetBits (main_eventgroup_handle_powersave,
							MAIN_EVENTGROUP_PWRSAVE_EV_SRL_GSM_RX);
		xEventGroupSetBits (main_eventgroup_handle_powersave,
							MAIN_EVENTGROUP_PWRSAVE_EV_SRL_GSM_TX);
		xEventGroupSetBits (main_eventgroup_handle_powersave,
							MAIN_EVENTGROUP_PWRSAVE_EV_SRL_SENSOR);
		xEventGroupSetBits (main_eventgroup_handle_powersave, MAIN_EVENTGROUP_PWRSAVE_EV_APRS_TRIG);
		xEventGroupSetBits (main_eventgroup_handle_powersave, MAIN_EVENTGROUP_PWRSAVE_FANET);

		main_suspend_task_for_psaving ();

		main_get_tasks_stats ();

		SUPERVISOR_MONITOR_SET_CHECKPOINT (TASK_POWERSAV, 3);

		// get current battery voltage. for non parameteo this will return 0
		// main_battery_measurement_res = io_vbat_meas_get(&rte_main_battery_voltage);
		rte_main_battery_voltage = io_vbat_meas_get_synchro_old ();
		rte_main_average_battery_voltage = io_vbat_meas_average (rte_main_battery_voltage);

		SUPERVISOR_MONITOR_SET_CHECKPOINT (TASK_POWERSAV, 4);

		// meas average will return 0 if internal buffer isn't filled completely
		if (rte_main_average_battery_voltage == 0) {
			rte_main_average_battery_voltage = rte_main_battery_voltage;
		}

		SUPERVISOR_MONITOR_SET_CHECKPOINT (TASK_POWERSAV, 5);

		if (rte_main_check_adc == 1) {
			AD_Restart ();

			rte_main_check_adc = 0;
		}

		// inhibit any power save switching when modem transmits data
		if (!main_afsk.sending && rte_main_woken_up == 0 &&
			packet_tx_is_gsm_meteo_pending () == 0) {
			SUPERVISOR_MONITOR_SET_CHECKPOINT (TASK_POWERSAV, 6);

			rte_main_curret_powersave_mode =
				pwr_save_pooling_handler (main_config_data_mode,
										  packet_tx_meteo_interval,
										  packet_tx_get_minutes_to_next_wx (),
										  rte_main_average_battery_voltage,
										  rte_main_battery_voltage,
										  &main_continue_loop);
		}

		SUPERVISOR_MONITOR_SET_CHECKPOINT (TASK_POWERSAV, 8);

		// this is set to zero after a call to "pwr_save_switch_mode_to_l7"
		if (main_continue_loop == 0) {
			continue;
		}

		SUPERVISOR_MONITOR_SET_CHECKPOINT (TASK_POWERSAV, 9);

		main_resume_task_for_psaving ();

		SUPERVISOR_MONITOR_SET_CHECKPOINT (TASK_POWERSAV, 10);
	} //  wile(1)
}
