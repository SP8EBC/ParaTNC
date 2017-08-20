/* 
	Library for handling RAW mode transmission. It consist simply functions witch "pop" data from
	receiving fifo queue, detect begining and end of HDLC frames (based on standard 0x7E flag) and
	swtich do adequatly event
	
*/

#include "raw.h"

volatile bool prev_dcd_state;

unsigned char RawRXBuff[128];	// Buffer for reveiced data
FIFOBuffer RawRXFifo;

void RawFifoInit(void) {
	fifo_init(&RawRXFifo, RawRXBuff, sizeof(RawRXBuff));
}


void raw_poll(AX25Ctx *ctx, Hdlc *hdlc) {
//	int i;
	if (prev_dcd_state == true && hdlc->raw_dcd == false) {
//		i = 3;
		prev_dcd_state = false;
	}
	else {

	}
	prev_dcd_state = hdlc->raw_dcd; 
}


 

