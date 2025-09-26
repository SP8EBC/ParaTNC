/*
 * task_main.c
 *
 *  Created on: Jul 28, 2025
 *      Author: mateusz
 */

#include <stdint.h>
#include <string.h>

#include "main.h"
#include "main_getters_for_task.h"
#include "main_freertos_externs.h"
#include "main_gsm_pool_handler.h"
#include "rte_main.h"
#include "rte_wx.h"

#include "events_definitions/events_main.h"

#include "etc/pwr_save_configuration.h"
#include "etc/serial_config.h"
#include "etc/misc_config.h"
#include "etc/kiss_configuation.h"
#include "etc/dallas_temperature_limits.h"

#include "drivers/max31865.h"

#include "kiss_communication/kiss_communication.h"
#include "aprs/ax25.h"
#include "aprs/afsk_pr.h"
#include "aprs/status.h"
#include "http_client/http_client.h"
#include "api/api.h"
#include "davis_vantage/davis.h"
#include "davis_vantage/davis_parsers.h"
#include "modbus_rtu/rtu_serial_io.h"
#include "umb_master/umb_0x26_status.h"
#include "nvm/nvm_event.h"

#include "supervisor.h"
#include "backup_registers.h"

#include "pwr_save.h"
#include "io.h"
#include "adc.h"
#include "aprsis.h"
#include "digi.h"
#include "TimerConfig.h"
#include "LedConfig.h"
#include "button.h"
#include "wx_handler.h"
#include "wx_pwr_switch.h"
#include "gsm_comm_state_handler.h"
#include "packet_tx_handler.h"
#include "ntp.h"

#if defined(PARAMETEO)
#include <stm32l4xx.h>
#include <system_stm32l4xx.h>
#include "gsm/sim800c_tcpip.h"
#include "gsm/sim800c_gprs.h"
#include "gsm/sim800c_poolers.h"
#include "gsm/sim800c_engineering.h"
//#include "gsm/sim800c"
#endif

#include <FreeRTOS.h>
#include <task.h>

//! one hour interval incremented inside one minute
static int8_t main_one_hour_pool_timer = 60;

//! six hour interval incremented inside one hour
static int8_t main_six_hour_pool_timer = 4;

static uint8_t main_continue_loop = 0;

//!< Triggers additional check if ADC has properly reinitialized and conversion is working
uint8_t main_check_adc = 0;

//!< Used to store an information which telemetry descritpion frame should be sent next
static telemetry_description_t main_telemetry_description = TELEMETRY_NOTHING;



static void message_callback(struct AX25Msg *msg) {

}

