/*
 * task_ten_second.c
 *
 *  Created on: Aug 5, 2025
 *      Author: mateusz
 */


#include <FreeRTOS.h>
#include <stdbool.h>
#include <stdint.h>
#include <task.h>


#include "drivers/serial.h"

void task_ten_second( void * parameters )
{
	// get current battery voltage. for non parameteo this will return 0
	//main_battery_measurement_res = io_vbat_meas_get(&rte_main_battery_voltage);
	rte_main_battery_voltage = io_vbat_meas_get_synchro_old();
	rte_main_average_battery_voltage = io_vbat_meas_average(rte_main_battery_voltage);


	// meas average will return 0 if internal buffer isn't filled completely
	if (rte_main_average_battery_voltage == 0) {
		rte_main_average_battery_voltage = rte_main_battery_voltage;
	}

	backup_reg_set_monitor(8);

	// check if consecutive weather frame has been triggered from 'packet_tx_handler'
	if (rte_main_trigger_wx_packet == 1 && io_get_cntrl_vbat_r() == 1) {

		packet_tx_send_wx_frame();

		rte_main_trigger_wx_packet = 0;
	}

#ifdef PARAMETEO

	if (rte_main_check_adc == 1) {
		AD_Restart();

		rte_main_check_adc = 0;
	}

	// inhibit any power save switching when modem transmits data
	if (!main_afsk.sending && rte_main_woken_up == 0 && packet_tx_is_gsm_meteo_pending() == 0) {
		rte_main_curret_powersave_mode =
				pwr_save_pooling_handler(
						main_config_data_mode,
						packet_tx_meteo_interval,
						packet_tx_get_minutes_to_next_wx(),
						rte_main_average_battery_voltage,
						rte_main_battery_voltage,
						&main_continue_loop);
	}

	// this is set to zero after a call to "pwr_save_switch_mode_to_l7"
	if (main_continue_loop == 0) {
		continue;
	}
#endif

	backup_reg_set_monitor(9);

#ifdef PARAMETEO
	if (main_config_data_mode->gsm == 1 && io_get_cntrl_vbat_g () == 1 &&
		rte_main_woken_up == 0) {

		gsm_sim800_poolers_ten_seconds (main_gsm_srl_ctx_ptr, &main_gsm_state);

		if (gsm_comm_state_get_current() == GSM_COMM_APRSIS) {
			packet_tx_tcp_handler ();
		}
	}
#endif

	if (main_config_data_mode->wx_umb == 1) {
		umb_channel_pool(&rte_wx_umb, &rte_wx_umb_context, main_config_data_umb);
	}

	if (main_config_data_mode->wx_umb == 1) {
		rte_wx_umb_qf = umb_get_current_qf(&rte_wx_umb_context, master_time);
	}

	if (main_config_data_mode->wx_umb == 1) {
		const uint8_t umb_watchdog_state = umb_master_watchdog(&rte_wx_umb_context, master_time);

		if (umb_watchdog_state == 1) {
		  const uint32_t wx_baudrate = main_config_data_umb->serial_speed;

		  srl_close(main_wx_srl_ctx_ptr);
		  srl_init(main_wx_srl_ctx_ptr, USART2, srl_usart2_rx_buffer, RX_BUFFER_2_LN, srl_usart2_tx_buffer, TX_BUFFER_2_LN, wx_baudrate, 1);
		  umb_master_init(&rte_wx_umb_context, main_wx_srl_ctx_ptr, main_config_data_umb);
		}
		else if (umb_watchdog_state > 1) {
			rte_main_reboot_req = 1;
		}
		else {
			; // everything is ok
		}
	}

	if (main_config_data_mode->wx != 0) {

		#ifdef STM32L471xx
			if (io_get_cntrl_vbat_s() == 1) {
		#else
			if (io_get_5v_isol_sw___cntrl_vbat_s() == 1) {
		#endif
				// pool anemometer only when power is applied  /// RESET
				wx_pool_anemometer(main_config_data_wx_sources, main_config_data_mode, main_config_data_umb, main_config_data_rtu);
			}
	}

	if (main_get_main_davis_serial_enabled() == 1) {

		// if previous LOOP packet is ready for processing
		if (rte_wx_davis_loop_packet_avaliable == 1) {
			davis_parsers_loop(main_kiss_srl_ctx_ptr->srl_rx_buf_pointer, main_kiss_srl_ctx_ptr->srl_rx_buf_ln, &rte_wx_davis_loop_content);
		}

		// trigger consecutive LOOP packet
		davis_trigger_loop_packet();
	}

}


