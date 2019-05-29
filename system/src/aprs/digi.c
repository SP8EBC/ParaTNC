/*
 * digi.c
 *
 *  Created on: 28.05.2017
 *      Author: mateusz
 */

#include <string.h>

#include "aprs/digi.h"
#include "main.h"
#include "TimerConfig.h"
#include "delay.h"

#include "station_config.h"
#include "config.h"

char digi_q = 0;

uint8_t digi_msg[CONFIG_AX25_FRAME_BUF_LEN];
uint16_t digi_msg_len;

uint8_t digi_limit_to_ssid789;


char Digi(struct AX25Msg *msg) {
#ifdef _DIGI

#ifdef _DIGI_ONLY_789
	digi_limit_to_ssid789 = 1;
#else
	digi_limit_to_ssid789 = 0;
#endif

	AX25Call digi_path[7];
	uint8_t call_len = 0;
	memset(digi_path, sizeof(AX25Call) * 7, 0x00);

	// check if the received message is not too long for the transmit buffers
	if (msg->len >= (CONFIG_AX25_FRAME_BUF_LEN - sizeof(AX25Call) * 7) ) {
		return DIGI_PACKET_TOO_LONG;
	}

	if ((msg->src.ssid < 7 || msg->src.ssid > 9) && digi_limit_to_ssid789 == 1) {
		return 0;
	}

	if (main_afsk.sending != 1 && after_tx_lock == 0) {
		/* funkcja wywoływana po odbiorze ramki - tu powinna być obsługa digi */
		if((msg->rpt_cnt >= 1) /*&& CheckIsOwnPacket(msg) == 0*/) {
//			if (msg->rpt_cnt == 1 && strcmp("WIDE2", msg->rpt_lst[0].call) == 0 && (msg->rpt_lst[0].ssid == 1 || msg->rpt_lst[0].ssid == 2)) {
//				/* Powtarzanie ścieżki WIDE2-1 i WIDE2-2 */
//				strcpy(digi_path[0].call, msg->dst.call);	// znak docelowy
//				digi_path[0].ssid = msg->dst.ssid;
//				strcpy(digi_path[1].call, msg->src.call);	// znak zrodlowy
//				digi_path[1].ssid = msg->src.ssid;
//				sprintf(digi_path[2].call, "%s", "SR8WXO");	// zamiana WIDE2-2 albo WIDE2-1 na znak digi
//				digi_path[2].ssid = 0x40;
//				sprintf(digi_path[3].call, "%s", "WIDE2");	// dodawanie WIDE2*
//				digi_path[3].ssid = 0x40;			/* 0x40 oznacza jedynkę na 6 bicie (przy numeracji od zera). Po przesunięciu o jedno miejsce
//													   otrzymuje się 0x80 czyli jedynkę na H-bicie */
//				digi_q = 1;
//				call_len = 4;	// długość ścieżki

//			}
			if (msg->rpt_cnt == 1 && strcmp("WIDE1", msg->rpt_lst[0].call) == 0 && msg->rpt_lst[0].ssid == 1 ) {
				/* Powtarzanie ścieżki WIDE1-1 */
				strcpy(digi_path[0].call, msg->dst.call);	// znak docelowy
				digi_path[0].ssid = msg->dst.ssid;
				strcpy(digi_path[1].call, msg->src.call);	// znak zrodlowy
				digi_path[1].ssid = msg->src.ssid;
				sprintf(digi_path[2].call, "%s", _CALL);	// zamiana WIDE2-2 albo WIDE2-1 na znak digi
				digi_path[2].ssid = (_SSID | 0x40);
				sprintf(digi_path[3].call, "%s", "WIDE1");	// dodawanie WIDE2*
				digi_path[3].ssid = 0x40;			/* 0x40 oznacza jedynkę na 6 bicie (przy numeracji od zera). Po przesunięciu o jedno miejsce
													   otrzymuje się 0x80 czyli jedynkę na H-bicie */
				digi_q = 1;
				call_len = 4;	// długość ścieżki

			}
			else if(msg->rpt_cnt > 1 && strcmp("WIDE1", msg->rpt_lst[0].call) == 0 && strcmp("WIDE2", msg->rpt_lst[1].call) == 0 && msg->rpt_lst[0].ssid == 1 && msg->rpt_lst[1].ssid == 1) {
				/* Powtarzanie aliasu WIDE1-1 w sciezce typu WIDE1-1,WIDE2-1     */
				strcpy(digi_path[0].call, msg->dst.call);	// znak docelowy
				digi_path[0].ssid = msg->dst.ssid;
				strcpy(digi_path[1].call, msg->src.call);	// znak zrodlowy
				digi_path[1].ssid = msg->src.ssid;
				sprintf(digi_path[2].call, "%s", _CALL);	// zamiana WIDE1-1 na własny znak digi
				digi_path[2].ssid =  (_SSID | 0x40);
				sprintf(digi_path[3].call, "%s", "WIDE1");	// dodawanie WIDE1* na końcu
				digi_path[3].ssid = 0x40;
				sprintf(digi_path[4].call, "%s", msg->rpt_lst[1].call);	// przepisywanie WIDE2-1
				digi_path[4].ssid = msg->rpt_lst[1].ssid;
				digi_q = 1;
				call_len = 5;
			}
			else if(msg->rpt_cnt > 1 && strcmp("WIDE1", msg->rpt_lst[0].call) == 0 && strcmp("WIDE2", msg->rpt_lst[1].call) == 0 && msg->rpt_lst[0].ssid == 1 && msg->rpt_lst[1].ssid == 2) {
				/* Powtarzanie aliasu WIDE1-1 w sciezce typu WIDE1-1,WIDE2-2     */
				strcpy(digi_path[0].call, msg->dst.call);	// znak docelowy
				digi_path[0].ssid = msg->dst.ssid;
				strcpy(digi_path[1].call, msg->src.call);	// znak zrodlowy
				digi_path[1].ssid = msg->src.ssid;
				sprintf(digi_path[2].call, "%s", _CALL);	// zamiana WIDE1-1 na własny znak digi
				digi_path[2].ssid = (_SSID | 0x40);
				sprintf(digi_path[3].call, "%s", "WIDE1");	// dodawanie WIDE1* na końcu
				digi_path[3].ssid = 0x40;
				sprintf(digi_path[4].call, "%s", "WIDE2");	// skracanie dalszej czesci do WIDE2-1
				digi_path[4].ssid = 1;
				digi_q = 1;
				call_len = 5;
			}
//			else if (msg->rpt_cnt >= 2 && strcmp("WIDE2", msg->rpt_lst[1].call) == 0 && msg->rpt_lst[1].ssid == 1) {
//				/* Powtarzanie aliasu WIDE2-1 w sciezce WIDE2*,WIDE2-1 */
//				strcpy(digi_path[0].call, msg->dst.call);	// znak docelowy
//				digi_path[0].ssid = msg->dst.ssid;
//				strcpy(digi_path[1].call, msg->src.call);	// znak zrodlowy
//				digi_path[1].ssid = msg->src.ssid;
//				sprintf(digi_path[2].call, "%s", msg->rpt_lst[0].call);	// dołączanie znaku poprzedniego digi
//				digi_path[2].ssid = (msg->rpt_lst[0].ssid | 0x40);
//				sprintf(digi_path[3].call, "%s", "SR8WXO");	// zamiana WIDE2-1 na własny znak digi
//				digi_path[3].ssid = 0x40;
//				sprintf(digi_path[4].call, "%s", "WIDE2");	// dodawanie WIDE2* na końcu
//				digi_path[4].ssid = 0x40;
//				digi_q = 1;
//				call_len = 5;
//			}
//			else if(msg->rpt_cnt >= 2 && strcmp("WIDE2", msg->rpt_lst[2].call) == 0 && (msg->rpt_lst[2].ssid == 1 || msg->rpt_lst[2].ssid == 2)) {
//				/* Powtarzanie aliasu WIDE2-1 oraz WIDE 2-2 w sciezce typu WIDE1*,WIDE2-n     */
//				strcpy(digi_path[0].call, msg->dst.call);	// znak docelowy
//				digi_path[0].ssid = msg->dst.ssid;
//				strcpy(digi_path[1].call, msg->src.call);	// znak zrodlowy
//				digi_path[1].ssid = msg->src.ssid;
//				sprintf(digi_path[2].call, "%s", msg->rpt_lst[0].call);	// dołączanie znaku poprzedniego digi
//				digi_path[2].ssid = (msg->rpt_lst[0].ssid | 0x40);
// 				sprintf(digi_path[3].call, "%s", "WIDE1");	// dodawanie WIDE1 na końcu
//				digi_path[3].ssid = 0x40;
//				sprintf(digi_path[4].call, "%s", "SR8WXO");		// zamiana WIDE2-1 albo WIDE2-2 na własny znak digi
//				digi_path[4].ssid = 0x40;
//				sprintf(digi_path[5].call, "%s", "WIDE2");	// dodawanie WIDE2* na końcu
//				digi_path[5].ssid = 0x40;
//				digi_q = 1;
//				call_len = 6;
//			}
//			else if(msg->rpt_cnt >= 4 && strcmp("WIDE2", msg->rpt_lst[3].call) == 0 && (msg->rpt_lst[3].ssid == 1 || msg->rpt_lst[3].ssid == 2)) {
//				/* Powtarzanie aliasu WIDE2-1 w sciezce typu WIDE1*,WIDE2*,WIDE2-1     */
//				strcpy(digi_path[0].call, msg->dst.call);	// znak docelowy
//				digi_path[0].ssid = msg->dst.ssid;
//				strcpy(digi_path[1].call, msg->src.call);	// znak zrodlowy
//				digi_path[1].ssid = msg->src.ssid;
//				sprintf(digi_path[2].call, "%s", msg->rpt_lst[0].call);	// dołączanie znaku poprzedniego digi WIDE1*
//				digi_path[2].ssid = (msg->rpt_lst[0].ssid | 0x40);
//				sprintf(digi_path[3].call, "%s", "WIDE1");	// dodawanie WIDE1
//				digi_path[3].ssid = 0x40;
//				sprintf(digi_path[4].call, "%s", msg->rpt_lst[2].call);	// dołączanie znaku poprzedniego digi WIDE2*
//				digi_path[4].ssid = (msg->rpt_lst[2].ssid | 0x40);
//				sprintf(digi_path[5].call, "%s", "SR8WXO");		// zamiana WIDE2-1 na własny znak digi
//				digi_path[5].ssid = 0x40;
//				sprintf(digi_path[6].call, "%s", "WIDE2");	// dodawanie WIDE2* na końcu
//				digi_path[6].ssid = 0x40;
//				digi_q = 1;
//				call_len = 7;
//			}
			else
				digi_q = 0;

			if (digi_q == 1) {
#ifdef _DBG_TRACE
				trace_printf("Digi:call_len=%d\r\n", call_len);
#endif
				digi10m++;
				digi_msg_len = msg->len+1;
				snprintf(digi_msg, msg->len+1, "%s", msg->info);
				delay_from_preset();

				while(main_ax25.dcd == true);
				ax25_sendVia(&main_ax25, digi_path, call_len, digi_msg, digi_msg_len-1);
				after_tx_lock = 1;
				afsk_txStart(&main_afsk);
				return 1;
			}
			else {
				;
			}
			}
		else {
			;
		}
	}
	else
		after_tx_lock = 0;
#endif
	return 0;
}
