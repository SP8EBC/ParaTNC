/*
 * sim800c_tcpip.h
 *
 *  Created on: Feb 17, 2022
 *      Author: mateusz
 */

#ifndef INCLUDE_GSM_SIM800C_TCPIP_H_
#define INCLUDE_GSM_SIM800C_TCPIP_H_

#define TEST_IP 	"euro.aprs2.net\0"
#define TEST_PORT	"14580\0"

#include <stdint.h>

#include "drivers/serial.h"

#include "gsm/sim800_return_t.h"
#include "gsm/sim800_state_t.h"

extern const char * TCP2;
extern const char * TCP3;
extern const char * TCP4;

typedef void(*gsm_sim800_tcpip_receive_callback_t)(srl_context_t*);

/**
 * Starts TCP connection or UDP session
 * @param ip_or_dns_address pointer to a string with DNS hostname or IP address
 * @param address_ln length of a buffer with an address
 * @param port pointer to a string with port number
 * @param port_ln length of a buffer with port number
 * @param srl_context pointer to serial context struct
 * @param state pointer to an enum with GSM module state
 * @param tcp_or_udp zero if TCP connection shall be established, non zero value for udp
 * @return
 */
sim800_return_t gsm_sim800_tcpip_connect(char * ip_or_dns_address, uint8_t address_ln, char * port, uint8_t port_ln, srl_context_t * srl_context, gsm_sim800_state_t * state, uint8_t tcp_or_udp);

/**
 *
 * @param srl_context
 * @param state
 * @param rx_callback
 * @param timeout
 * @param rx_done_callback
 * @return
 */
sim800_return_t gsm_sim800_tcpip_async_receive(srl_context_t * srl_context, gsm_sim800_state_t * state, srl_rx_termination_callback_t rx_callback, uint32_t timeout, gsm_sim800_tcpip_receive_callback_t rx_done_callback);

/**
 *
 * @param buffer
 * @param buffer_size
 * @param srl_context
 * @param state
 * @param rx_callback
 * @param timeout
 * @return
 */
sim800_return_t gsm_sim800_tcpip_receive(uint8_t * buffer, uint16_t buffer_size, srl_context_t * srl_context, gsm_sim800_state_t * state, srl_rx_termination_callback_t rx_callback, uint32_t timeout);

/**
 *
 * @param data
 * @param data_len
 * @param srl_context
 * @param state
 * @return
 */
sim800_return_t gsm_sim800_tcpip_async_write(uint8_t * data, uint16_t data_len, srl_context_t * srl_context, gsm_sim800_state_t * state);

/**
 *
 * @param data
 * @param data_len
 * @param srl_context
 * @param state
 * @return
 */
sim800_return_t gsm_sim800_tcpip_write(uint8_t * data, uint16_t data_len, srl_context_t * srl_context, gsm_sim800_state_t * state);

/**
 * Closes established TCP connection
 * @param srl_context	pointer to serial context used to communication with gprs module
 * @param state
 * @param force			force changing internal connection state even if there
 * 						were problems with a response to diconnection AT command.
 * @return	SIM800_OK connection was closed successfully
 *			SIM800_TCP_CLOSE_ALREADY connection has been closed in the meantime by remote server
 *			SIM800_TCP_CLOSE_UNCERTAIN no valid response was received from gprs module on disconnect
 *request SIM800_WRONG_STATE_TO_CLOSE no connection has been
 */
sim800_return_t gsm_sim800_tcpip_close(srl_context_t * srl_context, gsm_sim800_state_t * state, uint8_t force);

void gsm_sim800_tcpip_rx_done_callback(srl_context_t * srl_context, gsm_sim800_state_t * state);

/**
 * Callback used from serial port context, to notify that a transmission during active TCP
 * connection is done Please note that it doesn't mean that a TCP connection is done, so
 * still a care must be taken, not to mix data from simultaneous connections!
 * A code defined under a macro @link{GSM_TCPIP_TX_DONE_CALLBACK} might be put inside
 * a body of this function. This can be used in RTOS environment, to release shared
 * resource
 * @param srl_context
 * @param state
 */
void gsm_sim800_tcpip_tx_done_callback (srl_context_t *srl_context, gsm_sim800_state_t *state);

/**
 * If macro @link{GSM_TCPIP_TX_BUSY_CALLBACK} is not defined it simply returns a value of
 * @link{gsm_sim800_tcpip_transmitting}. The macro is optionally put before return
 * statement, what might be used in multitasking environment to signalize a mutex, to protect
 * a shared resource
 * @return value of tcpip_transmitting
 */
uint8_t gsm_sim800_tcpip_tx_busy (void);

uint8_t gsm_sim800_newline_terminating_callback(uint8_t current_data, const uint8_t * const rx_buffer, uint16_t rx_bytes_counter);

void gsm_sim800_tcpip_reset(void);

// uint8_t current_data, const uint8_t * const rx_buffer, uint16_t rx_bytes_counter

#endif /* INCLUDE_GSM_SIM800C_TCPIP_H_ */
