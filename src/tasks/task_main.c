/*
 * task_main.c
 *
 *  Created on: Jul 28, 2025
 *      Author: mateusz
 */

#include <stdint.h>
#include <string.h>

#include "main.h"
#include "main_freertos_externs.h"
#include "main_getters_for_task.h"
#include "main_gsm_pool_handler.h"
#include "rte_main.h"
#include "rte_wx.h"

#include "events_definitions/events_main.h"
#include "events_definitions/events_pwr_save.h"

#include "etc/kiss_configuation.h"
#include "etc/misc_config.h"
#include "etc/pwr_save_configuration.h"
#include "etc/serial_config.h"

#include "drivers/max31865.h"

#include "api/api.h"
#include "aprs/afsk_pr.h"
#include "aprs/ax25.h"
#include "aprs/status.h"
#include "davis_vantage/davis.h"
#include "davis_vantage/davis_parsers.h"
#include "http_client/http_client.h"
#include "kiss_communication/kiss_communication.h"
#include "modbus_rtu/rtu_serial_io.h"
#include "umb_master/umb_0x26_status.h"

#include "backup_registers.h"
#include "supervisor.h"

#include "LedConfig.h"
#include "TimerConfig.h"
#include "adc.h"
#include "aprsis.h"
#include "button.h"
#include "digi.h"
#include "io.h"
#include "ntp.h"
#include "pwr_save.h"
#include "wx_handler.h"
#include "wx_pwr_switch.h"

#if defined(PARAMETEO)
#include "gsm/sim800c_engineering.h"
#include "gsm/sim800c_gprs.h"
#include "gsm/sim800c_poolers.h"
#include "gsm/sim800c_tcpip.h"
#include <stm32l4xx.h>
#include <system_stm32l4xx.h>
#endif

#include <FreeRTOS.h>
#include <task.h>

// static uint8_t main_continue_loop = 0;

//!< Triggers additional check if ADC has properly reinitialized and conversion is working
uint8_t main_check_adc = 0;

//!< Used to store an information which telemetry descritpion frame should be sent next
// static telemetry_description_t main_telemetry_description = TELEMETRY_NOTHING;

static void message_callback (struct AX25Msg *msg)
{
	(void)msg;
}

