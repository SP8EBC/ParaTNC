#ifndef _RAW_H
#define _RAW_H

#ifndef CCC
#include "stm32f10x_conf.h"
#endif

#include "ax25.h"
#include "afsk.h"
#include <stdbool.h>

#include "cfifo.h"

extern unsigned char RawRXBuff[128];
extern FIFOBuffer RawRXFifo;

void raw_poll(AX25Ctx *ctx, Hdlc *hdlc);
void RawFifoInit(void);

#endif

