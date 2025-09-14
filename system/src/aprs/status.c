/*
 * status.c
 *
 *  Created on: Jul 22, 2023
 *      Author: mateusz
 */

#include "status.h"

#include "main.h"
#include "backup_registers.h"

#include <stdio.h>
#include <string.h>

#include "ax25.h"

#ifdef PARAMETEO
#include "pwr_save.h"

const char * status_vbatt_normal 		= "VBATT_GOOD";
const char * status_vbatt_low 			= "VBATT_LOW";
const char * status_vbatt_cutoff 		= "VBATT_CUTOFF";
const char * status_vbatt_unknown		= "VBATT_UNKNOWN";

#include "gsm/sim800c_gprs.h"
#include "gsm/sim800c.h"
#endif

/* FreeRTOS includes. */
#include <FreeRTOS.h>
#include <task.h>

void status_send(void) {
	memset(main_own_aprs_msg, 0x00, sizeof(main_own_aprs_msg));
	taskENTER_CRITICAL();
#ifdef STM32L471xx
	main_own_aprs_msg_len = snprintf(main_own_aprs_msg, OWN_APRS_MSG_LN, ">ParaMETEO firmware %s-%s by SP8EBC - PV powered, fully outdoor, 3in1 APRS device", SW_VER, SW_DATE);
#else
	main_own_aprs_msg_len = snprintf(main_own_aprs_msg, OWN_APRS_MSG_LN, ">ParaTNC firmware %s-%s by SP8EBC", SW_VER, SW_DATE);
#endif
	taskEXIT_CRITICAL();
	ax25_sendVia(&main_ax25, main_own_path, main_own_path_ln, main_own_aprs_msg, main_own_aprs_msg_len);
	WAIT_FOR_CHANNEL_FREE();
	afsk_txStart(&main_afsk);

}


void status_send_raw_values_modbus(void) {
#ifdef _MODBUS_RTU
	uint8_t status_ln = 0;

	rtu_get_raw_values_string(main_own_aprs_msg, OWN_APRS_MSG_LN, &status_ln);

 	ax25_sendVia(&main_ax25, main_own_path, main_own_path_ln, main_own_aprs_msg, status_ln);
	WAIT_FOR_CHANNEL_FREE();
	afsk_txStart(&main_afsk);
	main_wait_for_tx_complete();
#endif
}


void status_send_powersave_cutoff(uint16_t battery_voltage, int8_t previous_cutoff, int8_t current_cutoff) {
#ifdef PARAMETEO
	const char *p, *c;

	// telemetry_vbatt_unknown

	if ((previous_cutoff & CURRENTLY_CUTOFF) != 0) {
		p = status_vbatt_cutoff;
	}
	else if ((previous_cutoff & CURRENTLY_VBATT_LOW) != 0) {
		p = status_vbatt_low;
	}
	else if (((previous_cutoff & CURRENTLY_CUTOFF) == 0) && (previous_cutoff & CURRENTLY_VBATT_LOW) == 0){
		p = status_vbatt_normal;
	}
	else {
		p = status_vbatt_unknown;
	}

	if ((current_cutoff & CURRENTLY_CUTOFF) != 0) {
		c = status_vbatt_cutoff;
	}
	else if ((current_cutoff & CURRENTLY_VBATT_LOW) != 0) {
		c = status_vbatt_low;
	}
	else if (((current_cutoff & CURRENTLY_CUTOFF) == 0) && (current_cutoff & CURRENTLY_VBATT_LOW) == 0){
		c = status_vbatt_normal;
	}
	else {
		c = status_vbatt_unknown;
	}

	main_wait_for_tx_complete();

	taskENTER_CRITICAL();
	memset(main_own_aprs_msg, 0x00, sizeof(main_own_aprs_msg));
	main_own_aprs_msg_len = snprintf(main_own_aprs_msg, OWN_APRS_MSG_LN, ">[powersave cutoff change][Vbatt: %dV][previous: %d - %s][currently: %d - %s]", battery_voltage, previous_cutoff, p, current_cutoff, c);
	taskEXIT_CRITICAL();
	ax25_sendVia(&main_ax25, main_own_path, main_own_path_ln, main_own_aprs_msg, main_own_aprs_msg_len);
	//while (main_ax25.dcd == 1);
	afsk_txStart(&main_afsk);
	main_wait_for_tx_complete();
#endif
}

