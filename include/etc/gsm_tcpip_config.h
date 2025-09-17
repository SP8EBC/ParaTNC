/*
 * gsm_tcpip_config.h
 *
 *  Created on: Sep 14, 2025
 *      Author: mateusz
 */

#ifndef ETC_GSM_TCPIP_CONFIG_H_
#define ETC_GSM_TCPIP_CONFIG_H_

extern void main_handle_mutex_gsm_tcpip (uint8_t what_to_do);

// #define GSM_TCPIP_IS_TX_BUSY_CALLBACK		main_handle_mutex_gsm_tcpip(1);
// #define GSM_TCPIP_TX_DONE_CALLBACK			main_handle_mutex_gsm_tcpip(2);

/**
 * Enabling this macro will cause 'gsm_sim800_tcpip_receive' and 'gsm_sim800_tcpip_write'
 * to use FreeRTOS events to block and wait for transmission (or reception) to finish
 */
#define GSM_TCPIP_RTOS_BLOCKING

#ifdef GSM_TCPIP_RTOS_BLOCKING
#include "main_freertos_externs.h"

#define GSM_TCPIP_RTOS_BLOCKING_TAKE_MUTEX 1
#define GSM_TCPIP_RTOS_BLOCKING_GIVE_MUTEX 2

#define GSM_TCPIP_RTOS_BLOCKING_MUTEX_CALL_TO_TAKE \
	main_handle_mutex_gsm_tcpip (GSM_TCPIP_RTOS_BLOCKING_TAKE_MUTEX);
#define GSM_TCPIP_RTOS_BLOCKING_MUTEX_CALL_TO_GIVE \
	main_handle_mutex_gsm_tcpip (GSM_TCPIP_RTOS_BLOCKING_GIVE_MUTEX);

#define GSM_TCPIP_RTOS_BLOCKING_EVENT			 main_eventgroup_handle_serial_gsm
#define GSM_TCPIP_RTOS_BLOCKING_EVENT_BITMASK_TX MAIN_EVENTGROUP_SERIAL_GSM_TX_DONE
#define GSM_TCPIP_RTOS_BLOCKING_EVENT_BITMASK_RX MAIN_EVENTGROUP_SERIAL_GSM_RX_DONE

#else
#define GSM_TCPIP_RTOS_BLOCKING_MUTEX_CALL_TO_TAKE
#define GSM_TCPIP_RTOS_BLOCKING_MUTEX_CALL_TO_GIVE
#endif

#endif /* ETC_GSM_TCPIP_CONFIG_H_ */
