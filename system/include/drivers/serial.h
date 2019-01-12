#ifndef __SERIAL_H
#define __SERIAL_H

#include "stdint.h"

#define RX_BUFFER_LN 512
#define TX_BUFFER_LN 512

#define SEPARATE_RX_BUFF
#define SEPARATE_TX_BUFF

typedef enum srlState {
	SRL_NOT_CONFIG,
	SRL_IDLE,
	SRL_WAITING_TO_RX,
	SRL_RXING,
	SRL_TXING,
	SRL_RXING_TXING,
	SRL_RX_DONE,
	SRL_ERROR
}srlState;

#define SRL_OK							0
#define SRL_DATA_TOO_LONG 				1
#define SRL_BUSY						2
#define SRL_WRONG_BUFFER_PARAM 			3
#define SRL_WRONG_PARAMS_COMBINATION	4

extern srlState srl_state;
extern uint8_t srl_tx_buffer[TX_BUFFER_LN];

#ifdef __cplusplus
extern "C" {
#endif


void srl_init(void);
uint8_t srl_send_data(uint8_t* data, uint8_t mode, uint16_t leng, uint8_t internal_external);
uint8_t srl_start_tx(short leng);
void srl_irq_handler(void);
uint8_t srl_receive_data(int num, char start, char stop, char echo, char len_addr, char len_modifier);
uint8_t* srl_get_rx_buffer();

#ifdef __cplusplus
}
#endif


#endif
