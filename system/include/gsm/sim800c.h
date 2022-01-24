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
	SIM800_INITIALIZIG,
	SIM800_ALIVE,
	SIM800_GPRS_CONNECTED
}gsm_sim800_state_t;

void gsm_sim800_init(gsm_sim800_state_t * state);

void gsm_sim800_pool(srl_context_t * srl_context, gsm_sim800_state_t * state);
void gsm_sim800_rx_done_callback(srl_context_t * srl_context, gsm_sim800_state_t * state);

#endif /* INCLUDE_GSM_SIM800C_H_ */