void task_main (void *parameters)
{

	(void)(parameters);

	// int32_t ln = 0;
	// srl_context_t*  kiss_srl_ctx = main_get_kiss_srl_ctx_ptr();

	/* Block for 200ms. */
	const TickType_t xDelay = 200 / portTICK_PERIOD_MS;

	while (1) {
		if (supervisor_is_started () == 0) {
			// start the supervisor
			supervisor_start ();

			// and also set all events for 'main_eventgroup_handle_powersave'
			// in case that some options like GPRS/GSM modem or Modbus-RTU is disabled
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
			xEventGroupSetBits (main_eventgroup_handle_powersave,
								MAIN_EVENTGROUP_PWRSAVE_EV_APRS_TRIG);
			xEventGroupSetBits (main_eventgroup_handle_powersave, MAIN_EVENTGROUP_PWRSAVE_FANET);

			// this is required because task powersave will trip the supervisor before 1 minute
			// task will execute
			xEventGroupSetBits (main_eventgroup_handle_powersave, MAIN_EVENTGROUP_PWRSAVE_ONE_MIN);
		}

		SUPERVISOR_MONITOR_CLEAR (MAIN_LOOP);

		// TODO: fixme please :)
		vTaskDelay (xDelay);

		SUPERVISOR_MONITOR_SET_CHECKPOINT (MAIN_LOOP, 1);

		// system reset may be requested from various places in the application
		if (rte_main_reboot_req == 1) {
			// vTaskDelay( xDelay );

			NVIC_SystemReset ();
		}
		else {
			;
		}

		if (rte_main_woken_up == RTE_MAIN_GO_TO_INTERMEDIATE_SLEEP) {
			event_log_sync (EVENT_INFO,
							EVENT_SRC_PWR_SAVE,
							EVENTS_PWR_SAVE_GO_TO_INTERMEDIATE_SLEEP,
							0,
							0,
							0,
							0,
							0,
							0);

			// this sleep is used by @link{pwr_save_after_stop2_rtc_wakeup_it} to
			// go to the intermediate sleep in L4 powersave mode, when xxx seconds
			// long sleep is divided into 'PWR_SAVE_STOP2_CYCLE_LENGHT_SEC' to
			// service IWDG in between
			system_clock_configure_auto_wakeup_l4 (PWR_SAVE_STOP2_CYCLE_LENGHT_SEC);

			pwr_save_enter_stop2 ();
		}
		else if (rte_main_woken_up == RTE_MAIN_WOKEN_UP_RTC_INTERRUPT) {
			// controller is woken up from sleep in stop2 mode, now it must
			// be checked if this was the last sleep in the row, or micro
			// must go to sleep (stop2 mode) once again
			pwr_save_after_stop2_rtc_wakeup_it ();
		}
		else if (rte_main_woken_up == RTE_MAIN_WOKEN_UP_AFTER_LAST_SLEEP) {
			event_log_sync (EVENT_INFO,
							EVENT_SRC_PWR_SAVE,
							EVENTS_PWR_SAVE_WOKEN_UP_AFTER_LAST_SLEEP,
							0,
							0,
							0,
							0,
							0,
							0);

			system_clock_configure_l4 ();

			pwr_save_exit_after_last_stop2_cycle ();

			rte_main_woken_up = RTE_MAIN_WOKEN_UP_EXITED;
		}
		else if (rte_main_woken_up == RTE_MAIN_WOKEN_UP_EXITED) {

			// restart ADCs
			io_vbat_meas_enable ();

			// get current battery voltage and calculate current average
			rte_main_battery_voltage = io_vbat_meas_get_synchro_old ();
			rte_main_average_battery_voltage = io_vbat_meas_average (rte_main_battery_voltage);

			event_log_sync (EVENT_INFO,
							EVENT_SRC_PWR_SAVE,
							EVENTS_PWR_SAVE_WOKEN_UP_EXITED,
							0,
							0,
							rte_main_battery_voltage,
							rte_main_average_battery_voltage,
							0,
							0);

			// meas average will return 0 if internal buffer isn't filled completely
			if (rte_main_average_battery_voltage == 0) {
				rte_main_average_battery_voltage = rte_main_battery_voltage;
			}

			// reinitialize APRS radio modem to clear all possible intermittent state caused by
			// switching power state in the middle of APRS packet reception
			ax25_new_msg_rx_flag = 0;
			main_ax25.dcd = false;

			// DA_Init();
			ADCStartConfig ();
			DACStartConfig ();
			AFSK_Init (&main_afsk);
			ax25_init (&main_ax25, &main_afsk, 0, message_callback, 0);
			TimerAdcEnable ();

			// artificially report all tasks to the supervisor
			// not to trip a restart after waking up from sleep
			for (size_t i = 0; i < SUPERVISOR_THREAD_COUNT; i++) {
				supervisor_iam_alive ((supervisor_watchlist_t)i);
			}

			rte_main_woken_up = 0;

			rte_main_reset_gsm_modem = 1;

			main_check_adc = 1;

			// reinitialize UART used to communicate with GPRS modem
			srl_init (main_gsm_srl_ctx_ptr,
					  USART3,
					  srl_usart3_rx_buffer,
					  RX_BUFFER_3_LN,
					  srl_usart3_tx_buffer,
					  TX_BUFFER_3_LN,
					  115200,
					  1);
		}
		else {

			SUPERVISOR_MONITOR_SET_CHECKPOINT (MAIN_LOOP, 2);
			xEventGroupClearBits (main_eventgroup_handle_powersave, MAIN_EVENTGROUP_PWRSAVE_MAIN);

			// if Victron VE.direct client is enabled
			if (main_config_data_mode->victron == 1) {}
			else {
			}

			// if Davis wx station is enabled and it is alive
			if (main_get_main_davis_serial_enabled () == 1) {

				// pool the Davis wx station driver for LOOP packet
				davis_loop_packet_pooler (&rte_wx_davis_loop_packet_avaliable);

				davis_rxcheck_packet_pooler ();
			}

			SUPERVISOR_MONITOR_SET_CHECKPOINT (MAIN_LOOP, 3);

			if (main_config_data_mode->wx_umb == 1) {
				SUPERVISOR_MONITOR_SET_CHECKPOINT (MAIN_LOOP, 4);

				//					// if some UMB data have been received
				//					if (main_wx_srl_ctx_ptr->srl_rx_state == SRL_RX_DONE) {
				//						umb_pooling_handler(&rte_wx_umb_context,
				// REASON_RECEIVE_IDLE, master_time, main_config_data_umb);
				//
				//						if (rte_wx_umb_context.state ==
				// UMB_STATUS_RESPONSE_AVALIABLE) { led_blink_led2_botoom();
				//						}
				//					}
				//
				//					// if there were an error during receiving frame from host,
				// restart rxing once again 					if
				// (main_wx_srl_ctx_ptr->srl_rx_state == SRL_RX_ERROR) {
				//						umb_pooling_handler(&rte_wx_umb_context,
				// REASON_RECEIVE_ERROR, master_time, main_config_data_umb);
				//					}
				//
				if (main_wx_srl_ctx_ptr->srl_tx_state == SRL_TX_IDLE) {
					xEventGroupSetBits (main_eventgroup_handle_serial_sensor,
										MAIN_EVENTGROUP_SERIAL_WX_TX_DONE);
					// umb_pooling_handler(&rte_wx_umb_context, REASON_TRANSMIT_IDLE, master_time,
					// main_config_data_umb);
				}
			}
			// if modbus rtu master is enabled
			else if (main_get_modbus_rtu_master_enabled () == 1 && io_get_cntrl_vbat_m () == 1) {
				SUPERVISOR_MONITOR_SET_CHECKPOINT (MAIN_LOOP, 5);

				if (rte_main_reset_modbus_rtu == 1) {
					rte_main_reset_modbus_rtu = 0;

					//				  rtu_serial_init(&rte_rtu_pool_queue, 1, main_wx_srl_ctx_ptr,
					// main_config_data_rtu);
					//
					//				  main_target_wx_baudrate = main_config_data_rtu->slave_speed;
					//
					//				  srl_init(main_wx_srl_ctx_ptr, USART2, srl_usart2_rx_buffer,
					// RX_BUFFER_2_LN, srl_usart2_tx_buffer, TX_BUFFER_2_LN,
					// main_target_wx_baudrate, main_config_data_rtu->slave_stop_bits);
					//				  srl_switch_tx_delay(main_wx_srl_ctx_ptr, 1);
					//
					//				  rtu_serial_start();
				}
				else {
					SUPERVISOR_MONITOR_SET_CHECKPOINT (MAIN_LOOP, 6);

					rtu_serial_pool ();
				}
			}

			SUPERVISOR_MONITOR_SET_CHECKPOINT (MAIN_LOOP, 7);

			button_check_all (main_get_button_one_left (), main_get_button_two_right ());

			// get all meteo measuremenets each 65 seconds. some values may not be
			// downloaded from sensors if _METEO and/or _DALLAS_AS_TELEM aren't defined
			if (main_wx_sensors_pool_timer < 10) {
				if ((main_config_data_mode->wx & WX_ENABLED) == 1 &&
					(io_get_cntrl_vbat_s () == 1)) {
					SUPERVISOR_MONITOR_SET_CHECKPOINT (MAIN_LOOP, 8);

					// notice: UMB-master and Modbus-RTU uses the same UART
					// so they cannot be enabled both at once

					// check if modbus rtu master is enabled and configured properly
					if (main_get_modbus_rtu_master_enabled () == 1) {
						// start quering all Modbus RTU devices & registers one after another
						rtu_serial_start ();
					}
					else if (main_config_data_mode->wx_umb == 1) {
						// request status from the slave if UMB master is enabled
						umb_0x26_status_request (&rte_wx_umb,
												 &rte_wx_umb_context,
												 main_config_data_umb);
					}
					else {
						;
					}

					SUPERVISOR_MONITOR_SET_CHECKPOINT (MAIN_LOOP, 9);

					// davis serial weather station is connected using UART / RS232 used normally
					// for KISS communcation between modem and host PC
					if (main_get_main_davis_serial_enabled () == 1) {
						davis_trigger_rxcheck_packet ();
					}

					// get all measurements from 'internal' sensors (except wind which is handled
					// separately)
					wx_get_all_measurements (main_config_data_wx_sources,
											 main_config_data_mode,
											 main_config_data_umb,
											 main_config_data_rtu);

					SUPERVISOR_MONITOR_SET_CHECKPOINT (MAIN_LOOP, 10);
				}

				main_wx_sensors_pool_timer = 65500;
			}

			/**
			 * FOUR SECOND POOLING
			 */
			if (main_four_second_pool_timer < 10) {
				main_four_second_pool_timer = 4000;
				SUPERVISOR_MONITOR_SET_CHECKPOINT (MAIN_LOOP, 11);

				if (rte_main_trigger_radio_event_log > 0 && io_get_cntrl_vbat_r () == 1) {
					SUPERVISOR_MONITOR_SET_CHECKPOINT (MAIN_LOOP, 12);

					// set a pointer to even in exposed form which will be sent now
					const event_log_exposed_t *current_exposed_event =
						&rte_main_exposed_events[rte_main_trigger_radio_event_log - 1];

					status_send_from_exposed_eveny_log (current_exposed_event);

					rte_main_trigger_radio_event_log--;
				}
			} // end of four second pooling
			SUPERVISOR_MONITOR_SET_CHECKPOINT (MAIN_LOOP, 13);
			xEventGroupSetBits (main_eventgroup_handle_powersave, MAIN_EVENTGROUP_PWRSAVE_MAIN);

		} // else under if (rte_main_woken_up == RTE_MAIN_WOKEN_UP_EXITED)
		main_get_tasks_stats ();

		supervisor_iam_alive (SUPERVISOR_THREAD_MAIN_LOOP);

	} // Infinite loop, never return.
}
