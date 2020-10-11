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

// global variables to store a frame content to be digipeated
uint8_t digi_msg[CONFIG_AX25_FRAME_BUF_LEN];
uint16_t digi_msg_len;
AX25Call digi_path[7];
uint8_t digi_call_len = 0;

// digipeater working mode
digi_mode_t digi_mode;

uint8_t digi_viscous_delay_sec;
uint8_t digi_viscous_counter_sec;


void digi_init(void) {
	digi_viscous_counter_sec = 0;

	digi_msg_len = 0;

	digi_mode = DIGI_OFF;

#ifdef _DIGI_VISCOUS

	#ifdef _DIGI_ONLY_789
		digi_mode = DIGI_VISCOUS_SSID_WIDE1;
	#else
		digi_mode = DIGI_VISCOUS_ALL_WIDE1;
	#endif

#else

	#ifdef _DIGI_ONLY_789
		digi_mode = DIGI_ON_SSID_WIDE1;
	#else
		digi_mode = DIGI_ON_ALL_WIDE1;
	#endif

#endif

#ifdef _DIGI_VISCOUS_DEALY
		digi_viscous_delay_sec =_DIGI_VISCOUS_DEALY;
#else
		digi_viscous_delay_sec = 3;
#endif
}

uint8_t digi_process(struct AX25Msg *msg) {
#ifdef _DIGI



	uint8_t retval = DIGI_PACKET_DIDNT_DIGIPEATED;

	// check if the received message is not too long for the transmit buffers
	if (msg->len >= (CONFIG_AX25_FRAME_BUF_LEN - sizeof(AX25Call) * 7) ) {
		return DIGI_PACKET_TOO_LONG;
	}

	if ((msg->src.ssid < 7 || msg->src.ssid > 9) && (digi_mode == DIGI_ON_SSID_WIDE1 || digi_mode == DIGI_VISCOUS_SSID_WIDE1)) {
		return DIGI_PACKET_DIDNT_DIGIPEATED;
	}

	if (after_tx_lock == 0) {
		// if the packet has any path and there is no packed waiting in viscous delay
		if(msg->rpt_cnt >= 1 && digi_msg_len == 0) {

			// initialize global variables used to store digipeated packet
			memset(digi_path, 0x00, sizeof(AX25Call) * 7);
			memset(digi_msg, 0x00, CONFIG_AX25_FRAME_BUF_LEN);

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
				retval = DIGI_PACKET_DIGIPEATED;
				digi_call_len = 4;	// długość ścieżki

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
				retval = DIGI_PACKET_DIGIPEATED;
				digi_call_len = 5;
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
				retval = DIGI_PACKET_DIGIPEATED;
				digi_call_len = 5;
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

			if (retval == DIGI_PACKET_DIGIPEATED) {

				// copying the content
				digi_msg_len = msg->len+1;
				snprintf(digi_msg, msg->len+1, "%s", msg->info);

				// if Viscous mode is not enabled proceed to transmission immediately
				if (digi_mode == DIGI_ON_SSID_WIDE1 || digi_mode == DIGI_ON_ALL_WIDE1) {
					digi10m++;

					// delaying retransmission.. well this will block I/O for a while
					// and it could be refactor to ommit that but for now it needs to stay
					// as it is.
					delay_from_preset();

					while(main_ax25.dcd == true);
					ax25_sendVia(&main_ax25, digi_path, digi_call_len, digi_msg, digi_msg_len-1);
					after_tx_lock = 1;
					afsk_txStart(&main_afsk);

					// clear variables when there are not needed
					memset(digi_path, 0x00, sizeof(AX25Call) * 7);
					memset(digi_msg, 0x00, CONFIG_AX25_FRAME_BUF_LEN);
					digi_msg_len = 0;

					return retval;

				} // digi_mode == DIGI_ON_SSID_WIDE1 || digi_mode == DIGI_ON_ALL_WIDE1
			}	// retval == DIGI_PACKET_DIGIPEATED
		} // msg->rpt_cnt >= 1 && digi_msg_len == 0
	}
	else {
		after_tx_lock = 0;
	}
#endif
	return retval;
}

uint8_t digi_check_with_viscous(struct AX25Msg *msg) {
	uint8_t retval = 0;

	// if there is a message waiting in buffer for digipeating
	if (digi_msg_len > 0) {
		// check the source call
		if (strncmp(msg->src.call, digi_path[1].call, 6) == 0)  {
			// if the source call is the same check the SSID
			if (msg->src.ssid == digi_path[1].ssid) {
				// this is a message from the same station check the message content
				if (memcmp(msg->info, digi_msg, msg->len) == 0) {
					// if the message content is the same it means that current buffer content shall be discarded
					// and frame shall not be retransmited
					digi_msg_len = 0;
					memset(digi_msg, 0x00, CONFIG_AX25_FRAME_BUF_LEN);

					// increase viscous drop counter
					digidrop10m++;
				}
			}
		}
	}

	return retval;
}

uint8_t digi_pool_viscous(void) {
	uint8_t retval = DIGI_PACKET_DIDNT_DIGIPEATED;

	// proceed only if Viscous mode is enabled
	if (digi_mode == DIGI_VISCOUS_ALL_WIDE1 || digi_mode == DIGI_VISCOUS_SSID_WIDE1) {

		// if there is any message waiting in viscous delay
		if (digi_msg_len > 0) {
			digi_viscous_counter_sec++;

			// if the counter reach the delay limit and packet still waits
			// so it hasn't been cleared by 'digi_check_with_viscous'
			if (digi_viscous_counter_sec >= digi_viscous_delay_sec) {

				// wait when radio channel will became avaliable
				while(main_ax25.dcd == true);

				// put message vaiting in viscous dealy into AX25 buffer in correct, encoded form
				ax25_sendVia(&main_ax25, digi_path, digi_call_len, digi_msg, digi_msg_len-1);
				after_tx_lock = 1;

				// start transmission (non blicking call)
				afsk_txStart(&main_afsk);

				// clear variables when there are not needed
				memset(digi_path, 0x00, sizeof(AX25Call) * 7);
				memset(digi_msg, 0x00, CONFIG_AX25_FRAME_BUF_LEN);
				digi_msg_len = 0;

				digi10m++;

				retval = DIGI_PACKET_DIGIPEATED;
			}

		}
		else {
			// if there isn't only zero the counter and do nothing
			digi_viscous_counter_sec = 0;
		}

	}

	return retval;
}

