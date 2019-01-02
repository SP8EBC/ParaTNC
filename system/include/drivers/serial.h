#ifndef __SERIAL_H
#define __SERIAL_H

#include "stdint.h"

#define RX_BUFFER_LN 128
#define TX_BUFFER_LN 128

#define SEPARATE_RX_BUFF
#define SEPARATE_TX_BUFF


//extern int srlTXQueueLen;
//extern int srlTRXDataCounter;
//extern int srlTXing;
//extern int srlRXing;
//extern int srlRXBytesNum;
//extern char srlRxDummy;
//extern uint8_t srlStartChar;
//extern uint8_t srlStopChar;
//extern int srlIdle;

typedef enum srlState {
	SRL_NOT_CONFIG,
	SRL_IDLE,
	SRL_WAITING_TO_RX,
	SRL_RXING,
	SRL_TXING,
	SRL_RXING_TXING
}srlState;

extern srlState srl_state;

#ifdef __cplusplus
extern "C" {
#endif


void srl_init(void);
void srl_send_data(uint8_t* data, uint8_t mode, uint16_t leng, uint8_t internal_external);
void srl_start_tx(short leng);
void srl_irq_handler(void);
void srl_receive_data(int num, char start, char stop, char echo, char len_addr, char len_modifier);

#ifdef __cplusplus
}
#endif


#endif
