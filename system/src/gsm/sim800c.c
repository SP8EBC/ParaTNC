/*
 * sim800c.c
 *
 *  Created on: Jan 18, 2022
 *      Author: mateusz
 */

#include "gsm/sim800c.h"
#include "gsm/sim800c_engineering.h"
#include "gsm/sim800c_gprs.h"
#include "gsm/sim800c_inline.h"

#include "main.h"

#include <string.h>
#include <stdlib.h>

#define SIM800_DEFAULT_TIMEOUT 250	// in miliseconds

static const char * AUTOBAUD_STRING 			= "AT\r\0";
static const char * GET_SIGNAL_LEVEL 			= "AT+CSQ\r\0";
static const char * GET_NETWORK_REGISTRATION 	= "AT+CREG?\r\0";
static const char * GET_PIN_STATUS 				= "AT+CPIN?\r\0";
static const char * GET_REGISTERED_NETWORK		= "AT+COPS?\r\0";
extern const char * START_CONFIG_APN;


static const char * OK = "OK\r\n\0";
static const char * SIGNAL_LEVEL = "+CSQ:\0";
static const char * NETWORK_REGISTRATION = "+CREG:\0";
static const char * CPIN = "+CPIN:\0";
static const char * CPIN_READY = "READY";
static const char * CPIN_SIMPIN = "SIMPIN";
static const char * REGISTERED_NETWORK = "+COPS:\0";
static const char * INCOMING_CALL = "RING\0";
#define INCOMING_CALL_LN 4
static const char * NOCARRIER = "NO CARRIER\0";
#define NOCARRIER_LN 10
static const char * UVP_PDOWN = "UNDER-VOLTAGE POWER DOWN\0";
#define UVP_PDOWN_LN 24
static const char * UVP_WARNING = "UNDER-VOLTAGE WARNNING\0";
#define UVP_WARNING_LN 22
static const char * OVP_PDWON = "OVER-VOLTAGE POWER DOWN\0";
#define IVP_PDWON_LN 23
static const char * OVP_WARNING = "OVER-VOLTAGE WARNNING\0";
#define OVP_WARNING_LN 21
static const char * CALL_RDY = "Call Ready\0";
#define CALL_RDY_LN 10
static const char * SMS_RDY = "SMS Ready\0";
#define SMS_RDY_LN 9

uint32_t gsm_time_of_last_command_send_to_module = 0;

// let's the library know if gsm module echoes every AT command send through serial port
static uint8_t gsm_at_comm_echo = 1;

// how many newlines
volatile static int8_t gsm_terminating_newline_counter = 1;

// used to receive echo and response separately
static uint8_t gsm_receive_newline_counter = 0;

// first character of non-echo response from the module
static uint16_t gsm_response_start_idx = 0;

// a pointer to the last command string which sent in SIM800_INITIALIZING state
const char * gsm_at_command_sent_last = 0;

// set to one to lock 'gsm_sim800_pool' in SIM800_INITIALIZING state until the response is received
static uint8_t gsm_waiting_for_command_response = 0;

uint8_t gsm_sim800_registration_status = 4;	// unknown

// string with sim status
char gsm_sim800_sim_status[10];

char gsm_sim800_registered_network[16];

int8_t gsm_sim800_signal_level_dbm = 0;

float gsm_sim800_bcch_frequency = 0;

char gsm_sim800_cellid[5] = {0, 0, 0, 0, 0};

char gsm_sim800_lac[5] = {0, 0, 0, 0, 0};

inline static void gsm_sim800_replace_non_printable_with_space(char * str) {
	for (int i = 0; *(str + i) != 0 ; i++) {
		char current = *(str + i);

		if (current != 0x00) {
			if (current < 0x21 || current > 0x7A) {
				*(str + i) = ' ';
			}
		}
	}
}


