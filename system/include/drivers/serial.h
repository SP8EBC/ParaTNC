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
	SRL_RXING_TXING,
	SRL_RX_DONE
}srlState;

#define SRL_OK	0
#define SRL_DATA_TOO_LONG 1
#define SRL_BUSY	2
#define SRL_WRONG_BUFFER_PARAM 3

extern srlState srl_state;

#ifdef __cplusplus
extern "C" {
#endif


void srl_init(void);
uint8_t srl_send_data(uint8_t* data, uint8_t mode, uint16_t leng, uint8_t internal_external);
uint8_t srl_start_tx(short leng);
void srl_irq_handler(void);
uint8_t srl_receive_data(int num, char start, char stop, char echo, char len_addr, char len_modifier);

#ifdef __cplusplus
}
#endif


#endif
