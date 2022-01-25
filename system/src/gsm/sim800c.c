/*
 * sim800c.c
 *
 *  Created on: Jan 18, 2022
 *      Author: mateusz
 */

#include "gsm/sim800c.h"

#include "main.h"

#include <string.h>

static const char * AUTOBAUD_STRING 			= "AT\r\0";
static const char * GET_SIGNAL_LEVEL 			= "AT+CSQ\r\0";
static const char * GET_NETWORK_REGISTRATION 	= "AT+CREG?\r\0";

static const char * OK = "OK\r\n\0";
static const char * NETWORK_REGISTRATION = "+CREG\0";

uint32_t gsm_time_of_last_command_send_to_module = 0;

// let's the library know if gsm module echoes every AT command send through serial port
uint8_t gsm_at_comm_echo = 1;

// used to receive echo and response separately
uint8_t gsm_receive_newline_counter = 0;

// first character of non-echo response from the module
uint16_t gsm_response_start_idx = 0;

//
const char * gsm_at_command_sent_last = 0;

uint8_t gsm_waiting_for_command_response = 0;

uint8_t gsm_registration_status = 4;	// unknown

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
		*state = SIM800_HANDSHAKING;

		// wait for the handshake to transmit
		srl_wait_for_tx_completion(srl_context);

		// start data reception
		srl_receive_data_with_callback(srl_context, gsm_sim800_rx_terminating_callback);

		// record when the handshake has been sent
		gsm_time_of_last_command_send_to_module = main_get_master_time();
	}
	else if (*state == SIM800_INITIALIZING && gsm_waiting_for_command_response == 0) {

		// check what command has been sent
		switch ((uint32_t)gsm_at_command_sent_last) {
			case 0: {	// no command has been send so far

				// ask for network registration status
				srl_send_data(srl_context, (const uint8_t*) GET_NETWORK_REGISTRATION, SRL_MODE_ZERO, strlen(GET_NETWORK_REGISTRATION), SRL_INTERNAL);

				// wait for command completion
				srl_wait_for_tx_completion(srl_context);

				gsm_at_command_sent_last = GET_NETWORK_REGISTRATION;

				gsm_waiting_for_command_response = 1;

				srl_receive_data_with_callback(srl_context, gsm_sim800_rx_terminating_callback);

				break;
			}
		}
	}
}

/**
 * Callback to be called just after the reception is done
 */
uint8_t gsm_sim800_rx_terminating_callback(uint8_t current_data, const uint8_t * const rx_buffer, uint16_t rx_bytes_counter) {

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

		gsm_waiting_for_command_response = 0;

		return 1;
	}

	return 0;

}

void gsm_sim800_rx_done_event_handler(srl_context_t * srl_context, gsm_sim800_state_t * state) {

	int comparision_result = 123;

	gsm_waiting_for_command_response = 0;

	// if the library expects to receive a handshake from gsm module
	if (*state == SIM800_HANDSHAKING) {
		comparision_result = strcmp(OK, (const char *)(srl_context->srl_rx_buf_pointer + gsm_response_start_idx));

		// if 'OK' has been received from the module
		if (comparision_result == 0) {
			*state = SIM800_INITIALIZING;
		}
		else {
			*state = SIM800_NOT_YET_COMM;
		}
	}
	else if (*state == SIM800_INITIALIZING) {
		comparision_result = strncmp(NETWORK_REGISTRATION, (const char *)(srl_context->srl_rx_buf_pointer + gsm_response_start_idx), 5);

		if (comparision_result == 0) {
			gsm_registration_status = atoi((const char *)(srl_context->srl_rx_buf_pointer + gsm_response_start_idx + 9));
		}
	}
}
