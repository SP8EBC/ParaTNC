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

sim800_return_t gsm_sim800_tcpip_connect(char * ip_or_dns_address, uint8_t address_ln, char * port, uint8_t port_ln, srl_context_t * srl_context, gsm_sim800_state_t * state);
sim800_return_t gsm_sim800_tcpip_async_receive(srl_context_t * srl_context, gsm_sim800_state_t * state, srl_rx_termination_callback_t rx_callback, uint32_t timeout, gsm_sim800_tcpip_receive_callback_t rx_done_callback);
sim800_return_t gsm_sim800_tcpip_receive(uint8_t * buffer, uint16_t buffer_size, srl_context_t * srl_context, gsm_sim800_state_t * state, srl_rx_termination_callback_t rx_callback, uint32_t timeout);
sim800_return_t gsm_sim800_tcpip_async_write(uint8_t * data, uint16_t data_len, srl_context_t * srl_context, gsm_sim800_state_t * state);
sim800_return_t gsm_sim800_tcpip_write(uint8_t * data, uint16_t data_len, srl_context_t * srl_context, gsm_sim800_state_t * state);
sim800_return_t gsm_sim800_tcpip_close(srl_context_t * srl_context, gsm_sim800_state_t * state, uint8_t force);

void gsm_sim800_tcpip_rx_done_callback(srl_context_t * srl_context, gsm_sim800_state_t * state);
void gsm_sim800_tcpip_tx_done_callback(srl_context_t * srl_context, gsm_sim800_state_t * state);

uint8_t gsm_sim800_newline_terminating_callback(uint8_t current_data, const uint8_t * const rx_buffer, uint16_t rx_bytes_counter);

void gsm_sim800_tcpip_reset(void);

// uint8_t current_data, const uint8_t * const rx_buffer, uint16_t rx_bytes_counter

#endif /* INCLUDE_GSM_SIM800C_TCPIP_H_ */
