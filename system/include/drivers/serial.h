#ifndef __SERIAL_H
#define __SERIAL_H

#include "stdint.h"

extern uint8_t srlTXData[128];
extern uint8_t srlRXData[128];
extern int srlTXQueueLen;
extern int srlTRXDataCounter;
extern int srlTXing;
extern int srlRXing;
extern int srlRXBytesNum;
extern char srlRxDummy;
extern uint8_t srlStartChar;
extern uint8_t srlStopChar;
extern int srlIdle;

#ifdef __cplusplus
extern "C" {
#endif

void SrlConfig(void);
void SrlSendData(char* data, char mode, short leng);
void SrlStartTX(short leng);
void SrlIrqHandler(void);
//void USART1_IRQHandler(void);
void SrlReceiveData(int num, char start, char stop, char echo, char len_addr, char len_modifier);

#ifdef __cplusplus
}
#endif


#endif
