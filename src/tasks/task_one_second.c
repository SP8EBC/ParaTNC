/*
 * task_one_second.c
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

#include "LedConfig.h"
#include "aprsis.h"
#include "backup_registers.h"
#include "button.h"
#include "digi.h"
#include "fanet_app.h"
#include "gsm_comm_state_handler.h"
#include "io.h"
#include "supervisor.h"

#include "drivers/analog_anemometer.h"
#include "drivers/serial.h"
#include "etc/serial_config.h"
#include "gsm/sim800c.h"
#include "gsm/sim800c_gprs.h"
#include "gsm/sim800c_poolers.h"
#include "gsm/sim800c_tcpip.h"

#include "event_log.h"
#include "events_definitions/events_main.h"

void task_one_second (void *parameters)
{
	(void)(parameters);

	/* Block for 1000ms. */
	const TickType_t xDelay = 1000 / portTICK_PERIOD_MS;

	while (1) {
		SUPERVISOR_MONITOR_CLEAR (TASK_ONE_SEC);

		vTaskDelay (xDelay);

		SUPERVISOR_MONITOR_SET_CHECKPOINT (TASK_ONE_SEC, 1);

		xEventGroupClearBits (main_eventgroup_handle_powersave, MAIN_EVENTGROUP_PWRSAVE_ONE_SEC);

		if (digi_is_enabled () != 0) {
			digi_pool_viscous ();
		}

		SUPERVISOR_MONITOR_SET_CHECKPOINT (TASK_ONE_SEC, 2);

		button_debounce ();

		SUPERVISOR_MONITOR_SET_CHECKPOINT (TASK_ONE_SEC, 3);

		if (rte_main_reboot_scheduled_diag == RTE_MAIN_REBOOT_SCHEDULED_APRSMSG) {
			if (gsm_sim800_tcpip_tx_busy () == 0) {
				rte_main_reboot_scheduled_diag = 0;

				NVIC_SystemReset ();
			}
		}

		// this if cannot be guarded by checking if VBAT_G is enabled
		// because VBAT_G itself is controlled by initialization
		// pooler
		if (main_config_data_mode->gsm == 1) {
			SUPERVISOR_MONITOR_SET_CHECKPOINT (TASK_ONE_SEC, 4);

			gsm_sim800_initialization_pool (main_gsm_srl_ctx_ptr, &main_gsm_state);

			SUPERVISOR_MONITOR_SET_CHECKPOINT (TASK_ONE_SEC, 5);
		}

		if ((main_config_data_mode->gsm == 1) && (io_get_cntrl_vbat_g () == 1) &&
			(rte_main_woken_up == 0)) {
			SUPERVISOR_MONITOR_SET_CHECKPOINT (TASK_ONE_SEC, 6);

			// check if GSM modem must be power-cycled / restarted like after
			// waking up from deep sleep or chaning power saving mode
			if (rte_main_reset_gsm_modem == 1) {
				SUPERVISOR_MONITOR_SET_CHECKPOINT (TASK_ONE_SEC, 7);

				// rest the flag
				rte_main_reset_gsm_modem = 0;

				srl_init (main_gsm_srl_ctx_ptr,
						  USART3,
						  srl_usart3_rx_buffer,
						  RX_BUFFER_3_LN,
						  srl_usart3_tx_buffer,
						  TX_BUFFER_3_LN,
						  115200,
						  1);

				// reset gsm modem
				gsm_sim800_reset (&main_gsm_state);

				// please remember that a reset might not be performed if
				// the GSM modem is inhibited completely, due to current
				// power saving mode and few another things. In that case
				// the flag will be cleared but modem NOT restarted
			}

			if (aprsis_get_aprsis_logged () == 1) {
				led_control_led1_upper (true);
			}
			else {
				led_control_led1_upper (false);
			}

			SUPERVISOR_MONITOR_SET_CHECKPOINT (TASK_ONE_SEC, 8);

			gsm_sim800_poolers_one_second (main_gsm_srl_ctx_ptr,
										   &main_gsm_state,
										   main_config_data_gsm);

			SUPERVISOR_MONITOR_SET_CHECKPOINT (TASK_ONE_SEC, 9);

			if (gsm_comm_state_get_current () == GSM_COMM_APRSIS) {
				aprsis_check_alive ();
			}
		}

		if ((io_get_cntrl_vbat_s () == 1) && (main_config_data_mode->wx & WX_ENABLED) == 1) {
			SUPERVISOR_MONITOR_SET_CHECKPOINT (TASK_ONE_SEC, 10);

			analog_anemometer_direction_handler ();
		}

		supervisor_iam_alive (SUPERVISOR_THREAD_TASK_ONE_SEC);

		xEventGroupSetBits (main_eventgroup_handle_powersave, MAIN_EVENTGROUP_PWRSAVE_ONE_SEC);
	} // while(1)
	// end of while loop
}
