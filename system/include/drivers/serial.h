#ifndef __SERIAL_H
#define __SERIAL_H

#include "stdint.h"
#ifdef STM32F10X_MD_VL
#include <stm32f10x.h>
#include <stm32f10x_usart.h>
#endif
#ifdef STM32L471xx
#include <stm32l4xx.h>
#include <stm32l4xx_ll_usart.h>
#endif

#define RX_BUFFER_1_LN 384
#define TX_BUFFER_1_LN 384

#define RX_BUFFER_2_LN 96
#define TX_BUFFER_2_LN 96

#define RX_BUFFER_3_LN 768
#define TX_BUFFER_3_LN 768

#define SEPARATE_RX_BUFF
#define SEPARATE_TX_BUFF

#define SRL_TX_DELAY_IN_MS	30
#define SRL_DEFAULT_RX_TIMEOUT_IN_MS 1200

#define SRL_TIMEOUT_ENABLE	1
#define SRL_TIMEOUT_DISABLE 0

#define SRL_INTERNAL 0
#define SRL_EXTERNAL 1

#define SRL_MODE_ZERO	0
#define SRL_MODE_DEFLN	1

#define SRL_NO_START_CHR	0
#define SRL_NO_STOP_CHR		0

#define SRL_ECHO_ENABLE		1
#define SRL_ECHO_DISABLE	0

/**
 * Callback definition which may be called after receiving each byte by an UART
 *
 * @param current_data	received byte
 * @param rx_bytes_counter receive bytes counter incremented AFTER this callback is processed so it's point to the byte which triggered this call
 */
typedef uint8_t(*srl_rx_termination_callback_t)(uint8_t current_data, const uint8_t * const rx_buffer, uint16_t rx_bytes_counter);

typedef enum srlRxState {
	SRL_RX_NOT_CONFIG,
	SRL_RX_IDLE,
	SRL_WAITING_TO_RX,
	SRL_RXING,
	SRL_RX_DONE,
	SRL_RX_ERROR
}srl_rx_state_t;

typedef enum srlTxState {
	SRL_TX_NOT_CONFIG,
	SRL_TX_IDLE,
	SRL_TX_WAITING,
	SRL_TXING,
	SRL_TX_ERROR
}srl_tx_state_t;

typedef enum srl_timeout_mode {
	SRL_TIMEOUT_OFF,
	SRL_TIMEOUT_INSTANT,
	SRL_TIMEOUT_TRIGGERED
}srl_timeout_mode_t;

typedef enum srl_error_reason {
	SRL_ERR_NONE,
	SRL_ERR_TIMEOUT_WAITING,
	SRL_ERR_TIMEOUT_RECEIVING,
	SRL_ERR_OVERFLOW
} srl_error_reason_t;

typedef struct srl_context_t {
	USART_TypeDef *port;

	uint32_t port_baurate;
	uint8_t port_stopbits;

	GPIO_TypeDef* te_port;
	uint16_t te_pin;

	// these pointers points to either internal buffer
	// or external provided by caller, depends on 'uint8_t internal_external'
	// parameter in send functions
	uint8_t *srl_tx_buf_pointer;
	uint8_t *srl_rx_buf_pointer;

	// thatt pointer always points to internal buffer
	uint8_t *srl_tx_internal_buf_pointer;

	uint16_t srl_rx_buf_ln;
	uint16_t srl_tx_buf_ln;

	uint16_t srl_tx_internal_buf_ln;

	uint16_t srl_rx_bytes_counter;
	uint16_t srl_tx_bytes_counter;

	uint16_t srl_rx_bytes_req;
	uint16_t srl_tx_bytes_req;

	uint8_t srl_triggered_start;
	uint8_t srl_triggered_stop;

	uint8_t srl_start_trigger;
	uint8_t srl_stop_trigger;

	// temporary storage for reading the 'DR' register during waiting for the first byte
	// of the protocol data to be received
	volatile uint8_t srl_garbage_storage;

	uint8_t srl_rx_timeout_enable;
	uint8_t srl_rx_timeout_waiting_enable;
	uint8_t srl_rx_timeout_calc_started;
	uint8_t srl_rx_timeout_waiting_calc_started;
	uint32_t srl_rx_timeout_trigger_value_in_msec;
	uint32_t srl_rx_start_time;
	uint32_t srl_rx_waiting_start_time;

	// this is a time when srl_start_tx or srl_send_data initiated the transfer. If this is set to 0xFFFFFFFF
	// the transmission will start immediately. If it is set to any other value the transmission
	// (first and any consecutive byte) will be delayed by value defined by 'SRL_TX_DELAY_IN_MS'.
	//
	// This is a workaround of garbage occurring on RS485 bus when TE/RE pin of MAX485 is switched.
	// A delay will separate this artificial 0xFF (when TE/RE are switched) from the rest of data
	// what should make Modbus-RTU working again -> this 0xFF will be ignored.
	uint32_t srl_tx_start_time;

	srl_rx_state_t srl_rx_state;
	srl_tx_state_t srl_tx_state;

	srl_error_reason_t srl_rx_error_reason;
	uint8_t total_idle_counter;
	uint8_t total_overrun_counter;

	uint8_t srl_enable_echo;

	uint8_t srl_rx_lenght_param_addres;
	uint8_t srl_rx_lenght_param_modifier;

	// this is a pointer to function which could be optionally called
	// after each byte received by the serial port
	// the value returned by this function determines if the receiving shall
	// be continued (if returned 0) or not (if returned 1)
	srl_rx_termination_callback_t srl_rx_term;

	// counters of how many bytes was received and transmitted. This actually
	// counts how many times the interrupt has been raised because of TXE or RXNE
	uint32_t total_rx_bytes;
	uint32_t total_tx_bytes;

}srl_context_t;


