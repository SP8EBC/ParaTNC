/*
 * telemetry.c
 *
 *  Created on: 01.07.2017
 *      Author: mateusz
 */

#include "aprs/telemetry.h"
#include "main.h"
#include "station_config.h"
#include "delay.h"

uint16_t telemetry_counter = 0;

void telemetry_send_chns_description(void) {
	while (main_afsk.sending == 1);

#if (_SSID == 0)
	aprs_msg_len = sprintf(main_own_aprs_msg, ":%s   :PARM.Rx10min,Tx10min,Digi10min,HostTx10m,Tempre,DS_QF_FULL,DS_QF_DEGRAD,DS_QF_NAVBLE,MS_QF_FULL,DHT_QF_FULL,N", _CALL);
#endif
#if (_SSID > 0 && _SSID <= 9)
	aprs_msg_len = sprintf(main_own_aprs_msg, ":%s-%d :PARM.Rx10min,Tx10min,Digi10min,HostTx10m,Tempre,DS_QF_FULL,DS_QF_DEGRAD,DS_QF_NAVBLE,MS_QF_FULL,DHT_QF_FULL,N", _CALL, _SSID);
#endif
#if (_SSID > 9 && _SSID <= 15)
aprs_msg_len = sprintf(main_own_aprs_msg, ":%s-%d:PARM.Rx10min,Tx10min,Digi10min,HostTx10m,Tempre,DS_QF_FULL,DS_QF_DEGRADA,DS_QF_NAVBLE,MS_QF_FULL,DHT_QF_FULL,N", _CALL, _SSID);
#endif
	main_own_aprs_msg[aprs_msg_len] = 0;
	ax25_sendVia(&main_ax25, path, path_len, main_own_aprs_msg, aprs_msg_len);
	after_tx_lock = 1;
	afsk_txStart(&main_afsk);

	while (main_afsk.sending == 1);

	delay_fixed(1200);

#if (_SSID == 0)
	aprs_msg_len = sprintf(main_own_aprs_msg, ":%s   :EQNS.0,1,0,0,1,0,0,1,0,0,1,0,0,0.25,-25", _CALL);
#endif
#if (_SSID > 0 && _SSID <= 9)
	aprs_msg_len = sprintf(main_own_aprs_msg, ":%s-%d :EQNS.0,1,0,0,1,0,0,1,0,0,1,0,0,0.25,-25", _CALL, _SSID);
#endif
#if (_SSID > 9 && _SSID <= 15)
	aprs_msg_len = sprintf(main_own_aprs_msg, ":%s-%d:EQNS.0,1,0,0,1,0,0,1,0,0,1,0,0,0.25,-25", _CALL, _SSID);
#endif
	main_own_aprs_msg[aprs_msg_len] = 0;
	ax25_sendVia(&main_ax25, path, path_len, main_own_aprs_msg, aprs_msg_len);
	after_tx_lock = 1;
	afsk_txStart(&main_afsk);

	while (main_afsk.sending == 1);

	delay_fixed(1200);

#if (_SSID == 0)
	aprs_msg_len = sprintf(main_own_aprs_msg, ":%s   :UNIT.Pkt,Pkt,Pkt,Pkt,DegC,Hi,Hi,Hi,Hi,Hi,N", _CALL);
#endif
#if (_SSID > 0 && _SSID <= 9)
	aprs_msg_len = sprintf(main_own_aprs_msg, ":%s-%d :UNIT.Pkt,Pkt,Pkt,Pkt,DegC,Hi,Hi,Hi,Hi,Hi,N", _CALL, _SSID);
#endif
#if (_SSID > 9 && _SSID <= 15)
	aprs_msg_len = sprintf(main_own_aprs_msg, ":%s-%d:UNIT.Pkt,Pkt,Pkt,Pkt,DegC,Hi,Hi,Hi,Hi,Hi,N", _CALL, _SSID);
#endif
	main_own_aprs_msg[aprs_msg_len] = 0;
	ax25_sendVia(&main_ax25, path, path_len, main_own_aprs_msg, aprs_msg_len);
	after_tx_lock = 1;
	afsk_txStart(&main_afsk);

	delay_fixed(1200);

}

void telemetry_send_values(	uint8_t rx_pkts,
							uint8_t tx_pkts,
							uint8_t digi_pkts,
							uint8_t kiss_pkts,
							float temperature,
							DallasQF dallas_qf,
							ms5611_qf_t ms_qf,
							dht22QF ds_qf) {


	// local variables with characters to be inserted to APRS telemetry frame
	char qf = '0', degr = '0', nav = '0';
	char ms_qf_full = '0';
	char dht_qf_full = '0';

	uint8_t scaled_temperature = 0;

	if (temperature < -25.0f) {
		scaled_temperature = (uint8_t)0;
	}
	else if (temperature > 38.75f) {
		scaled_temperature = (uint8_t)255;
	}
	else {
		scaled_temperature = (uint8_t)((temperature + 25.0f) * 4.0f);
	}

	switch (dallas_qf) {
	case DALLAS_QF_FULL:
		qf = '1', degr = '0', nav = '0';
		break;
	case DALLAS_QF_DEGRADATED:
		qf = '0', degr = '1', nav = '0';
		break;

	case DALLAS_QF_NOT_AVALIABLE:
		qf = '0', degr = '0', nav = '1';
		break;

	default:
		qf = '0', degr = '0', nav = '0';
		break;
	}

	switch (ms_qf) {
	case MS5611_QF_FULL:
		 ms_qf_full = '1';
		 break;
	default:
		ms_qf_full = '0';
		break;
	}

	switch (ds_qf) {
	case DHT22_QF_FULL:
		dht_qf_full = '1';
		break;
	default:
		dht_qf_full = '0';
	}

#ifdef _DALLAS_AS_TELEM
	aprs_msg_len = sprintf(main_own_aprs_msg, "T#%03d,%03d,%03d,%03d,%03d,%03d,%c%c%c%c%c000", telemetry_counter++, rx_pkts, tx_pkts, digi_pkts, kiss_pkts, scaled_temperature, qf, degr, nav, ms_qf_full, dht_qf_full);
#else
	aprs_msg_len = sprintf(main_own_aprs_msg, "T#%03d,%03d,%03d,%03d,%03d,%03d,%c%c%c%c%c000", telemetry_counter++, rx_pkts, tx_pkts, digi_pkts, kiss_pkts, 0, qf, degr, nav, ms_qf_full, dht_qf_full);
#endif

	if (telemetry_counter > 999)
		telemetry_counter = 0;
	main_own_aprs_msg[aprs_msg_len] = 0;
 	ax25_sendVia(&main_ax25, path, path_len, main_own_aprs_msg, aprs_msg_len);
	after_tx_lock = 1;
//	while(ax25.dcd == true);


	afsk_txStart(&main_afsk);
}