void gsm_sim800_check_for_async_messages(uint8_t * ptr, uint16_t size, uint16_t * offset) {
	// offset is a pointer to variable where this function will store a position of first response character
	// after the async message


	int comparision_result = 123;

	// simplified check, not to waste time for full strncmp
	if (*ptr == 'R') {
		comparision_result = strncmp(INCOMING_CALL, (const char *)ptr, (size_t)INCOMING_CALL_LN);
	}
	else if (*ptr == 'N') {
		comparision_result = strncmp(NOCARRIER, (const char *)ptr, (size_t)NOCARRIER_LN);
	}
	else if (*ptr == 'S') {
		comparision_result = strncmp(SMS_RDY, (const char *)ptr, (size_t)SMS_RDY_LN);
	}
	else if (*ptr == 'C') {
		comparision_result = strncmp(CALL_RDY, (const char *)ptr, (size_t)CALL_RDY_LN);
	}
	else if (*ptr == 'O') {
		comparision_result = strncmp(OVP_WARNING, (const char *)ptr, (size_t)OVP_WARNING_LN);

		if (comparision_result != 0) {
			comparision_result = strncmp(OVP_PDWON, (const char *)ptr, (size_t)IVP_PDWON_LN);
		}
	}
	else if (*ptr == 'U') {
		comparision_result = strncmp(UVP_WARNING, (const char *)ptr, (size_t)UVP_WARNING_LN);

		if (comparision_result != 0) {
			comparision_result = strncmp(UVP_PDOWN, (const char *)ptr, (size_t)UVP_PDOWN_LN);
		}
	}

	// check if this has been found
	if (comparision_result == 0) {
		// if yes rewind to the start of response
		for (int i = INCOMING_CALL_LN; i < size && *(ptr + i) != 0; i++) {
			if (*(ptr + i) > 0x2A && *(ptr + i) < 0x5B) {
				// start the check from '+' and end on 'Z'
				*offset = (uint16_t)i;

				break;
			}
		}
	}

}

uint32_t gsm_sim800_check_for_extra_newlines(uint8_t * ptr, uint16_t size) {

	// this bitmask stores positions of first four lines of text in input buffer
	// the position value is set to 0xFF if a position of the newline is beyond 255 offset
	uint32_t output_bitmask = 0;

	int8_t newlines = 0;

	int i = 0;

	char current, previous;

	current = (char)*ptr;

	for (i = 0; (i < size) && *(ptr + i) != 0; i++) {

		previous = current;

		current = (char)*(ptr + i);

		if (previous == '\n' && current >= 0x20 && current < 0x7F) {

			output_bitmask |= ((uint8_t)i << (8 * newlines));

			newlines++;
		}


		if (newlines > 3) {
			break;
		}
	}

	if (newlines < 4 && (*(ptr + i - 1) == '\n' || *(ptr + i) == '\n')) {
		output_bitmask |= ((uint8_t)i << (8 * newlines));

	}

	return output_bitmask;
}

uint8_t gsm_sim800_get_waiting_for_command_response(void) {
	return gsm_waiting_for_command_response;
}

void gsm_sim800_init(gsm_sim800_state_t * state, uint8_t enable_echo) {

	gsm_at_comm_echo = enable_echo;

	gsm_response_start_idx = 0;

	if (state != 0x00) {
		*state = SIM800_NOT_YET_COMM;
	}
}

