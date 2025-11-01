/*
 * tasks_list.h
 *
 *  Created on: Nov 1, 2025
 *      Author: mateusz
 */

#ifndef ETC_TASKS_LIST_H_
#define ETC_TASKS_LIST_H_


#define TASKS_LIST(ENTRY)																			\
		/* 	task_entry_point, 				task_name_string, 	stack_size, 					params, priority, 				output_handle */	\
	ENTRY(	task_main,						"task_main",		configMINIMAL_STACK_SIZE * 4, 	NULL,	tskIDLE_PRIORITY + 1, 	task_main_handle)	\
	ENTRY(	task_power_save,				"task_powersave",	configMINIMAL_STACK_SIZE, 		NULL,	tskIDLE_PRIORITY + 2, 	task_powersave_handle)	\
	ENTRY(	task_event_aprsis_msg_trigger,	"tev_apris_trig",	configMINIMAL_STACK_SIZE, 		NULL,	tskIDLE_PRIORITY + 3, 	task_powersave_handle)	\
	ENTRY(	task_event_api_ntp,				"tev_ntp_api",		configMINIMAL_STACK_SIZE, 		NULL,	tskIDLE_PRIORITY + 3, 	task_ev_ntp_and_api_client)	\
	ENTRY(	task_event_radio_message,		"tev_radio_message",configMINIMAL_STACK_SIZE, 		NULL,	tskIDLE_PRIORITY + 3, 	task_ev_radio_message_handle)	\
	ENTRY(	task_one_second,				"task_one_sec",		configMINIMAL_STACK_SIZE, 		NULL,	tskIDLE_PRIORITY + 4, 	task_one_sec_handle)	\
	ENTRY(	task_two_second,				"task_two_sec",		configMINIMAL_STACK_SIZE, 		NULL,	tskIDLE_PRIORITY + 5, 	task_two_sec_handle)	\
	ENTRY(	task_ten_second,				"task_ten_sec",		configMINIMAL_STACK_SIZE, 		NULL,	tskIDLE_PRIORITY + 6, 	task_ten_sec_handle)	\
	ENTRY(	task_one_minute,				"task_one_min",		configMINIMAL_STACK_SIZE * 2, 	NULL,	tskIDLE_PRIORITY + 6, 	task_one_min_handle)	\
	ENTRY(	task_event_kiss_rx_done,		"tev_serial_kiss",	configMINIMAL_STACK_SIZE, 		NULL,	tskIDLE_PRIORITY + 7, 	task_ev_serial_kiss_rx_done_handle)	\
	ENTRY(	task_event_kiss_tx_done,		"tev_serial_kiss_tx",	configMINIMAL_STACK_SIZE, 	NULL,	tskIDLE_PRIORITY + 7, 	task_ev_serial_kiss_tx_done_handle)	\
	ENTRY(	task_event_gsm_rx_done,			"tev_serial_gsm_rx",	configMINIMAL_STACK_SIZE, 	NULL,	tskIDLE_PRIORITY + 7, 	task_ev_serial_gsm_rx_done_handle)	\
	ENTRY(	task_event_gsm_tx_done,			"tev_serial_gsm_tx",	configMINIMAL_STACK_SIZE, 	NULL,	tskIDLE_PRIORITY + 7, 	task_ev_serial_gsm_tx_done_handle)	\

#endif /* ETC_TASKS_LIST_H_ */
