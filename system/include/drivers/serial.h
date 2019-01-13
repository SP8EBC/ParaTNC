#ifndef __SERIAL_H
#define __SERIAL_H

#include "stdint.h"

#define RX_BUFFER_LN 512
#define TX_BUFFER_LN 512

#define SEPARATE_RX_BUFF
#define SEPARATE_TX_BUFF

#define SRL_RX_TIMEOUT_IN_MS 1000

typedef enum srlRxState {
	SRL_RX_NOT_CONFIG,
	SRL_RX_IDLE,
	SRL_WAITING_TO_RX,
	SRL_RXING,
	SRL_RX_DONE,
	SRL_RX_ERROR
}srlRxState;

typedef enum srlTxState {
	SRL_TX_NOT_CONFIG,
	SRL_TX_IDLE,
	SRL_TXING,
	SRL_TX_ERROR
}srlTxState;

#define SRL_OK							0
#define SRL_DATA_TOO_LONG 				1
#define SRL_BUSY						2
#define SRL_WRONG_BUFFER_PARAM 			3
#define SRL_WRONG_PARAMS_COMBINATION	4

extern srlRxState srl_rx_state;
extern srlTxState srl_tx_state;
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
void srl_keep_timeout(void);
void srl_switch_timeout(uint8_t disable_enable);

#ifdef __cplusplus
}
#endif


#endif