void task_main( void * parameters )
{

	(void)(parameters);

	int32_t ln = 0;
	srl_context_t*  kiss_srl_ctx = main_get_kiss_srl_ctx_ptr();

	  while (1)
	    {
	    //vTaskDelay( xDelay );

		backup_reg_set_monitor(-1);

		supervisor_iam_alive(SUPERVISOR_THREAD_MAIN_LOOP);


		// system reset may be requested from various places in the application
		if (rte_main_reboot_req == 1) {
			NVIC_SystemReset();
		}
		else {
			;
		}

		  backup_reg_set_monitor(0);

	#if defined(PARAMETEO)
		  if (rte_main_woken_up == RTE_MAIN_GO_TO_INTERMEDIATE_SLEEP) {
				// this sleep is used by @link{pwr_save_after_stop2_rtc_wakeup_it} to
			  	// go to the intermediate sleep in L4 powersave mode, when xxx seconds
			  	// long sleep is divided into 'PWR_SAVE_STOP2_CYCLE_LENGHT_SEC' to
			    // service IWDG in between
				system_clock_configure_auto_wakeup_l4(PWR_SAVE_STOP2_CYCLE_LENGHT_SEC);

				pwr_save_enter_stop2();
		  }
		  else if (rte_main_woken_up == RTE_MAIN_WOKEN_UP_RTC_INTERRUPT) {
			  // controller is woken up from sleep in stop2 mode, now it must
			  // be checked if this was the last sleep in the row, or micro
			  // must go to sleep (stop2 mode) once again
			  	pwr_save_after_stop2_rtc_wakeup_it();
		  }
		  else if (rte_main_woken_up == RTE_MAIN_WOKEN_UP_AFTER_LAST_SLEEP) {
				system_clock_configure_l4();

				pwr_save_exit_after_last_stop2_cycle();

				rte_main_woken_up = RTE_MAIN_WOKEN_UP_EXITED;
		  }
		  else if (rte_main_woken_up == RTE_MAIN_WOKEN_UP_EXITED) {

		  		// restart ADCs
		  		io_vbat_meas_enable();

		  		// get current battery voltage and calculate current average
			    rte_main_battery_voltage = io_vbat_meas_get_synchro_old();
			    rte_main_average_battery_voltage = io_vbat_meas_average(rte_main_battery_voltage);

			    // meas average will return 0 if internal buffer isn't filled completely
			    if (rte_main_average_battery_voltage == 0) {
			    	rte_main_average_battery_voltage = rte_main_battery_voltage;
			    }

		  	    // reinitialize APRS radio modem to clear all possible intermittent state caused by
		  	    // switching power state in the middle of APRS packet reception
				ax25_new_msg_rx_flag = 0;
				main_ax25.dcd = false;

				//DA_Init();
				ADCStartConfig();
				DACStartConfig();
				AFSK_Init(&main_afsk);
				ax25_init(&main_ax25, &main_afsk, 0, message_callback, 0);
				TimerAdcEnable();

				rte_main_woken_up = 0;

				rte_main_reset_gsm_modem = 1;

		  		main_check_adc = 1;

				// reinitialize UART used to communicate with GPRS modem
				srl_init(main_gsm_srl_ctx_ptr, USART3, srl_usart3_rx_buffer, RX_BUFFER_3_LN, srl_usart3_tx_buffer, TX_BUFFER_3_LN, 115200, 1);

		  		backup_reg_set_monitor(1);
		  	}
		  	else {
	#endif

				backup_reg_set_monitor(11);

				// if new packet has been received from radio channel
				if(ax25_new_msg_rx_flag == 1) {

					// if serial port is currently not busy on transmission
					if (main_kiss_srl_ctx_ptr->srl_tx_state != SRL_TXING) {
						memset(kiss_srl_ctx->srl_tx_buf_pointer, 0x00, kiss_srl_ctx->srl_tx_buf_ln);

						if (main_kiss_enabled == 1) {
							// convert message to kiss format and send it to host
							srl_start_tx(main_kiss_srl_ctx_ptr, kiss_send_ax25_to_host(ax25_rxed_frame.raw_data, (ax25_rxed_frame.raw_msg_len - 2), main_kiss_srl_ctx_ptr->srl_tx_buf_pointer, main_kiss_srl_ctx_ptr->srl_tx_buf_ln));
						}
					}

					main_ax25.dcd = false;

					// check this frame against other frame in visvous buffer waiting to be transmitted
					digi_check_with_viscous(&ax25_rxed_frame);

					// check if this packet needs to be repeated (digipeated) and do it if it is necessary
					digi_process(&ax25_rxed_frame, main_config_data_basic, main_config_data_mode);

					ax25_new_msg_rx_flag = 0;
					rx10m++;
		#ifdef PARAMETEO
					rte_main_rx_total++;

					// if aprsis is logged
					if (aprsis_connected == 1 && gsm_sim800_tcpip_tx_busy() == 0) {
						aprsis_igate_to_aprsis(&ax25_rxed_frame, (const char *)&main_callsign_with_ssid);
					}

		#endif
				}

		#ifdef PARAMETEO
				// if GSM communication is enabled
				if (main_config_data_mode->gsm == 1  && io_get_cntrl_vbat_g() == 1) {
/*
					// pool all stuff related
					main_gsm_pool_handler(
											main_gsm_srl_ctx_ptr,			// 1
											&main_gsm_state,				// 2
											&rte_main_trigger_message_ack,	// 3
											&rte_main_received_message,		// 4
											main_kiss_from_message,			// 5
											&main_kiss_from_message_ln,		// 6
											main_kiss_response_message,		// 7
											&rte_main_message_for_transmitting,
											&rte_main_trigger_send_message,	// 9
											&rte_main_trigger_gsm_status,	// 10
											&rte_main_trigger_gsm_aprsis_counters_packet,
											&rte_main_trigger_gsm_loginstring_packet,
											&rte_main_trigger_gsm_telemetry_values,
											&rte_main_trigger_gsm_telemetry_descriptions,
											&main_telemetry_description,	// 15
											main_config_data_mode,			// 16
											main_config_data_basic,			// 17
											main_callsign_with_ssid,		// 18
											main_own_aprs_msg,
											&rte_main_trigger_gsm_event_log,
											rte_main_exposed_events);
*/
				}
		#endif

				// if Victron VE.direct client is enabled
				if (main_config_data_mode->victron == 1) {
		#ifndef PARAMETEO
					// if new KISS message has been received from the host
					if (main_kiss_srl_ctx_ptr->srl_rx_state == SRL_RX_DONE || main_kiss_srl_ctx_ptr->srl_rx_state == SRL_RX_ERROR) {

						// cutting received string to Checksum, everything after will be skipped
						ve_direct_cut_to_checksum(srl_get_rx_buffer(main_kiss_srl_ctx_ptr), TX_BUFFER_1_LN, &buffer_len);

						// checking if this frame is ok
						ve_direct_validate_checksum(srl_get_rx_buffer(main_kiss_srl_ctx_ptr), buffer_len, &retval);

						if (retval == 1) {
							// parsing data from input serial buffer to
							retval = ve_direct_parse_to_raw_struct(srl_get_rx_buffer(main_kiss_srl_ctx_ptr), buffer_len, &rte_pv_struct);

							if (retval == 0) {
								ve_direct_add_to_average(&rte_pv_struct, &rte_pv_average);

								ve_direct_get_averages(&rte_pv_average, &rte_pv_battery_current, &rte_pv_battery_voltage, &rte_pv_cell_voltage, &rte_pv_load_current);

								ve_direct_set_sys_voltage(&rte_pv_struct, &rte_pv_sys_voltage);

								ve_direct_store_errors(&rte_pv_struct, &rte_pv_last_error);

								rte_pv_messages_count++;
							}
						}
						else {
							rte_pv_corrupted_messages_count++;
						}

						//memset(srl_get_rx_buffer(main_kiss_srl_ctx_ptr), 0x00, TX_BUFFER_1_LN);

						srl_receive_data(main_kiss_srl_ctx_ptr, VE_DIRECT_MAX_FRAME_LN, 0, 0, 0, 0, 0);
					}
		#endif
				}
//				else if ((main_config_data_mode->wx_dust_sensor & WX_DUST_SDS011_SERIAL) > 0) {
//					if (main_kiss_srl_ctx_ptr->srl_rx_state == SRL_RX_DONE) {
//
//						sds011_get_pms(main_kiss_srl_ctx_ptr->srl_rx_buf_pointer, 10, &rte_wx_pm10, &rte_wx_pm2_5);
//
//						// restart reception
//						srl_receive_data(main_kiss_srl_ctx_ptr, 10, 0xAA, 0, 0, 0, 0);
//
//					}
//				}
				else {
					// if new KISS message has been received from the host
//					if (main_kiss_srl_ctx_ptr->srl_rx_state == SRL_RX_DONE && main_kiss_enabled == 1) {
//						// parse i ncoming data and then transmit on radio freq
//						ln = kiss_parse_received (srl_get_rx_buffer (kiss_srl_ctx),
//												  srl_get_num_bytes_rxed (main_kiss_srl_ctx_ptr),
//												  &main_ax25,
//												  &main_afsk,
//												  main_small_buffer,
//												  KISS_CONFIG_DIAGNOSTIC_BUFFER_LN,
//												  KISS_TRANSPORT_SERIAL_PORT);
//						if (ln == 0) {
//							kiss10m++; // increase kiss messages counter
//						}
//						else if (ln > 0) {
//							// if a response (ACK) to this KISS frame shall be sent
//
//							// wait for any pending transmission to complete
//							srl_wait_for_tx_completion (main_kiss_srl_ctx_ptr);
//
//							srl_send_data (main_kiss_srl_ctx_ptr, main_small_buffer, SRL_MODE_DEFLN, ln,
//										   SRL_INTERNAL);
//						}
//
//						// restart KISS receiving to be ready for next frame
//						srl_receive_data_kiss_protocol(main_kiss_srl_ctx_ptr, 120);
//					}

					// if there were an error during receiving frame from host, restart rxing once again
					if (main_kiss_srl_ctx_ptr->srl_rx_state == SRL_RX_ERROR && main_kiss_enabled == 1) {
						srl_receive_data_kiss_protocol (main_kiss_srl_ctx_ptr, 120);
					}
				}

				if (kiss_current_async_message != 0xFF && main_kiss_srl_ctx_ptr->srl_tx_state == SRL_TX_IDLE) {
					srl_start_tx(main_kiss_srl_ctx_ptr, kiss_async_pooler(main_kiss_srl_ctx_ptr->srl_tx_buf_pointer, main_kiss_srl_ctx_ptr->srl_tx_buf_ln));
				}

				// if Davis wx station is enabled and it is alive
				if (main_get_main_davis_serial_enabled() == 1) {

					// pool the Davis wx station driver for LOOP packet
					davis_loop_packet_pooler(&rte_wx_davis_loop_packet_avaliable);

					davis_rxcheck_packet_pooler();
				}

				if (main_config_data_mode->wx_umb == 1) {
					// if some UMB data have been received
					if (main_wx_srl_ctx_ptr->srl_rx_state == SRL_RX_DONE) {
						umb_pooling_handler(&rte_wx_umb_context, REASON_RECEIVE_IDLE, master_time, main_config_data_umb);
					}

					// if there were an error during receiving frame from host, restart rxing once again
					if (main_wx_srl_ctx_ptr->srl_rx_state == SRL_RX_ERROR) {
						umb_pooling_handler(&rte_wx_umb_context, REASON_RECEIVE_ERROR, master_time, main_config_data_umb);
					}

					if (main_wx_srl_ctx_ptr->srl_tx_state == SRL_TX_IDLE) {
						umb_pooling_handler(&rte_wx_umb_context, REASON_TRANSMIT_IDLE, master_time, main_config_data_umb);
					}
				}
				// if modbus rtu master is enabled
				else if (main_get_modbus_rtu_master_enabled() == 1 && io_get_cntrl_vbat_m() == 1) {

					if (rte_main_reset_modbus_rtu == 1) {
						rte_main_reset_modbus_rtu = 0;

	//				  rtu_serial_init(&rte_rtu_pool_queue, 1, main_wx_srl_ctx_ptr, main_config_data_rtu);
	//
	//				  main_target_wx_baudrate = main_config_data_rtu->slave_speed;
	//
	//				  srl_init(main_wx_srl_ctx_ptr, USART2, srl_usart2_rx_buffer, RX_BUFFER_2_LN, srl_usart2_tx_buffer, TX_BUFFER_2_LN, main_target_wx_baudrate, main_config_data_rtu->slave_stop_bits);
	//				  srl_switch_tx_delay(main_wx_srl_ctx_ptr, 1);
	//
	//				  rtu_serial_start();
					}
					else {
						rtu_serial_pool();
					}
				}

				button_check_all(main_get_button_one_left(), main_get_button_two_right());

				backup_reg_set_monitor(2);

				// get all meteo measuremenets each 65 seconds. some values may not be
				// downloaded from sensors if _METEO and/or _DALLAS_AS_TELEM aren't defined
				if (main_wx_sensors_pool_timer < 10) {
					if ((main_config_data_mode->wx & WX_ENABLED) == 1 && (io_get_cntrl_vbat_s() == 1)) {

						// notice: UMB-master and Modbus-RTU uses the same UART
						// so they cannot be enabled both at once

						// check if modbus rtu master is enabled and configured properly
						if (main_get_modbus_rtu_master_enabled() == 1) {
							// start quering all Modbus RTU devices & registers one after another
							rtu_serial_start();
						}
						else if (main_config_data_mode->wx_umb == 1) {
							// request status from the slave if UMB master is enabled
							umb_0x26_status_request(&rte_wx_umb, &rte_wx_umb_context, main_config_data_umb);
						}
						else {
							;
						}

						// davis serial weather station is connected using UART / RS232 used normally
						// for KISS communcation between modem and host PC
						if (main_get_main_davis_serial_enabled() == 1) {
							davis_trigger_rxcheck_packet();
						}

						// get all measurements from 'internal' sensors (except wind which is handled separately)
						wx_get_all_measurements(main_config_data_wx_sources, main_config_data_mode, main_config_data_umb, main_config_data_rtu);
					}

					backup_reg_set_monitor(3);

					main_wx_sensors_pool_timer = 65500;
				}

				/**
				 * ONE MINUTE POOLING
				 */
				if (main_one_minute_pool_timer < 10) {

					backup_reg_set_monitor(4);

					//main_nvm_timestamp = main_get_nvm_timestamp();

	#ifdef SX1262_IMPLEMENTATION
	//				fanet_success_cnt = 0;
	//				fanet_fail_cnt = 0;
	//				fanet_tx_success_cnt = 0;
	#endif

					#ifndef _MUTE_OWN
					packet_tx_handler(main_config_data_basic, main_config_data_mode);
					#endif

					backup_reg_set_monitor(5);

					#ifdef STM32L471xx
					if (main_config_data_mode->gsm == 1 && (io_get_cntrl_vbat_g() == 1)) {

						if (http_client_connection_errors > HTTP_CLIENT_MAX_CONNECTION_ERRORS) {
							NVIC_SystemReset();
						}

					}

					// send event log each 24 hours, but only once at the top of an hour
					if(main_get_rtc_datetime(MAIN_GET_RTC_HOUR) == 21) {
						if (backup_reg_get_event_log_report_sent_radio() == 0) {
							// set status bit in non-volatile backup register not to loop over and over again in case of a restart
							backup_reg_set_event_log_report_sent_radio();

							// extract events from NVM
							const nvm_event_result_stats_t events_stat = nvm_event_get_last_events_in_exposed(rte_main_exposed_events, MAIN_HOW_MANY_EVENTS_SEND_REPORT, EVENT_WARNING);

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
						backup_reg_reset_event_log_report_sent_radio();
					}

					if ((main_config_data_gsm->aprsis_enable != 0) &&
						(main_config_data_mode->gsm == 1) &&
						gsm_comm_state_get_current () == GSM_COMM_APRSIS) {

						// send event log each 24 hours, but only once at the top of an hour
						if (main_get_rtc_datetime (MAIN_GET_RTC_HOUR) == 18) {
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
							}
						}
						else {
							// reset flag if the time is not 18:xx
							backup_reg_reset_event_log_report_sent_aprsis ();
						}
					}

					if ((main_config_data_gsm->aprsis_enable != 0) &&
						(main_config_data_mode->gsm == 1) &&
						(pwr_save_is_currently_cutoff () == 0) &&
						(io_get_cntrl_vbat_g() == 1))
					{
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
					if (configuration_get_validate_parameters() == 1) {
						if (rte_wx_check_weather_measurements() == 0) {
							backup_reg_increment_weather_measurements_check_reset();

							NVIC_SystemReset();
						}

						if (rte_wx_dallas_degraded_counter > DALLAS_MAX_LIMIT_OF_DEGRADED) {
							backup_reg_increment_dallas_degraded_reset();

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
						if (system_is_rtc_ok() == 0) {

							backup_reg_increment_is_rtc_ok_check_reset();

							rte_main_reboot_req = 1;
						}

						if ((main_config_data_gsm->aprsis_enable != 0) && (main_config_data_mode->gsm == 1)) {
							xEventGroupSetBits (main_eventgroup_handle_aprs_trigger,
									MAIN_EVENTGROUP_APRSIS_TRIG_APRSIS_COUNTERS);
							//rte_main_trigger_gsm_aprsis_counters_packet = 1;
						}
		#endif
					}	// end of one hour

					/**
					 * SIX HOUR POOLING
					 */
					if (--main_six_hour_pool_timer < 0) {
						main_six_hour_pool_timer = 6;

					  event_log_sync(
								  EVENT_INFO_CYCLIC,
								  EVENT_SRC_MAIN,
								  EVENTS_MAIN_CYCLIC,
								  aprsis_get_successfull_conn_counter(),
								  aprsis_get_unsucessfull_conn_counter(),
								  aprsis_get_tx_counter(),
								  rte_main_average_battery_voltage,
								  rte_main_rx_total, rte_main_tx_total);

					}



					main_one_minute_pool_timer = 60000;
				} // end of one minute

				/**
				 * ONE SECOND POOLING
				 */
				if (main_one_second_pool_timer < 10) {
//
//					backup_reg_set_monitor(6);
//
//					digi_pool_viscous();
//
//					button_debounce();
//
//					#ifdef SX1262_IMPLEMENTATION
//					supervisor_iam_alive(SUPERVISOR_THREAD_MAIN_LOOP);
//					supervisor_iam_alive(SUPERVISOR_THREAD_SEND_WX);
//
//
//					retval = fanet_test();
//
//					if (retval != 0)
//					{
//						  event_log_sync(
//									  EVENT_INFO_CYCLIC,
//									  EVENT_SRC_MAIN,
//									  EVENTS_MAIN_CYCLIC,
//									  0, 0,
//									  0, 0,
//									  0xDDCCBBAA, retval);
//					}
//					#endif
//
//					#ifdef PARAMETEO
//					if (rte_main_reboot_scheduled_diag == RTE_MAIN_REBOOT_SCHEDULED_APRSMSG) {
//						if (gsm_sim800_tcpip_tx_busy() == 0) {
//							rte_main_reboot_scheduled_diag = 0;
//
//							NVIC_SystemReset();
//						}
//					}
//
//					// this if cannot be guarded by checking if VBAT_G is enabled
//					// because VBAT_G itself is controlled by initialization
//					// pooler
//					if (main_config_data_mode->gsm == 1) {
//						gsm_sim800_initialization_pool(main_gsm_srl_ctx_ptr, &main_gsm_state);
//					}
//
//					if ((main_config_data_mode->gsm == 1)  && (io_get_cntrl_vbat_g() == 1) && (rte_main_woken_up == 0)) {
//
//						// check if GSM modem must be power-cycled / restarted like after
//						// waking up from deep sleep or chaning power saving mode
//						if (rte_main_reset_gsm_modem == 1) {
//							// rest the flag
//							rte_main_reset_gsm_modem = 0;
//
//							srl_init(main_gsm_srl_ctx_ptr, USART3, srl_usart3_rx_buffer, RX_BUFFER_3_LN, srl_usart3_tx_buffer, TX_BUFFER_3_LN, 115200, 1);
//
//							// reset gsm modem
//							gsm_sim800_reset(&main_gsm_state);
//
//							// please remember that a reset might not be performed if
//							// the GSM modem is inhibited completely, due to current
//							// power saving mode and few another things. In that case
//							// the flag will be cleared but modem NOT restarted
//						}
//
//						if (aprsis_get_aprsis_logged() == 1) {
//							led_control_led1_upper(true);
//						}
//						else {
//							led_control_led1_upper(false);
//						}
//
//						if (gsm_sim800_gprs_ready == 1) {
//							/***
//							 *
//							 * TEST TEST TEST TODO
//							 */
//							//retval = http_client_async_get("http://pogoda.cc:8080/meteo_backend/status", strlen("http://pogoda.cc:8080/meteo_backend/status"), 0xFFF0, 0x1, dupa);
//							//retval = http_client_async_post("http://pogoda.cc:8080/meteo_backend/parameteo/skrzyczne/status", strlen("http://pogoda.cc:8080/meteo_backend/parameteo/skrzyczne/status"), post_content, strlen(post_content), 0, dupa);
//						}
//
//						gsm_sim800_poolers_one_second(main_gsm_srl_ctx_ptr, &main_gsm_state, main_config_data_gsm);
//
//						if (gsm_comm_state_get_current() == GSM_COMM_APRSIS) {
//							aprsis_check_alive();
//						}
//					}
//					#endif
//
//					if ((io_get_cntrl_vbat_s() == 1) && (main_config_data_mode->wx & WX_ENABLED) == 1) {
//						analog_anemometer_direction_handler();
//					}
//
//					backup_reg_set_monitor(7);
//
//					main_one_second_pool_timer = 1000;
				}	// end of one second pooler
				else if (main_one_second_pool_timer < -10) {
//
//					if ((main_config_data_mode->wx & WX_ENABLED) == 1) {
//						analog_anemometer_direction_reset();
//					}
//
//					main_one_second_pool_timer = 1000;
				}

				/**
				 * TWO SECOND POOLING
				 */
				if (main_two_second_pool_timer < 10) {

//					if (main_config_data_mode->wx != 0) {
//						// TODO:
//						if (configuration_get_inhibit_wx_pwr_handle() == 0) {
//							wx_pwr_switch_periodic_handle();
//						}
//
//						wx_check_force_i2c_reset();
//					}
//
//		#ifdef PARAMETEO
//					if (main_current_powersave_mode != PWSAVE_AGGRESV) {
//						if (configuration_get_power_cycle_vbat_r() == 1 && !main_afsk.sending) {
//							io_pool_vbat_r();
//						}
//					}
//					else {
//						io_inhibit_pool_vbat_r();
//					}
//		#endif
//
//		#ifdef PARAMETEO
//
//					if (io_get_cntrl_vbat_s() == 1) {
//						max31865_pool();
//					}
//
//					if (io_get_cntrl_vbat_g () == 1) {
//						if (main_config_data_mode->gsm == 1 && io_get_cntrl_vbat_g () == 1 &&
//							rte_main_woken_up == 0) {
//								gsm_comm_state_handler (gsm_sim800_engineering_get_is_done(), ntp_done, rte_main_events_extracted_for_api_stat.zz_total, gsm_sim800_gprs_ready);
//						}
//
//						// if GSM module is enabled and GPRS communication state is now on API phase
//						if (	(main_config_data_mode->gsm == 1) &&
//								(gsm_comm_state_get_current () == GSM_COMM_API)) {
//
//							// if there are any events remaining to push to API
//							if (rte_main_events_extracted_for_api_stat.zz_total > 0) {
//								// send current event
//								const uint8_t api_connection_result = api_send_json_event(&rte_main_exposed_events[rte_main_events_extracted_for_api_stat.zz_total - 1]);
//
//								// if TCP connection is established and data is currently sent asynchronously
//								if (api_connection_result == HTTP_CLIENT_OK) {
//									// end decrement remaining number of events
//									rte_main_events_extracted_for_api_stat.zz_total--;
//								}
//								else {
//									// for sake of simplicity break on first connection error
//									rte_main_events_extracted_for_api_stat.zz_total = 0;
//								}
//							}
//						}
//
//						if (gsm_comm_state_get_current() == GSM_COMM_NTP) {
//							ntp_get_sync();
//						}
//					}
//		#endif
//					main_reload_internal_wdg();
//
//					main_two_second_pool_timer = 2000;
				}	// end of two second pooling

				/**
				 * FOUR SECOND POOLING
				 */
				if (main_four_second_pool_timer < 10) {
					main_four_second_pool_timer = 4000;

		#ifdef PARAMETEO
					if (rte_main_trigger_radio_event_log > 0 && io_get_cntrl_vbat_r() == 1) {

						// set a pointer to even in exposed form which will be sent now
						const event_log_exposed_t * current_exposed_event = &rte_main_exposed_events[rte_main_trigger_radio_event_log - 1];

						status_send_from_exposed_eveny_log(current_exposed_event);

						rte_main_trigger_radio_event_log--;
					}
		#endif
				}	// end of four second pooling

				/**
				 * TEN SECOND POOLING
				 */
				if (main_ten_second_pool_timer < 10) {

//					// get current battery voltage. for non parameteo this will return 0
//					//main_battery_measurement_res = io_vbat_meas_get(&rte_main_battery_voltage);
//					rte_main_battery_voltage = io_vbat_meas_get_synchro_old();
//					rte_main_average_battery_voltage = io_vbat_meas_average(rte_main_battery_voltage);
//
//
//					// meas average will return 0 if internal buffer isn't filled completely
//					if (rte_main_average_battery_voltage == 0) {
//						rte_main_average_battery_voltage = rte_main_battery_voltage;
//					}
//
//					backup_reg_set_monitor(8);
//
//					// check if consecutive weather frame has been triggered from 'packet_tx_handler'
//					if (rte_main_trigger_wx_packet == 1 && io_get_cntrl_vbat_r() == 1) {
//
//						packet_tx_send_wx_frame();
//
//						rte_main_trigger_wx_packet = 0;
//					}
//
//		#ifdef PARAMETEO
//
//					if (main_check_adc == 1) {
//						AD_Restart();
//
//						main_check_adc = 0;
//					}
//
//					// inhibit any power save switching when modem transmits data
//					if (!main_afsk.sending && rte_main_woken_up == 0 && packet_tx_is_gsm_meteo_pending() == 0) {
//						rte_main_curret_powersave_mode =
//								pwr_save_pooling_handler(
//										main_config_data_mode,
//										packet_tx_meteo_interval,
//										packet_tx_get_minutes_to_next_wx(),
//										rte_main_average_battery_voltage,
//										rte_main_battery_voltage,
//										&main_continue_loop);
//					}
//
//					if (main_continue_loop == 0) {
//						continue;
//					}
//		#endif
//
//					backup_reg_set_monitor(9);
//
//		#ifdef PARAMETEO
//					if (main_config_data_mode->gsm == 1 && io_get_cntrl_vbat_g () == 1 &&
//						rte_main_woken_up == 0) {
//
//						gsm_sim800_poolers_ten_seconds (main_gsm_srl_ctx_ptr, &main_gsm_state);
//
//						if (gsm_comm_state_get_current() == GSM_COMM_APRSIS) {
//							packet_tx_tcp_handler ();
//						}
//					}
//		#endif
//
//					if (main_config_data_mode->wx_umb == 1) {
//						umb_channel_pool(&rte_wx_umb, &rte_wx_umb_context, main_config_data_umb);
//					}
//
//					if (main_config_data_mode->wx_umb == 1) {
//						rte_wx_umb_qf = umb_get_current_qf(&rte_wx_umb_context, master_time);
//					}
//
//					if (main_config_data_mode->wx_umb == 1) {
//						const uint8_t umb_watchdog_state = umb_master_watchdog(&rte_wx_umb_context, master_time);
//
//						if (umb_watchdog_state == 1) {
//						  const uint32_t wx_baudrate = main_config_data_umb->serial_speed;
//
//						  srl_close(main_wx_srl_ctx_ptr);
//						  srl_init(main_wx_srl_ctx_ptr, USART2, srl_usart2_rx_buffer, RX_BUFFER_2_LN, srl_usart2_tx_buffer, TX_BUFFER_2_LN, wx_baudrate, 1);
//						  umb_master_init(&rte_wx_umb_context, main_wx_srl_ctx_ptr, main_config_data_umb);
//						}
//						else if (umb_watchdog_state > 1) {
//							rte_main_reboot_req = 1;
//						}
//						else {
//							; // everything is ok
//						}
//					}
//
//					if (main_config_data_mode->wx != 0) {
//
//						#ifdef STM32L471xx
//							if (io_get_cntrl_vbat_s() == 1) {
//						#else
//							if (io_get_5v_isol_sw___cntrl_vbat_s() == 1) {
//						#endif
//								// pool anemometer only when power is applied  /// RESET
//								wx_pool_anemometer(main_config_data_wx_sources, main_config_data_mode, main_config_data_umb, main_config_data_rtu);
//							}
//					}
//
//					if (main_get_main_davis_serial_enabled() == 1) {
//
//						// if previous LOOP packet is ready for processing
//						if (rte_wx_davis_loop_packet_avaliable == 1) {
//							davis_parsers_loop(main_kiss_srl_ctx_ptr->srl_rx_buf_pointer, main_kiss_srl_ctx_ptr->srl_rx_buf_ln, &rte_wx_davis_loop_content);
//						}
//
//						// trigger consecutive LOOP packet
//						davis_trigger_loop_packet();
//					}

					main_ten_second_pool_timer = 10000;
				} 	// end of ten second pooling

			  backup_reg_set_monitor(10);

	#if defined(PARAMETEO)
			}	// else under if (rte_main_woken_up == RTE_MAIN_WOKEN_UP_EXITED)
	#endif
	    } // Infinite loop, never return.


}


