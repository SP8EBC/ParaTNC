/*
 * KissCommunication.c
 *
 *  Created on: 29.04.2017
 *      Author: mateusz
 */

#include "KissCommunication.h"
//#include "main.h"

#include "diag/Trace.h"
#include "station_config.h"
#include "TimerConfig.h"

extern unsigned short tx10m;
extern volatile int delay_5us;


short SendKISSToHost(KissFrame* in, uint8_t* frame, short int frm_len, uint8_t* output) {
	#define FEND	(uint8_t)0xC0
	#define FESC	(uint8_t)0xDB
	#define TFEND	(uint8_t)0xDC
	#define TFESC	(uint8_t)0xDD
	short int i /* Zmienna do poruszania siê po frame */, j /* zmienna do poruszani siê po data*/;

	uint8_t* data;
	if (output != 0x00)
		data = output;
	else
		data = in->data;

	data[0] = FEND;
	data[1] = 0x00;
//	KissFrm.data[2] = HDLC_FLAG;
	for (j = 2, i = 0; i < frm_len; j++, i++) {
		if (*(frame+i) != FEND && *(frame+i) != FESC)
			data[j] = frame[i];
		else if (*(frame+i) == FEND) {
			data[j] = FESC;
			*(data + (j++)) = TFEND;
		}
		else if(*(frame+i) == FESC) {
			data[j] = FESC;
			*(data + (j++)) = TFESC;
		}
		else {

		}
	}
//	*(KissFrm.data + (j++)) = HDLC_FLAG;
	*(data + (j++)) = FEND;
	in->lng = j;
	return j;
}

short ParseReceivedKISS(uint8_t* SrlRXData, AX25Ctx* ax25, Afsk* a) {
	int i/* zmienna do poruszania sie po buforze odbiorczym usart */;
	int j/* zmienna do poruszania sie po lokalnej tablicy do przepisywania*/;
	uint8_t FrameBuff[100];
	if (*(SrlRXData) != FEND)
		return 1;
	if (*(SrlRXData+1) != 0x00)
		return 1;
	for (i=2, j=0; (i<100 && *(SrlRXData+i) != FEND); i++, j++) {
		if (*(SrlRXData+i) == FESC) {
			if(*(SrlRXData+i+1) == TFEND)
				FrameBuff[j]=FEND;
			else if(*(SrlRXData+i+1) == TFESC)
				FrameBuff[j]=FESC;
			else;
			i++;
		}
		else
			FrameBuff[j] = *(SrlRXData+i);
	}
#ifdef _DBG_TRACE
	trace_printf("KISS-FromHost:Ln=%d;Content=%s\r\n", j, FrameBuff);
#endif
	tx10m++;
	while(ax25->dcd == true);
	while(a->sending == true);

	// delay before transmit
	TIM2Delay(_DELAY_BASE);
	while(delay_5us != 0);
	TIM2DelayDeConfig();
	// .. end delay

	ax25_sendRaw(ax25,FrameBuff,j);
 	afsk_txStart(a);
	return 0;
}

