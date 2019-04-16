/*
 * beacon.c
 *
 *  Created on: 01.07.2017
 *      Author: mateusz
 */


#include "aprs/beacon.h"
#include "main.h"
#include "rte_main.h"

#include "station_config.h"

void SendOwnBeacon(void) {
//	BcnLng = sprintf(BcnInfoField, "=%07.2f%c%c%08.2f%c%c %s", this->Settings.Lat, this->Settings.LatNS, this->Settings.BcnSymbolF, this->Settings.Lon, this->Settings.LonWE, this->Settings.BcnSymbolS, this->Settings.Comment);
	main_own_aprs_msg_len = sprintf(main_own_aprs_msg, "=%07.2f%c%c%08.2f%c%c %s\0", (float)_LAT, _LATNS, _SYMBOL_F, (float)_LON, _LONWE, _SYMBOL_S, _COMMENT);
	main_own_aprs_msg[main_own_aprs_msg_len] = 0;
//	fifo_flush(&a.tx_fifo);
 	ax25_sendVia(&main_ax25, main_own_path, main_own_path_ln, main_own_aprs_msg, main_own_aprs_msg_len);
	after_tx_lock = 1;
//		for(jj = 0; jj <= 0xFBFFF; jj++);
//	while(ax25.dcd == true);
 	afsk_txStart(&main_afsk);
}

void SendStartup(void) {
	main_own_aprs_msg_len = sprintf(main_own_aprs_msg, "> START boot_cnt %#.2x hf_cnt %#.2x hf_pc %#.8x hf_lr %#.8x", rte_main_boot_cycles, rte_main_hard_faults, rte_main_hardfault_pc, rte_main_hardfault_lr);
	main_own_aprs_msg[main_own_aprs_msg_len] = 0;
 	ax25_sendVia(&main_ax25, main_own_path, main_own_path_ln, main_own_aprs_msg, main_own_aprs_msg_len);
	after_tx_lock = 1;
//		for(jj = 0; jj <= 0xFBFFF; jj++);
//	while(ax25.dcd == true);
 	afsk_txStart(&main_afsk);
}
