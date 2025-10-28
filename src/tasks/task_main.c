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

#include "supervisor.h"
#include "backup_registers.h"

#include "pwr_save.h"
#include "adc.h"
#include "aprsis.h"
#include "digi.h"
#include "TimerConfig.h"
#include "LedConfig.h"
#include "button.h"
#include "wx_handler.h"
#include "wx_pwr_switch.h"
#include "ntp.h"
#include "io.h"

#if defined(PARAMETEO)
#include <stm32l4xx.h>
#include <system_stm32l4xx.h>
#include "gsm/sim800c_tcpip.h"
#include "gsm/sim800c_gprs.h"
#include "gsm/sim800c_poolers.h"
#include "gsm/sim800c_engineering.h"
#endif

#include <FreeRTOS.h>
#include <task.h>

//static uint8_t main_continue_loop = 0;

//!< Triggers additional check if ADC has properly reinitialized and conversion is working
uint8_t main_check_adc = 0;

//!< Used to store an information which telemetry descritpion frame should be sent next
//static telemetry_description_t main_telemetry_description = TELEMETRY_NOTHING;



static void message_callback(struct AX25Msg *msg) {
	(void)msg;
}

void task_main( void * parameters )
{

	(void)(parameters);

	//int32_t ln = 0;
	//srl_context_t*  kiss_srl_ctx = main_get_kiss_srl_ctx_ptr();

	/* Block for 200ms. */
	const TickType_t xDelay = 200 / portTICK_PERIOD_MS;

	  while (1)
	    {
	    vTaskDelay( xDelay );

		backup_reg_set_monitor(-1);

		// system reset may be requested from various places in the application
		if (rte_main_reboot_req == 1) {
		    vTaskDelay( xDelay );

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

//					// if serial port is currently not busy on transmission
//					if (main_kiss_srl_ctx_ptr->srl_tx_state != SRL_TXING) {
//						memset(kiss_srl_ctx->srl_tx_buf_pointer, 0x00, kiss_srl_ctx->srl_tx_buf_ln);
//
//						if (main_kiss_enabled == 1) {
//							// convert message to kiss format and send it to host
//							srl_start_tx(main_kiss_srl_ctx_ptr, kiss_send_ax25_to_host(ax25_rxed_frame.raw_data, (ax25_rxed_frame.raw_msg_len - 2), main_kiss_srl_ctx_ptr->srl_tx_buf_pointer, main_kiss_srl_ctx_ptr->srl_tx_buf_ln));
//						}
//					}
//
//					main_ax25.dcd = false;
//
//					// check this frame against other frame in visvous buffer waiting to be transmitted
//					digi_check_with_viscous(&ax25_rxed_frame);
//
//					// check if this packet needs to be repeated (digipeated) and do it if it is necessary
//					digi_process(&ax25_rxed_frame, main_config_data_basic, main_config_data_mode);
//
//					ax25_new_msg_rx_flag = 0;
//					rx10m++;
//		#ifdef PARAMETEO
//					rte_main_rx_total++;
//
//					// if aprsis is logged
//					if (aprsis_connected == 1 && gsm_sim800_tcpip_tx_busy() == 0) {
//						aprsis_igate_to_aprsis(&ax25_rxed_frame, (const char *)&main_callsign_with_ssid);
//					}
//
//		#endif
				}

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
				else {

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

				} // end of one minute

				/**
				 * ONE SECOND POOLING
				 */
				if (main_one_second_pool_timer < 10) {

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

					main_ten_second_pool_timer = 10000;
				} 	// end of ten second pooling

			  backup_reg_set_monitor(10);

	#if defined(PARAMETEO)
			}	// else under if (rte_main_woken_up == RTE_MAIN_WOKEN_UP_EXITED)
	#endif
			supervisor_iam_alive(SUPERVISOR_THREAD_MAIN_LOOP);

	    } // Infinite loop, never return.


}