#define SRL_UNINITIALIZED				127
#define SRL_OK							0
#define SRL_DATA_TOO_LONG 				1
#define SRL_BUSY						2
#define SRL_WRONG_BUFFER_PARAM 			3
#define SRL_WRONG_PARAMS_COMBINATION	4
#define SRL_TIMEOUT						5

#ifdef SEPARATE_TX_BUFF
extern uint8_t srl_usart1_tx_buffer[TX_BUFFER_1_LN];
#endif

#ifdef SEPARATE_RX_BUFF
extern uint8_t srl_usart1_rx_buffer[RX_BUFFER_1_LN];
#endif

#ifdef SEPARATE_TX_BUFF
extern uint8_t srl_usart2_tx_buffer[TX_BUFFER_2_LN];
#endif

#ifdef SEPARATE_RX_BUFF
extern uint8_t srl_usart2_rx_buffer[RX_BUFFER_2_LN];
#endif

#ifdef PARAMETEO
extern uint8_t srl_usart3_tx_buffer[TX_BUFFER_3_LN];
extern uint8_t srl_usart3_rx_buffer[RX_BUFFER_3_LN];
#endif


#ifdef __cplusplus
extern "C" {
#endif


void srl_init(srl_context_t *ctx, USART_TypeDef *port, uint8_t *rx_buffer, uint16_t rx_buffer_size, uint8_t *tx_buffer, uint16_t tx_buffer_size, uint32_t baudrate, uint8_t stop_bits);
void srl_reset(srl_context_t *ctx);
void srl_close(srl_context_t *ctx);
uint8_t srl_send_data(srl_context_t *ctx, const uint8_t* data, uint8_t mode, uint16_t leng, uint8_t internal_external);
uint8_t srl_start_tx(srl_context_t *ctx, short leng);
void srl_wait_for_tx_completion(srl_context_t *ctx);
uint8_t srl_wait_for_rx_completion_or_timeout(srl_context_t *ctx, uint8_t* output);
void srl_irq_handler(srl_context_t *ctx);
uint8_t srl_receive_data(srl_context_t *ctx, int num, char start, char stop, char echo, char len_addr, char len_modifier);
uint8_t srl_receive_data_with_instant_timeout(srl_context_t *ctx, int num, char start, char stop, char echo, char len_addr, char len_modifier);
uint8_t srl_receive_data_with_callback(srl_context_t *ctx, srl_rx_termination_callback_t cbk);
uint16_t srl_get_num_bytes_rxed(srl_context_t *ctx);
uint8_t* srl_get_rx_buffer(srl_context_t *ctx);
void srl_keep_tx_delay(srl_context_t *ctx);
void srl_switch_tx_delay(srl_context_t *ctx, uint8_t disable_enable);
void srl_keep_timeout(srl_context_t *ctx);
void srl_switch_timeout(srl_context_t *ctx, uint8_t disable_enable, uint32_t value);
void srl_switch_timeout_for_waiting(srl_context_t *ctx, uint8_t disable_enable);

#ifdef __cplusplus
}
#endif


#endif
