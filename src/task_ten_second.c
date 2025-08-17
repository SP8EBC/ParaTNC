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

#include "main.h"
#include "rte_main.h"
#include "rte_wx.h"

#include "gsm_comm_state_handler.h"
#include "io.h"
#include "packet_tx_handler.h"
#include "wx_handler.h"

#include "gsm/sim800c_poolers.h"
#include "davis_vantage/davis_parsers.h"
#include "davis_vantage/davis.h"
#include "umb_master/umb_channel_pool.h"
#include "drivers/serial.h"

void task_ten_second( void * parameters )
{
	(void) parameters;
	/* Block for 10000ms. */
	const TickType_t xDelay = 10000 / portTICK_PERIOD_MS;

	while(1) {
		vTaskDelay (xDelay);

		// check if consecutive weather frame has been triggered from 'packet_tx_handler'
		if (rte_main_trigger_wx_packet == 1 && io_get_cntrl_vbat_r() == 1) {

			packet_tx_send_wx_frame();

			rte_main_trigger_wx_packet = 0;
		}

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

}


