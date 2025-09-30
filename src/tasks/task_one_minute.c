/*
 * task_one_minute.c
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
#include "rte_wx.h"

#include "packet_tx_handler.h"
#include "io.h"
#include "backup_registers.h"
#include "gsm_comm_state_handler.h"

#include "events_definitions/events_main.h"
#include "http_client/http_client.h"
#include "nvm/nvm_event.h"

#include "etc/dallas_temperature_limits.h"

//! one hour interval incremented inside one minute
static int8_t main_one_hour_pool_timer = 60;

//! six hour interval incremented inside one hour
static int8_t main_six_hour_pool_timer = 4;

void task_one_minute (void *unused)
{
	/* Block for 60 seconds. */
	const TickType_t xDelay = 60000 / portTICK_PERIOD_MS;

	while (1) {
		vTaskDelay (xDelay);

		xEventGroupClearBits (main_eventgroup_handle_powersave, MAIN_EVENTGROUP_PWRSAVE_ONE_MIN);

		backup_reg_set_monitor (4);

		// main_nvm_timestamp = main_get_nvm_timestamp();

#ifdef SX1262_IMPLEMENTATION
//				fanet_success_cnt = 0;
//				fanet_fail_cnt = 0;
//				fanet_tx_success_cnt = 0;
#endif

#ifndef _MUTE_OWN
		packet_tx_handler (main_config_data_basic, main_config_data_mode);
#endif

		backup_reg_set_monitor (5);

#ifdef STM32L471xx
		if (main_config_data_mode->gsm == 1 && (io_get_cntrl_vbat_g () == 1)) {

			if (http_client_connection_errors > HTTP_CLIENT_MAX_CONNECTION_ERRORS) {
				NVIC_SystemReset ();
			}
		}

		// send event log each 24 hours, but only once at the top of an hour
		if (main_get_rtc_datetime (MAIN_GET_RTC_HOUR) == 21) {
			if (backup_reg_get_event_log_report_sent_radio () == 0) {
				// set status bit in non-volatile backup register not to loop over and over again in
				// case of a restart
				backup_reg_set_event_log_report_sent_radio ();

				// extract events from NVM
				const nvm_event_result_stats_t events_stat =
					nvm_event_get_last_events_in_exposed (rte_main_exposed_events,
														  MAIN_HOW_MANY_EVENTS_SEND_REPORT,
														  EVENT_WARNING);

				// set a trigger to number of events, which shall be sent
				// please note that we do not need to check here if APRS-IS
				// is connected. The check is done within specific APRS-IS functions
				//
				// definition MAIN_HOW_MANY_EVENTS_SEND_REPORT is not used here
				// because NVM can contain less events
				rte_main_trigger_radio_event_log = events_stat.zz_total;
			}
		}
		else {
			// reset flag if the time is not 21:xx
			backup_reg_reset_event_log_report_sent_radio ();
		}

		if ((main_config_data_gsm->aprsis_enable != 0) && (main_config_data_mode->gsm == 1) &&
			gsm_comm_state_get_current () == GSM_COMM_APRSIS) {

			// send event log each 24 hours, but only once at the top of an hour
			if (main_get_rtc_datetime (MAIN_GET_RTC_HOUR) == 19) {
				if (backup_reg_get_event_log_report_sent_aprsis () == 0) {
					// set status bit in non-volatile backup register not to loop over and over
					// again in case of a restart
					backup_reg_set_event_log_report_sent_aprsis ();

					// extract events from NVM
					const nvm_event_result_stats_t events_stat =
						nvm_event_get_last_events_in_exposed (rte_main_exposed_events,
															  MAIN_HOW_MANY_EVENTS_SEND_REPORT,
															  EVENT_WARNING);

					// set a trigger to number of events, which shall be sent
					// please note that we do not need to check here if APRS-IS
					// is connected. The check is done within specific APRS-IS functions
					//
					// definition MAIN_HOW_MANY_EVENTS_SEND_REPORT is not used here
					// because NVM can contain less events
					rte_main_trigger_gsm_event_log = events_stat.zz_total;

					xEventGroupSetBits (main_eventgroup_handle_aprs_trigger,
										MAIN_EVENTGROUP_APRSIS_TRIG_EVENTS);
				}
			}
			else {
				// reset flag if the time is not 18:xx
				backup_reg_reset_event_log_report_sent_aprsis ();
			}
		}

		if ((main_config_data_gsm->aprsis_enable != 0) && (main_config_data_mode->gsm == 1) &&
			(pwr_save_is_currently_cutoff () == 0) && (io_get_cntrl_vbat_g () == 1)) {
			// this checks when APRS-IS was alive last time and when any packet
			// has been sent to the server.
			const int i_am_ok_with_aprsis = aprsis_check_connection_attempt_alive ();

			if (i_am_ok_with_aprsis != 0) {

				// increase counter stored in RTC backup register
				backup_reg_increment_aprsis_check_reset ();

				// trigger a restart
				NVIC_SystemReset ();
			}
		}
#endif
		if (configuration_get_validate_parameters () == 1) {
			if (rte_wx_check_weather_measurements () == 0) {
				backup_reg_increment_weather_measurements_check_reset ();

				NVIC_SystemReset ();
			}

			if (rte_wx_dallas_degraded_counter > DALLAS_MAX_LIMIT_OF_DEGRADED) {
				backup_reg_increment_dallas_degraded_reset ();

				rte_main_reboot_req = 1;
			}
		}

		/**
		 * ONE HOUR POOLING
		 */
		if (--main_one_hour_pool_timer < 0) {
			main_one_hour_pool_timer = 60;

#if defined(STM32L471xx)
			// check if RTC is working correctly
			if (system_is_rtc_ok () == 0) {

				backup_reg_increment_is_rtc_ok_check_reset ();

				rte_main_reboot_req = 1;
			}

			if ((main_config_data_gsm->aprsis_enable != 0) && (main_config_data_mode->gsm == 1)) {
				xEventGroupSetBits (main_eventgroup_handle_aprs_trigger,
									MAIN_EVENTGROUP_APRSIS_TRIG_APRSIS_COUNTERS);
				// rte_main_trigger_gsm_aprsis_counters_packet = 1;
			}
#endif
		} // end of one hour

		/**
		 * SIX HOUR POOLING
		 */
		if (--main_six_hour_pool_timer < 0) {
			main_six_hour_pool_timer = 6;

			event_log_sync (EVENT_INFO_CYCLIC,
							EVENT_SRC_MAIN,
							EVENTS_MAIN_CYCLIC,
							aprsis_get_successfull_conn_counter (),
							aprsis_get_unsucessfull_conn_counter (),
							aprsis_get_tx_counter (),
							rte_main_average_battery_voltage,
							rte_main_rx_total,
							rte_main_tx_total);
		}

		main_one_minute_pool_timer = 60000;

		xEventGroupSetBits (main_eventgroup_handle_powersave, MAIN_EVENTGROUP_PWRSAVE_ONE_SEC);
	}
}