void gsm_sim800_initialization_pool(srl_context_t * srl_context, gsm_sim800_state_t * state) {

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

		// start timeout calculation
		srl_context->srl_rx_timeout_calc_started = 1;

		// record when the handshake has been sent
		gsm_time_of_last_command_send_to_module = main_get_master_time();

		gsm_at_command_sent_last = 0;
	}
	else if (*state == SIM800_INITIALIZING && gsm_waiting_for_command_response == 0) {

		// check what command has been sent
		//switch ((uint32_t)gsm_at_command_sent_last) {
		if (gsm_at_command_sent_last == 0) {
			// no command has been send so far

			// ask for network registration status
			srl_send_data(srl_context, (const uint8_t*) GET_NETWORK_REGISTRATION, SRL_MODE_ZERO, strlen(GET_NETWORK_REGISTRATION), SRL_INTERNAL);

			// wait for command completion
			srl_wait_for_tx_completion(srl_context);

			gsm_at_command_sent_last = GET_NETWORK_REGISTRATION;

			gsm_waiting_for_command_response = 1;

			srl_receive_data_with_callback(srl_context, gsm_sim800_rx_terminating_callback);

			// start timeout calculation
			srl_context->srl_rx_timeout_calc_started = 1;

			// record when the command has been sent
			gsm_time_of_last_command_send_to_module = main_get_master_time();

		}
		else if (gsm_at_command_sent_last == GET_NETWORK_REGISTRATION) {
				// ask for network registration status
				srl_send_data(srl_context, (const uint8_t*) GET_PIN_STATUS, SRL_MODE_ZERO, strlen(GET_PIN_STATUS), SRL_INTERNAL);

				// wait for command completion
				srl_wait_for_tx_completion(srl_context);

				gsm_at_command_sent_last = GET_PIN_STATUS;

				gsm_waiting_for_command_response = 1;

				srl_receive_data_with_callback(srl_context, gsm_sim800_rx_terminating_callback);

				// start timeout calculation
				srl_context->srl_rx_timeout_calc_started = 1;

				// record when the command has been sent
				gsm_time_of_last_command_send_to_module = main_get_master_time();

		}
		else if (gsm_at_command_sent_last == GET_PIN_STATUS) {
			// ask for network registration status
			srl_send_data(srl_context, (const uint8_t*) GET_REGISTERED_NETWORK, SRL_MODE_ZERO, strlen(GET_REGISTERED_NETWORK), SRL_INTERNAL);

			// wait for command completion
			srl_wait_for_tx_completion(srl_context);

			gsm_at_command_sent_last = GET_REGISTERED_NETWORK;

			gsm_waiting_for_command_response = 1;

			srl_receive_data_with_callback(srl_context, gsm_sim800_rx_terminating_callback);

			// start timeout calculation
			srl_context->srl_rx_timeout_calc_started = 1;

			// record when the command has been sent
			gsm_time_of_last_command_send_to_module = main_get_master_time();
		}
		else if (gsm_at_command_sent_last == GET_REGISTERED_NETWORK) {
			// ask for signal level
			srl_send_data(srl_context, (const uint8_t*) GET_SIGNAL_LEVEL, SRL_MODE_ZERO, strlen(GET_SIGNAL_LEVEL), SRL_INTERNAL);

			// wait for command completion
			srl_wait_for_tx_completion(srl_context);

			gsm_at_command_sent_last = GET_SIGNAL_LEVEL;

			gsm_waiting_for_command_response = 1;

			srl_receive_data_with_callback(srl_context, gsm_sim800_rx_terminating_callback);

			// start timeout calculation
			srl_context->srl_rx_timeout_calc_started = 1;

			// record when the command has been sent
			gsm_time_of_last_command_send_to_module = main_get_master_time();
		}
		else if (gsm_at_command_sent_last == GET_SIGNAL_LEVEL) {
			*state = SIM800_ALIVE;

			gsm_at_command_sent_last = 0;
		}
	}
	else if (*state == SIM800_INITIALIZING_GPRS  && gsm_waiting_for_command_response == 0) {
		// do not
		if (gsm_at_command_sent_last == 0) {
			srl_send_data(srl_context, (const uint8_t*) SHUTDOWN_GPRS, SRL_MODE_ZERO, strlen(SHUTDOWN_GPRS), SRL_INTERNAL);

			// wait for command completion
			srl_wait_for_tx_completion(srl_context);

			gsm_at_command_sent_last = SHUTDOWN_GPRS;

			gsm_waiting_for_command_response = 1;

			srl_receive_data_with_callback(srl_context, gsm_sim800_rx_terminating_callback);

			// 'AT+CIPSHUT' has maximum response time of 65 seconds
			srl_switch_timeout(srl_context, 1, 11000);

			// start timeout calculation
			srl_context->srl_rx_timeout_calc_started = 1;

			// record when the command has been sent
			gsm_time_of_last_command_send_to_module = main_get_master_time();
		}
		else if (gsm_at_command_sent_last == SHUTDOWN_GPRS) {
			// create GPRS APN configuration string
			sim800_gprs_create_apn_config_str((char * )srl_context->srl_tx_buf_pointer, srl_context->srl_tx_buf_ln);

			// send created data to GSM module
			//srl_send_data(srl_context, srl_context->srl_tx_buf_pointer, SRL_MODE_ZERO, strlen((const char *)srl_context->srl_tx_buf_pointer), SRL_EXTERNAL);
			srl_start_tx(srl_context, strlen((const char*) srl_context->srl_tx_buf_pointer));

			// wait for command completion
			srl_wait_for_tx_completion(srl_context);

			gsm_at_command_sent_last = START_CONFIG_APN;

			gsm_waiting_for_command_response = 1;

			srl_receive_data_with_callback(srl_context, gsm_sim800_rx_terminating_callback);

			srl_switch_timeout(srl_context, 1, SIM800_DEFAULT_TIMEOUT);

			// start timeout calculation
			srl_context->srl_rx_timeout_calc_started = 1;

			// record when the command has been sent
			gsm_time_of_last_command_send_to_module = main_get_master_time();
		}
		else if (gsm_at_command_sent_last == START_CONFIG_APN) {

			srl_send_data(srl_context, (const uint8_t*) START_GPRS, SRL_MODE_ZERO, strlen(START_GPRS), SRL_INTERNAL);

			// wait for command completion
			srl_wait_for_tx_completion(srl_context);

			gsm_at_command_sent_last = START_GPRS;

			gsm_waiting_for_command_response = 1;

			srl_receive_data_with_callback(srl_context, gsm_sim800_rx_terminating_callback);

			// starting GPRS session has maximum response time of 65 seconds
			srl_switch_timeout(srl_context, 1, 15000);		// TODO

			// start timeout calculation
			srl_context->srl_rx_timeout_calc_started = 1;

			// record when the command has been sent
			gsm_time_of_last_command_send_to_module = main_get_master_time();

		}
		else if (gsm_at_command_sent_last == START_GPRS) {

			srl_send_data(srl_context, (const uint8_t*) GET_IP_ADDRESS, SRL_MODE_ZERO, strlen(GET_IP_ADDRESS), SRL_INTERNAL);

			// wait for command completion
			srl_wait_for_tx_completion(srl_context);

			gsm_at_command_sent_last = GET_IP_ADDRESS;

			gsm_waiting_for_command_response = 1;

			srl_receive_data_with_callback(srl_context, gsm_sim800_rx_terminating_callback);

			// reverting back to default timeout
			srl_switch_timeout(srl_context, 1, 0);

			// start timeout calculation
			srl_context->srl_rx_timeout_calc_started = 1;

			// record when the command has been sent
			gsm_time_of_last_command_send_to_module = main_get_master_time();

//			srl_send_data(srl_context, (const uint8_t*) ENABLE_EDGE, SRL_MODE_ZERO, strlen(ENABLE_EDGE), SRL_INTERNAL);
//
//			// wait for command completion
//			srl_wait_for_tx_completion(srl_context);
//
//			gsm_at_command_sent_last = ENABLE_EDGE;
//
//			gsm_waiting_for_command_response = 1;
//
//			srl_receive_data_with_callback(srl_context, gsm_sim800_rx_terminating_callback);
//
//			// this command has standard response time
//			srl_switch_timeout(srl_context, 1, 0);
//
//			// start timeout calculation
//			srl_context->srl_rx_timeout_calc_started = 1;
//
//			// record when the command has been sent
//			gsm_time_of_last_command_send_to_module = main_get_master_time();
		}
		else if (gsm_at_command_sent_last == GET_IP_ADDRESS) {
			srl_send_data(srl_context, (const uint8_t*) GET_CONNECTION_STATUS, SRL_MODE_ZERO, strlen(GET_CONNECTION_STATUS), SRL_INTERNAL);

			// wait for command completion
			srl_wait_for_tx_completion(srl_context);

			gsm_at_command_sent_last = GET_CONNECTION_STATUS;

			gsm_waiting_for_command_response = 1;

			srl_receive_data_with_callback(srl_context, gsm_sim800_rx_terminating_callback);

			// reverting back to default timeout
			srl_switch_timeout(srl_context, 1, SIM800_DEFAULT_TIMEOUT);

			// start timeout calculation
			srl_context->srl_rx_timeout_calc_started = 1;

			// record when the command has been sent
			gsm_time_of_last_command_send_to_module = main_get_master_time();
		}
	}
}

