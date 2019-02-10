/*
 * KissCommunication.c
 *
 *  Created on: 29.04.2017
 *      Author: mateusz
 */

#include "KissCommunication.h"
//#include "main.h"

#include "drivers/serial.h"

#include "diag/Trace.h"
#include "station_config.h"
#include "TimerConfig.h"

extern unsigned short tx10m;

int32_t SendKISSToHost(uint8_t* input_frame, uint16_t input_frame_len, uint8_t* output, uint16_t output_len) {
	#define FEND	(uint8_t)0xC0
	#define FESC	(uint8_t)0xDB
	#define TFEND	(uint8_t)0xDC
	#define TFESC	(uint8_t)0xDD
	short int i /* Zmienna do poruszania siê po frame */, j /* zmienna do poruszani siê po data*/;

	if (input_frame_len >= output_len)
		return KISS_TOO_LONG_FRM;

	uint8_t* data;
	data = output;

	data[0] = FEND;
	data[1] = 0x00;
//	KissFrm.data[2] = HDLC_FLAG;
	for (j = 2, i = 0; i < input_frame_len; j++, i++) {

		// if we reach the maximu size of an output buffer
		if (j >= output_len)
			return KISS_TOO_LONG_FRM;

		if (*(input_frame+i) != FEND && *(input_frame+i) != FESC)
			data[j] = input_frame[i];
		else if (*(input_frame+i) == FEND) {
			data[j] = FESC;
			*(data + (j++)) = TFEND;
		}
		else if(*(input_frame+i) == FESC) {
			data[j] = FESC;
			*(data + (j++)) = TFESC;
		}
		else {

		}
	}
//	*(KissFrm.data + (j++)) = HDLC_FLAG;
	*(data + (j++)) = FEND;
	return j;
}

short ParseReceivedKISS(uint8_t* input_frame_from_host, uint16_t input_len, AX25Ctx* ax25, Afsk* a) {
	int i/* zmienna do poruszania sie po buforze odbiorczym usart */;
	int j/* zmienna do poruszania sie po lokalnej tablicy do przepisywania*/;
//	uint8_t FrameBuff[100];

	// to save some memory utilize the serial port buffer
	uint8_t *FrameBuff = srl_tx_buffer;

	// check if frame from host is not too long
	if (input_len >= TX_BUFFER_LN)
		return 1;

	if (*(input_frame_from_host) != FEND)
		return 1;
	if (*(input_frame_from_host+1) != 0x00)
		return 1;
	for (i=2, j=0; (i<input_len && *(input_frame_from_host+i) != FEND); i++, j++) {
		if (*(input_frame_from_host+i) == FESC) {
			if(*(input_frame_from_host+i+1) == TFEND)
				FrameBuff[j]=FEND;
			else if(*(input_frame_from_host+i+1) == TFESC)
				FrameBuff[j]=FESC;
			else;
			i++;
		}
		else
			FrameBuff[j] = *(input_frame_from_host+i);
	}
#ifdef _DBG_TRACE
	trace_printf("KISS-FromHost:Ln=%d;Content=%s\r\n", j, FrameBuff);
#endif
	tx10m++;

	// keep this commented until reseting the DCD variable will be moved outside main for (;;) loop
	//	while(ax25->dcd == true);
	while(a->sending == true);


	ax25_sendRaw(ax25,FrameBuff,j);
 	afsk_txStart(a);
	return 0;
}

