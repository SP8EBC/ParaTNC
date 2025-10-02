/*
 * task_one_second.c
 *
 *  Created on: Aug 5, 2025
 *      Author: mateusz
 */

#include <FreeRTOS.h>
#include <stdbool.h>
#include <stdint.h>
#include <task.h>
#include <event_groups.h>

#include "main.h"
#include "main_freertos_externs.h"
#include "rte_main.h"

#include "LedConfig.h"
#include "aprsis.h"
#include "backup_registers.h"
#include "button.h"
#include "digi.h"
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

void task_one_second (void *parameters)
{
	(void)(parameters);

	srl_context_t *kiss_srl_ctx = main_get_kiss_srl_ctx_ptr ();

	/* Block for 1000ms. */
	const TickType_t xDelay = 1000 / portTICK_PERIOD_MS;

	while (1) {
		vTaskDelay (xDelay);

		xEventGroupClearBits(main_eventgroup_handle_powersave, MAIN_EVENTGROUP_PWRSAVE_ONE_SEC);

		supervisor_iam_alive(SUPERVISOR_THREAD_TASK_ONE_SEC);

		backup_reg_set_monitor (6);

		digi_pool_viscous ();

		button_debounce ();

#ifdef SX1262_IMPLEMENTATION
		supervisor_iam_alive (SUPERVISOR_THREAD_MAIN_LOOP);
		supervisor_iam_alive (SUPERVISOR_THREAD_SEND_WX);

		retval = fanet_test ();

		if (retval != 0) {
			event_log_sync (EVENT_INFO_CYCLIC, EVENT_SRC_MAIN, EVENTS_MAIN_CYCLIC, 0, 0, 0, 0,
							0xDDCCBBAA, retval);
		}
#endif

#ifdef PARAMETEO
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
			gsm_sim800_initialization_pool (main_gsm_srl_ctx_ptr, &main_gsm_state);
		}

		if ((main_config_data_mode->gsm == 1) && (io_get_cntrl_vbat_g () == 1) &&
			(rte_main_woken_up == 0)) {

			// check if GSM modem must be power-cycled / restarted like after
			// waking up from deep sleep or chaning power saving mode
			if (rte_main_reset_gsm_modem == 1) {
				// rest the flag
				rte_main_reset_gsm_modem = 0;

				srl_init (main_gsm_srl_ctx_ptr, USART3, srl_usart3_rx_buffer, RX_BUFFER_3_LN,
						  srl_usart3_tx_buffer, TX_BUFFER_3_LN, 115200, 1);

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

			if (gsm_sim800_gprs_ready == 1) {
				/***
				 *
				 * TEST TEST TEST TODO
				 */
				// retval = http_client_async_get("http://pogoda.cc:8080/meteo_backend/status",
				// strlen("http://pogoda.cc:8080/meteo_backend/status"), 0xFFF0, 0x1, dupa); retval
				// =
				// http_client_async_post("http://pogoda.cc:8080/meteo_backend/parameteo/skrzyczne/status",
				// strlen("http://pogoda.cc:8080/meteo_backend/parameteo/skrzyczne/status"),
				// post_content, strlen(post_content), 0, dupa);
			}

			gsm_sim800_poolers_one_second (main_gsm_srl_ctx_ptr, &main_gsm_state,
										   main_config_data_gsm);

			if (gsm_comm_state_get_current () == GSM_COMM_APRSIS) {
				aprsis_check_alive ();
			}
		}
#endif

		if ((io_get_cntrl_vbat_s () == 1) && (main_config_data_mode->wx & WX_ENABLED) == 1) {
			analog_anemometer_direction_handler ();
		}

		backup_reg_set_monitor (7);

		xEventGroupSetBits(main_eventgroup_handle_powersave, MAIN_EVENTGROUP_PWRSAVE_ONE_SEC);
	}
	// end of while loop
}
