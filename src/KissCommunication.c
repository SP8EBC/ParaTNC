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

#include <crc.h>

#include <string.h>

extern unsigned short tx10m;


uint8_t kiss_buffer[KISS_BUFFER_LN];

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

	uint8_t *FrameBuff = kiss_buffer;

	// check if frame from host is not too long
	if (input_len >= KISS_BUFFER_LN)
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

void kiss_reset_buffer(uint8_t* output, uint16_t output_len, uint16_t* current_len) {
	memset(output, 0x00, sizeof(output_len));

	output[0] = FEND;
	output[1] = 0x00;

	*current_len = 2;
}

uint8_t kiss_put_char(uint8_t c, uint8_t* output, uint16_t output_len, uint16_t* current_len, uint16_t* crc) {

	uint16_t new_crc = 0;

	if (*current_len >= output_len) {
		return 1;
	}

	if (c == HDLC_FLAG || c == HDLC_RESET || c == AX25_ESC)
	{
		kiss_put_char(AX25_ESC, output, output_len, current_len, crc);
	}

	if (c == FEND)
	{
		kiss_put_char(FESC, output, output_len, current_len, crc);
		kiss_put_char(TFEND, output, output_len, current_len, crc);
	}

	else if (c == FESC)
	{
		kiss_put_char(FESC, output, output_len, current_len, crc);
		kiss_put_char(TFESC, output, output_len, current_len, crc);
	}

	else {
		output[*current_len++] = c;
	}

	if (crc == NULL) {
		;
	}
	else {
		new_crc = updcrc_ccitt(c, *crc);

		*crc = new_crc;
	}

	return 0;
}

void kiss_put_call(const AX25Call *addr, uint8_t is_last, uint8_t* output, uint16_t output_len, uint16_t* current_len, uint16_t* crc) {

	uint16_t i;
	uint8_t ssid;
	uint16_t len = MIN(sizeof(addr->call), strlen(addr->call));


	for (i = 0; i < len; i++)
	{
		uint8_t c = addr->call[i];
		kiss_put_char(c << 1, output, output_len, current_len, crc);
	}

	if (len < sizeof(addr->call))
	{
		for (i = 0; i < sizeof(addr->call) - len; i++)
		{
			kiss_put_char(' ' << 1, output, output_len, current_len, crc);
		}
	}

	ssid = 0x60 | (addr->ssid << 1) | (is_last ? 0x01 : 0);
	kiss_put_char(ssid, output, output_len, current_len, crc);

}

void kiss_finalize_buffer(uint8_t* output, uint16_t output_len, uint16_t* current_len) {
	output[*current_len++] = FEND;
}

uint8_t* kiss_get_buff_ptr(void) {
	return kiss_buffer;
}