/**
 * Callback to be called just after the reception is done
 */
uint8_t gsm_sim800_rx_terminating_callback(uint8_t current_data, const uint8_t * const rx_buffer, uint16_t rx_bytes_counter) {

	char current = (char) current_data;

	char before = '\0';

	// special case for CENG request
	if (gsm_at_command_sent_last == ENGINEERING_GET) {
		gsm_terminating_newline_counter = 4;
	}
	else if (gsm_at_command_sent_last == GET_CONNECTION_STATUS) {
		gsm_terminating_newline_counter = 4;
	}
	else {
		gsm_terminating_newline_counter = 4;
	}

	if (rx_bytes_counter > 0) {
		before = (char) *(rx_buffer + rx_bytes_counter - 1);
	}

	// check what character has been received
	if (current == '\n') {
		// increase newline counter
		gsm_receive_newline_counter++;
	}

	// check if this is first character of response (first printable after the echo)
	if (current != '\n' && current != '\r' && (before == '\n' || before == '\r') && gsm_receive_newline_counter < 2) {
		gsm_response_start_idx = rx_bytes_counter;
	}

	// if an echo is enabled and second newline has been received
	if (gsm_at_comm_echo == 1 && gsm_receive_newline_counter > gsm_terminating_newline_counter && gsm_response_start_idx > 0) {

		gsm_receive_newline_counter = 0;

		gsm_waiting_for_command_response = 0;

		return 1;
	}

	return 0;

}

