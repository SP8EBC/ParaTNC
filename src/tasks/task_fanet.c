/*
 * task_fanet.c
 *
 *  Created on: Nov 1, 2025
 *      Author: mateusz
 */

#include "main.h"
#include "main_getters_for_task.h"
#include "main_freertos_externs.h"
#include "main_gsm_pool_handler.h"
#include "rte_main.h"
#include "rte_wx.h"

#include <FreeRTOS.h>
#include <task.h>

void task_fanet( void * parameters )
{
	while (1) {
		// wait infinite amount of time for event from a serial port indicating that
		(void)xEventGroupWaitBits (main_eventgroup_handle_radio_message,
								   MAIN_EVENTGROUP_RADIO_MESSAGE_RXED,
								   pdTRUE,
								   pdTRUE,
								   0xFFFFFFFFu);

	}
}


