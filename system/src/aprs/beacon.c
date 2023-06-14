/*
 * beacon.c
 *
 *  Created on: 01.07.2017
 *      Author: mateusz
 */


#include "aprs/beacon.h"
#include "main.h"
#include "rte_main.h"
#include "pwr_save_configuration.h"

#include "station_config.h"

#include <stdio.h>
#include <string.h>

#ifdef INHIBIT_CUTOFF
const char * idiot = "You idiot!! You have INHIBIT_CUTOFF define uncomented!\0\0";
#endif

void beacon_send_own(uint16_t voltage, uint8_t rtc_ok) {
	main_wait_for_tx_complete();
#ifdef INHIBIT_CUTOFF
	  main_own_aprs_msg_len = sprintf(main_own_aprs_msg, "=%s%c%c%s%c%c %s", main_string_latitude, main_config_data_basic->n_or_s, main_symbol_f, main_string_longitude, main_config_data_basic->e_or_w, main_symbol_s, idiot);

#else
	if (voltage == 0 && rtc_ok == 0) {
		  main_own_aprs_msg_len = sprintf(main_own_aprs_msg, "=%s%c%c%s%c%c %s", main_string_latitude, main_config_data_basic->n_or_s, main_symbol_f, main_string_longitude, main_config_data_basic->e_or_w, main_symbol_s, main_config_data_basic->comment);
	}
	else {
		  main_own_aprs_msg_len = sprintf(main_own_aprs_msg, "=%s%c%c%s%c%c %s [vbat: %d][rtc_ok: %d]", main_string_latitude, main_config_data_basic->n_or_s, main_symbol_f, main_string_longitude, main_config_data_basic->e_or_w, main_symbol_s, main_config_data_basic->comment, (int)voltage , (int)rtc_ok);
	}
#endif
	main_own_aprs_msg[main_own_aprs_msg_len] = 0;
 	ax25_sendVia(&main_ax25, main_own_path, main_own_path_ln, main_own_aprs_msg, main_own_aprs_msg_len);
	after_tx_lock = 1;
 	afsk_txStart(&main_afsk);
}

void beacon_send_on_startup(void) {
	main_own_aprs_msg_len = sprintf(main_own_aprs_msg, "> START boot_cnt %#.2x hf_cnt %#.2x", (uint16_t)rte_main_boot_cycles, (uint16_t)rte_main_hard_faults);
	main_own_aprs_msg[main_own_aprs_msg_len] = 0;
 	ax25_sendVia(&main_ax25, main_own_path, main_own_path_ln, main_own_aprs_msg, main_own_aprs_msg_len);
	after_tx_lock = 1;
 	afsk_txStart(&main_afsk);
}

#ifdef INHIBIT_CUTOFF
void beacon_send_from_user_content(uint16_t content_ln, char* content_ptr) {
	main_wait_for_tx_complete();
 	ax25_sendVia(&main_ax25, main_own_path, main_own_path_ln, idiot, strlen(idiot));
	after_tx_lock = 1;
 	afsk_txStart(&main_afsk);
}
#else
void beacon_send_from_user_content(uint16_t content_ln, char* content_ptr) {
	main_wait_for_tx_complete();
 	ax25_sendVia(&main_ax25, main_own_path, main_own_path_ln, content_ptr, content_ln);
	after_tx_lock = 1;
 	afsk_txStart(&main_afsk);
}
#endif
