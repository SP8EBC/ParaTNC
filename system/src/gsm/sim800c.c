/*
 * sim800c.c
 *
 *  Created on: Jan 18, 2022
 *      Author: mateusz
 */

#include "gsm/sim800c.h"

#include "main.h"

#include <string.h>

static const char * AUTOBAUD_STRING = "AT\r\0";
static const char * OK = "OK\0";

uint32_t gsm_time_of_last_command_send_to_module = 0;

// let's the library know if gsm module echoes every AT command send through serial port
uint8_t gsm_at_comm_echo = 1;

// used to receive echo and response separately
uint8_t gsm_receive_newline_counter = 0;

// first character of non-echo response from the module
uint16_t gsm_response_start_idx = 0;

void gsm_sim800_init(gsm_sim800_state_t * state, uint8_t enable_echo) {

	gsm_at_comm_echo = enable_echo;

	gsm_response_start_idx = 0;

	if (state != 0x00) {
		*state = SIM800_NOT_YET_COMM;
	}
}

void gsm_sim800_pool(srl_context_t * srl_context, gsm_sim800_state_t * state) {

	if (*state == SIM800_NOT_YET_COMM) {

		// configure rx timeout
		srl_switch_timeout(srl_context, 1, 0);

		// send handshake
		srl_send_data(srl_context, (const uint8_t*) AUTOBAUD_STRING, SRL_MODE_ZERO, strlen(AUTOBAUD_STRING), SRL_INTERNAL);

		// switch the state
		*state = SIM800_INITIALIZIG;

		// wait for the handshake to transmit
		srl_wait_for_tx_completion(srl_context);

		// start data reception
		//srl_receive_data(srl_context, 8, SRL_NO_START_CHR, SRL_NO_STOP_CHR, SRL_ECHO_DISABLE, 0, 0);
		srl_receive_data_with_callback(srl_context, gsm_sim800_rx_callback);

		// record when the handshake has been sent
		gsm_time_of_last_command_send_to_module = main_get_master_time();
	}
}

/**
 * Callback to be called just after the reception is done
 */
uint8_t gsm_sim800_rx_callback(uint8_t current_data, const uint8_t * const rx_buffer, uint16_t rx_bytes_counter) {

	char current = (char) current_data;

	char before = '\0';

	if (rx_bytes_counter > 0) {
		before = (char) *(rx_buffer + rx_bytes_counter - 1);
	}

	// check what character has been received
	if (current == '\n') {
		// increase newline counter
		gsm_receive_newline_counter++;
	}

	// check if this is first character of response
	if (current != '\n' && current != '\r' && (before == '\n' || before == '\r')) {
		gsm_response_start_idx = rx_bytes_counter;
	}

	// if an echo is enabled and second newline has been received
	if (gsm_at_comm_echo == 1 && gsm_receive_newline_counter > 1 && gsm_response_start_idx > 0) {

		gsm_receive_newline_counter = 0;

		return 1;
	}

	return 0;

}
