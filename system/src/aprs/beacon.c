/*
 * beacon.c
 *
 *  Created on: 01.07.2017
 *      Author: mateusz
 */


#include "aprs/beacon.h"
#include "main.h"

#include "station_config.h"

void SendOwnBeacon(void) {
//	BcnLng = sprintf(BcnInfoField, "=%07.2f%c%c%08.2f%c%c %s", this->Settings.Lat, this->Settings.LatNS, this->Settings.BcnSymbolF, this->Settings.Lon, this->Settings.LonWE, this->Settings.BcnSymbolS, this->Settings.Comment);
	aprs_msg_len = sprintf(aprs_msg, "=%07.2f%c%c%08.2f%c%c %s\0", (float)_LAT, _LATNS, _SYMBOL_F, (float)_LON, _LONWE, _SYMBOL_S, _COMMENT);
	aprs_msg[aprs_msg_len] = 0;
//	fifo_flush(&a.tx_fifo);
 	ax25_sendVia(&ax25, path, path_len, aprs_msg, aprs_msg_len);
	after_tx_lock = 1;
//		for(jj = 0; jj <= 0xFBFFF; jj++);
//	while(ax25.dcd == true);
 	afsk_txStart(&a);
}