void gsm_sim800_rx_done_event_handler(srl_context_t * srl_context, gsm_sim800_state_t * state) {

	int comparision_result = 123;

	uint8_t second_line, third_line, fourth_line;

	uint32_t newlines = 0;

	gsm_waiting_for_command_response = 0;

	if (srl_context->srl_rx_state == SRL_RX_ERROR) {
		gsm_receive_newline_counter = 0;
	}

	// check how many lines of text
	newlines = gsm_sim800_check_for_extra_newlines(srl_context->srl_rx_buf_pointer + gsm_response_start_idx, srl_context->srl_rx_buf_ln);

	// if a library expects only single line of response
	if (gsm_terminating_newline_counter == 1) {
		// if more than one line of response has been received
		second_line = (newlines & 0x0000FF00) >> 8;
		third_line = (newlines & 0x00FF0000) >> 16;
		fourth_line = (newlines & 0xFF000000) >> 24;

		if (second_line != 0) {
			gsm_sim800_check_for_async_messages(srl_context->srl_rx_buf_pointer + gsm_response_start_idx + second_line, srl_context->srl_rx_buf_ln, & gsm_response_start_idx);
		}
	}

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
		if (gsm_at_command_sent_last == GET_NETWORK_REGISTRATION) {
			comparision_result = strncmp(NETWORK_REGISTRATION, (const char *)(srl_context->srl_rx_buf_pointer + gsm_response_start_idx), 5);

			if (comparision_result == 0) {
				comparision_result = atoi((const char *)(srl_context->srl_rx_buf_pointer + gsm_response_start_idx + 9));

				if (comparision_result >= 0 && comparision_result < 6) {
					gsm_sim800_registration_status = (int8_t)comparision_result;
				}
			}
		}
		else if (gsm_at_command_sent_last == GET_PIN_STATUS) {
			comparision_result = strncmp(CPIN, (const char *)(srl_context->srl_rx_buf_pointer + gsm_response_start_idx), 5);

			if (comparision_result == 0) {
				strncpy(gsm_sim800_sim_status, (const char *)(srl_context->srl_rx_buf_pointer + gsm_response_start_idx + 7), 10);

				gsm_sim800_replace_non_printable_with_space(gsm_sim800_sim_status);
			}

		}
		else if (gsm_at_command_sent_last == GET_REGISTERED_NETWORK) {
			comparision_result = strncmp(REGISTERED_NETWORK, (const char *)(srl_context->srl_rx_buf_pointer + gsm_response_start_idx), 5);

			if (comparision_result == 0) {
				strncpy(gsm_sim800_registered_network, (const char *)(srl_context->srl_rx_buf_pointer + gsm_response_start_idx + 12), 16);

				gsm_sim800_replace_non_printable_with_space(gsm_sim800_registered_network);
			}
		}
		else if (gsm_at_command_sent_last == GET_SIGNAL_LEVEL) {
			comparision_result = strncmp(SIGNAL_LEVEL, (const char *)(srl_context->srl_rx_buf_pointer + gsm_response_start_idx), 5);

			if (comparision_result == 0) {
				comparision_result = atoi((const char *)(srl_context->srl_rx_buf_pointer + gsm_response_start_idx + 6));

				if (comparision_result > 1 && comparision_result < 32) {
					gsm_sim800_signal_level_dbm = (int8_t)(-110 + 2 * (comparision_result - 2));
				}
				else if (comparision_result == 1) {
					gsm_sim800_signal_level_dbm = -111;
				}
				else {
					gsm_sim800_signal_level_dbm = -115;
				}
			}

		}
	}
	else if (*state == SIM800_INITIALIZING_GPRS) {
		sim800_gprs_response_callback(srl_context, state, gsm_response_start_idx);
	}
	else if (*state == SIM800_ALIVE_WAITING_MODEM_RESP) {

		if (gsm_at_command_sent_last == ENGINEERING_ENABLE) {
			gsm_sim800_engineering_response_callback(srl_context, state, gsm_response_start_idx);

			gsm_at_command_sent_last = 0;
		}
		else if (gsm_at_command_sent_last == ENGINEERING_GET) {
			gsm_sim800_engineering_response_callback(srl_context, state, gsm_response_start_idx);

			gsm_at_command_sent_last = 0;
		}
		else if (gsm_at_command_sent_last == ENGINEERING_DISABLE) {
			gsm_sim800_engineering_response_callback(srl_context, state, gsm_response_start_idx);

			gsm_at_command_sent_last = 0;
		}

		*state = SIM800_ALIVE;
	}
}

void gsm_sim800_tx_done_event_handler(srl_context_t * srl_context, gsm_sim800_state_t * state) {
	if (*state == SIM800_ALIVE_SENDING_TO_MODEM) {
		srl_receive_data_with_callback(srl_context, gsm_sim800_rx_terminating_callback);

		// start timeout calculation
		srl_context->srl_rx_timeout_calc_started = 1;

		*state = SIM800_ALIVE_WAITING_MODEM_RESP;

		gsm_time_of_last_command_send_to_module = main_get_master_time();
	}

}
