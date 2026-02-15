/*
 * task_event_serial_modbus_rx_done.c
 *
 *  Created on: Aug 22, 2025
 *      Author: mateusz
 */

#include <FreeRTOS.h>
#include <event_groups.h>
#include <stdbool.h>
#include <stdint.h>
#include <task.h>

#include "main.h"
#include "main_freertos_externs.h"

#include "LedConfig.h"

#include "rte_wx.h"

#include "drivers/serial.h"

void task_event_serial_sensor (void *param)
{
	(void)param;

	// another pointer to serial kiss context. with shorter name for convenience ;)
	srl_context_t *ctx = main_wx_srl_ctx_ptr;

	while (1) {

		// wait infinite amount of time for event from a serial port indicating that
		const EventBits_t bits_on_event = xEventGroupWaitBits (main_eventgroup_handle_serial_sensor,
															   MAIN_EVENTGROUP_SERIAL_WX,
															   pdTRUE,
															   pdFALSE,
															   0xFFFFFFFFu);

		xEventGroupClearBits (main_eventgroup_handle_powersave,
							  MAIN_EVENTGROUP_PWRSAVE_EV_SRL_SENSOR);

		if ((bits_on_event & MAIN_EVENTGROUP_SERIAL_WX_RX_DONE) != 0) {
			if (main_config_data_mode->wx_umb == 1) {
				umb_pooling_handler (&rte_wx_umb_context,
									 REASON_RECEIVE_IDLE,
									 master_time,
									 main_config_data_umb);

				if (rte_wx_umb_context.state == UMB_STATUS_RESPONSE_AVALIABLE) {
					led_blink_led2_botoom ();

					// one more call to move state machine from UMB_STATUS_RESPONSE_AVALIABLE
					umb_pooling_handler (&rte_wx_umb_context,
										 REASON_RECEIVE_IDLE,
										 master_time,
										 main_config_data_umb);
				}
			}
		} // 		if ((bits_on_event & MAIN_EVENTGROUP_SERIAL_WX_RX_DONE) != 0)

		if ((bits_on_event & MAIN_EVENTGROUP_SERIAL_WX_RX_ERROR) != 0) {
			if (main_config_data_mode->wx_umb == 1) {
				umb_pooling_handler (&rte_wx_umb_context,
									 REASON_RECEIVE_ERROR,
									 master_time,
									 main_config_data_umb);
			}
		} // 		if ((bits_on_event & MAIN_EVENTGROUP_SERIAL_WX_RX_ERROR) != 0)

		if ((bits_on_event & MAIN_EVENTGROUP_SERIAL_WX_TX_DONE) != 0) {
			if (main_config_data_mode->wx_umb == 1) {
				umb_pooling_handler (&rte_wx_umb_context,
									 REASON_TRANSMIT_IDLE,
									 master_time,
									 main_config_data_umb);
			}
		} // 		if ((bits_on_event & MAIN_EVENTGROUP_SERIAL_WX_TX_DONE) != 0)

		xEventGroupSetBits (main_eventgroup_handle_powersave,
							MAIN_EVENTGROUP_PWRSAVE_EV_SRL_SENSOR);
	}
}
