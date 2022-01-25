/*
 * sim800c.h
 *
 *  Created on: Jan 18, 2022
 *      Author: mateusz
 */

#ifndef INCLUDE_GSM_SIM800C_H_
#define INCLUDE_GSM_SIM800C_H_

#include "drivers/serial.h"

typedef enum gsm_sim800_state_t {
	SIM800_UNKNOWN,
	SIM800_POWERED_OFF,
	SIM800_NOT_YET_COMM,
	SIM800_HANDSHAKING,
	SIM800_INITIALIZING,
	SIM800_INITIALIZING_WAIT_RESPONSE,
	SIM800_ALIVE,
	SIM800_GPRS_CONNECTED
}gsm_sim800_state_t;

void gsm_sim800_init(gsm_sim800_state_t * state, uint8_t enable_echo);

void gsm_sim800_pool(srl_context_t * srl_context, gsm_sim800_state_t * state);
uint8_t gsm_sim800_rx_terminating_callback(uint8_t current_data, const uint8_t * const rx_buffer, uint16_t rx_bytes_counter);	// callback used to detect echo
void gsm_sim800_rx_done_event_handler(srl_context_t * srl_context, gsm_sim800_state_t * state);

#endif /* INCLUDE_GSM_SIM800C_H_ */