void status_send_powersave_registers(void) {

   const uint32_t last_sleep = backup_reg_get_last_sleep_timestamp();
   const uint32_t last_wakeup = backup_reg_get_last_wakeup_timestamp();
   const uint32_t sleep_counter = backup_reg_get_sleep_counter();
   const uint32_t wakeup_counter = backup_reg_get_wakeup_counter();
   const uint32_t last_monitor = backup_reg_get_monitor();

	main_wait_for_tx_complete();

	taskENTER_CRITICAL();
	memset(main_own_aprs_msg, 0x00, sizeof(main_own_aprs_msg));
	main_own_aprs_msg_len = snprintf(
									main_own_aprs_msg,
									OWN_APRS_MSG_LN,
									">[powersave registers][last_sleep_ts: 0x%lX][last_wakeup_ts: 0x%lX][sleep_cntr: 0x%lX][wakeup_cntr: 0x%lX][monitor: 0x%lX]",
									last_sleep,
									last_wakeup,
									sleep_counter,
									wakeup_counter,
									last_monitor);
	taskEXIT_CRITICAL();
 	ax25_sendVia(&main_ax25, main_own_path, main_own_path_ln, main_own_aprs_msg, main_own_aprs_msg_len);
	//while (main_ax25.dcd == 1);
	afsk_txStart(&main_afsk);
	main_wait_for_tx_complete();
}

void status_send_gsm(void){
	main_wait_for_tx_complete();

	// clear buffer
	memset(main_own_aprs_msg, 0x00, sizeof(main_own_aprs_msg));

	// append general status information about network connection
	gsm_sim800_create_status(main_own_aprs_msg, OWN_APRS_MSG_LN);

	// measure a lenght of existing status string
	const size_t size_so_far = strlen(main_own_aprs_msg);

	// append an ip address
	sim800_gprs_create_status(main_own_aprs_msg + size_so_far, OWN_APRS_MSG_LN - size_so_far);

	// full lenght of status string
	main_own_aprs_msg_len =  strlen(main_own_aprs_msg);

	// send message on radio
 	ax25_sendVia(&main_ax25, main_own_path, main_own_path_ln, main_own_aprs_msg, main_own_aprs_msg_len);
	afsk_txStart(&main_afsk);

}

void status_send_aprsis_timeout(uint8_t unsuccessfull_conn_cntr) {
	main_wait_for_tx_complete();

	taskENTER_CRITICAL();

	// clear buffer
	memset(main_own_aprs_msg, 0x00, sizeof(main_own_aprs_msg));

	// create message buffer
	main_own_aprs_msg_len = snprintf(main_own_aprs_msg, OWN_APRS_MSG_LN, ">[APRS-IS timeout][aprsis_unsucessfull_conn_counter: %d]", (int)unsuccessfull_conn_cntr);

	taskEXIT_CRITICAL();

	// send message on radio
	ax25_sendVia(&main_ax25, main_own_path, main_own_path_ln, main_own_aprs_msg, main_own_aprs_msg_len);
	afsk_txStart(&main_afsk);
}

void status_send_from_exposed_eveny_log(const event_log_exposed_t * const event) {
	main_wait_for_tx_complete();

	// clear buffer
	memset(main_own_aprs_msg, 0x00, sizeof(main_own_aprs_msg));

	// put '>' at the begining
	*main_own_aprs_msg = '>';

	// convert event to string representation, but shift output buffer by one, to make a room for '>'
	const uint16_t str_size = event_exposed_to_string(event, main_own_aprs_msg + 1, OWN_APRS_MSG_LN - 1);

	// set message lenght
	main_own_aprs_msg_len = str_size + 1;

	// send message on radio
	ax25_sendVia(&main_ax25, main_own_path, main_own_path_ln, main_own_aprs_msg, main_own_aprs_msg_len);
	afsk_txStart(&main_afsk);
}

