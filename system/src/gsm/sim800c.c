/*
 * sim800c.c
 *
 *  Created on: Jan 18, 2022
 *      Author: mateusz
 */

#include "gsm/sim800c.h"

#include "main.h"

#include <string.h>

static const char * AUTOBAUD_STRING = "AT\r\n\0";
static const char * OK = "OK\0";

uint32_t gsm_time_of_last_command_send_to_module = 0;

void gsm_sim800_init(gsm_sim800_state_t * state) {

	if (state != 0x00) {
		*state = SIM800_NOT_YET_COMM;
	}
}

void gsm_sim800_pool(srl_context_t * srl_context, gsm_sim800_state_t * state) {

	if (*state == SIM800_NOT_YET_COMM) {
		// send handshake
		srl_send_data(srl_context, (const uint8_t*) AUTOBAUD_STRING, SRL_MODE_ZERO, strlen(AUTOBAUD_STRING), SRL_INTERNAL);

		// switch the state
		*state = SIM800_INITIALIZIG;

		// wait for the handshake to transmit
		srl_wait_for_tx_completion(srl_context);

		// start data reception
		srl_receive_data(srl_context, 4, 0, '\r', 0, 0, 0);

		// record when the handshake has been sent
		gsm_time_of_last_command_send_to_module = main_get_master_time();
	}
}

/**
 * Callback to be called just after the reception is done
 */
void gsm_sim800_rx_done_callback(srl_context_t * srl_context, gsm_sim800_state_t * state) {

	int response_compare_res = 123;

	if (state == SIM800_INITIALIZIG) {
		// compare the response from the module
		response_compare_res = strcmp(OK, srl_context->srl_rx_buf_pointer);
	}
}
