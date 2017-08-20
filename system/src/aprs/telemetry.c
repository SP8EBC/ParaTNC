/*
 * telemetry.c
 *
 *  Created on: 01.07.2017
 *      Author: mateusz
 */

#include "aprs/telemetry.h"
#include "main.h"
#include "station_config.h"
#include "TimerConfig.h"

volatile int jj = 0;

extern volatile int delay_5us;


void SendSimpleTelemetry(char num) {
	aprs_msg_len = sprintf(aprs_msg, "T#%03d,%03d,%03d,%03d,000,000,00000000", t++, rx10m, tx10m, digi10m);
	if (t > 999)
		t = 0;
	aprs_msg[aprs_msg_len] = 0;
 	ax25_sendVia(&ax25, path, path_len, aprs_msg, aprs_msg_len);
	after_tx_lock = 1;
	while(ax25.dcd == true);

	// delay before transmit
	TIM2Delay(_DELAY_BASE);
	while(delay_5us != 0);
	TIM2DelayDeConfig();
	// .. end delay

	afsk_txStart(&a);
	rx10m = 0, tx10m = 0, digi10m = 0;
	if (num > 0) {
			while (a.sending == 1);
			for(jj = 0; jj <= 0x1F5FFF; jj++);

#if (_SSID == 0)
			aprs_msg_len = sprintf(aprs_msg, ":%s   :PARM.Rx10min,Tx10min,Digi10min,N,N,N,N,N,N,N,N", _CALL);
#endif
#if (_SSID > 0 && _SSID <= 9)
			aprs_msg_len = sprintf(aprs_msg, ":%s-%d :PARM.Rx10min,Tx10min,Digi10min,N,N,N,N,N,N,N,N", _CALL, _SSID);
#endif
#if (_SSID > 9 && _SSID <= 15)
	aprs_msg_len = sprintf(aprs_msg, ":%s-%d:PARM.Rx10min,Tx10min,Digi10min,N,N,N,N,N,N,N,N", _CALL, _SSID);
#endif
			aprs_msg[aprs_msg_len] = 0;
			ax25_sendVia(&ax25, path, path_len, aprs_msg, aprs_msg_len);
			after_tx_lock = 1;
			afsk_txStart(&a);

			while (a.sending == 1);
			for(jj = 0; jj <= 0x1F5FFF; jj++);

#if (_SSID == 0)
			aprs_msg_len = sprintf(aprs_msg, ":%s   :EQNS.0,1,0,0,1,0,0,1,0,0,1,0,0,1,0", _CALL);
#endif
#if (_SSID > 0 && _SSID <= 9)
			aprs_msg_len = sprintf(aprs_msg, ":%s-%d :EQNS.0,1,0,0,1,0,0,1,0,0,1,0,0,1,0", _CALL, _SSID);
#endif
#if (_SSID > 9 && _SSID <= 15)
			aprs_msg_len = sprintf(aprs_msg, ":%s-%d:EQNS.0,1,0,0,1,0,0,1,0,0,1,0,0,1,0", _CALL, _SSID);
#endif
			aprs_msg[aprs_msg_len] = 0;
			ax25_sendVia(&ax25, path, path_len, aprs_msg, aprs_msg_len);
			after_tx_lock = 1;
			afsk_txStart(&a);

				while (a.sending == 1);
				for(jj = 0; jj <= 0x1F5FFF; jj++);

#if (_SSID == 0)
			aprs_msg_len = sprintf(aprs_msg, ":%s   :UNIT.Pkt,Pkt,Pkt,N,N,N,N,N,N,N,N", _CALL);
#endif
#if (_SSID > 0 && _SSID <= 9)
			aprs_msg_len = sprintf(aprs_msg, ":%s-%d :UNIT.Pkt,Pkt,Pkt,N,N,N,N,N,N,N,N", _CALL, _SSID);
#endif
#if (_SSID > 9 && _SSID <= 15)
			aprs_msg_len = sprintf(aprs_msg, ":%s-%d:UNIT.Pkt,Pkt,Pkt,N,N,N,N,N,N,N,N", _CALL, _SSID);
#endif
			aprs_msg[aprs_msg_len] = 0;
			ax25_sendVia(&ax25, path, path_len, aprs_msg, aprs_msg_len);
			after_tx_lock = 1;
			afsk_txStart(&a);

			for(jj = 0; jj <= 0x1AFFFF; jj++);
	}
}

