/*
 * gsm_tcpip_config.h
 *
 *  Created on: Sep 14, 2025
 *      Author: mateusz
 */

#ifndef ETC_GSM_TCPIP_CONFIG_H_
#define ETC_GSM_TCPIP_CONFIG_H_

extern void main_handle_mutex_gsm_tcpip(uint8_t what_to_do);

//#define GSM_TCPIP_IS_TX_BUSY_CALLBACK		main_handle_mutex_gsm_tcpip(1);
//#define GSM_TCPIP_TX_DONE_CALLBACK			main_handle_mutex_gsm_tcpip(2);

/**
 * Enabling this macro will cause 'gsm_sim800_tcpip_async_receive' and 'gsm_sim800_tcpip_write'
 * to use FreeRTOS events to block and wait for transmission (or reception) to finish
 */
#define GSM_TCPIP_ASYNC_RTOS_BLOCKING

#endif /* ETC_GSM_TCPIP_CONFIG_H_ */
