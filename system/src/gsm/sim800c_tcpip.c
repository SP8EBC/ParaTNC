#include "gsm/sim800c_gprs.h"
#include "gsm/sim800c_tcpip.h"
#include "gsm/sim800c.h"

#include "main.h"
#include "delay.h"
#include "io.h"

#include <string.h>

#define CONNECT_TCP_LN			18
static const char * CONNECT_TCP 			= "AT+CIPSTART=\"TCP\",\0";
const static char * COMMA 					= ",\0";
const static char * QUOTATION_MARK 			= "\"\0";
const static char * NEWLINE 				= "\r\0";

const static char * CLOSE_TCP				= "AT+CIPCLOSE\r\0";

static const char * ESCAPE					= "+++\0";

static const char * CONNECT					= "OK\r\n\r\nCONNECT\0";
static const char * OK 						= "OK\0";
#define DISCONNECTED_LN		6
static const char * DISCONNECTED 			= "CLOSED\0";

const char * TCP2 = "TCP2\0";
const char * TCP3 = "TCP3\0";
const char * TCP4 = "TCP4\0";

#define LOCAL_BUFFER_LN		64
static char local_buffer[LOCAL_BUFFER_LN];

/**
 * This is set to one if gsm module detected that TCP connection has died
 * or the called party actively disconnected (CLOSED has been received from GSM module)
 */
uint8_t gsm_sim800_tcpip_connection_died = 0;

/**
 * This is set to one if acync receive is actually in progress
 */
uint8_t gsm_sim800_tcpip_receiving = 0;

uint8_t gsm_sim800_tcpip_transmitting = 0;

/**
 * This is a timestamp when last data has been received
 */
uint32_t gsm_sim800_tcpip_last_receive_done = 0;

static char gsm_sim800_previous = ' ';

gsm_sim800_tcpip_receive_callback_t gsm_sim800_tcpip_async_receive_cbk = 0;

static uint8_t gsm_sim800_escape_terminating_callback(uint8_t current_data, const uint8_t * const rx_buffer, uint16_t rx_bytes_counter) {
	if (gsm_sim800_previous == 'O') {
		if ((char)current_data == 'K') {
			gsm_sim800_previous = ' ';

			return 1;
		}
	}

	gsm_sim800_previous = (char) current_data;

	return 0;
}

sim800_return_t gsm_sim800_tcpip_connect(char * ip_or_dns_address, uint8_t address_ln, char * port, uint8_t port_ln, srl_context_t * srl_context, gsm_sim800_state_t * state) {
	// this function has blocking io

	sim800_return_t out = SIM800_OK;

	uint8_t receive_result = SIM800_UNSET;

	int comparision_result = 0;

	if (*state != SIM800_ALIVE) {
		out = SIM800_WRONG_STATE_TO_CONNECT;
	}
	else if (address_ln + port_ln >= LOCAL_BUFFER_LN - sizeof(CONNECT_TCP) - 6) {
		out = SIM800_ADDRESS_AND_PORT_TO_LONG;
	}
	else {
		// reset the buffer
		memset(local_buffer, 0x00, LOCAL_BUFFER_LN);

		// assemble the command with an address to connect
		strncat(local_buffer, CONNECT_TCP, CONNECT_TCP_LN);
		strncat(local_buffer, QUOTATION_MARK, 1);
		strncat(local_buffer, ip_or_dns_address, address_ln);
		strncat(local_buffer, QUOTATION_MARK, 1);
		strncat(local_buffer, COMMA, 1);
		strncat(local_buffer, QUOTATION_MARK, 1);
		strncat(local_buffer, port, port_ln);
		strncat(local_buffer, QUOTATION_MARK, 1);
		strncat(local_buffer, NEWLINE, 1);

		// send connect command to modem
		srl_send_data(srl_context, (const uint8_t*) local_buffer, SRL_MODE_ZERO, strlen(local_buffer), SRL_INTERNAL);

		// wait for transmission to finish
		srl_wait_for_tx_completion(srl_context);

		// due to GPRS delays connecting may last some time, so increase maximum timeout
		srl_switch_timeout(srl_context, 1, 2000);

		// trigger reception
		srl_receive_data_with_callback(srl_context, gsm_sim800_rx_terminating_callback);
		gsm_at_command_sent_last = TCP3;

		// start timeout calculation
		srl_context->srl_rx_timeout_calc_started = 1;

		// wait for it to finish
		srl_wait_for_rx_completion_or_timeout(srl_context, & receive_result);

		// if response from the modem has been received
		if (receive_result == SRL_OK) {
			// check if 'OK and 'CONNECT' has been received which means that connection has been established
			comparision_result = strncmp(OK, (const char *)(srl_context->srl_rx_buf_pointer + gsm_sim800_get_response_start_idx()), 2);

			if (comparision_result == 0) {
				*state = SIM800_TCP_CONNECTED;

				out = SIM800_OK;
			}
			else {
				out = SIM800_CONNECTING_FAILED;
			}
		}
		else {
			out = SIM800_CONNECTING_FAILED;
		}

	}

	return out;
}

sim800_return_t gsm_sim800_tcpip_async_receive(srl_context_t * srl_context, gsm_sim800_state_t * state, srl_rx_termination_callback_t rx_callback, uint32_t timeout, gsm_sim800_tcpip_receive_callback_t rx_done_callback) {

	sim800_return_t out = SIM800_OK;

	// check if connection died
	if (gsm_sim800_tcpip_connection_died == 1) {
		*state = SIM800_ALIVE;

		gsm_sim800_tcpip_connection_died = 0;
	}

	// check if library is in state when reception could be done
	if (state == SIM800_TCP_CONNECTED || gsm_sim800_tcpip_receiving == 0) {

		gsm_sim800_tcpip_async_receive_cbk = rx_done_callback;

		srl_switch_timeout(srl_context, 1, timeout);

		if (rx_callback != 0) {
			srl_receive_data_with_callback(srl_context, rx_callback);

		}
		else {
			srl_receive_data_with_callback(srl_context, gsm_sim800_newline_terminating_callback);
		}

		srl_context->srl_rx_timeout_calc_started = 1;

	}
	else {
		out = SIM800_WRONG_STATE_TO_RX;

	}

	return out;
}

sim800_return_t gsm_sim800_tcpip_receive(uint8_t * buffer, uint16_t buffer_size, srl_context_t * srl_context, gsm_sim800_state_t * state, srl_rx_termination_callback_t rx_callback, uint32_t timeout) {

	sim800_return_t out = SIM800_UNSET;

	uint8_t waiting_result = 0xFF;

	// temporary pointers to store current receive buffer
	uint8_t * temp_buf = 0;
	uint16_t temp_ln = 0;

	if (buffer != 0 && buffer_size != 0) {
		temp_buf = srl_context->srl_rx_buf_pointer;
		temp_ln = srl_context->srl_rx_buf_ln;

		srl_context->srl_rx_buf_pointer = buffer;
		srl_context->srl_rx_buf_ln = buffer_size;
	}

	gsm_sim800_tcpip_async_receive(srl_context, state, rx_callback, timeout, 0);

	srl_wait_for_rx_completion_or_timeout(srl_context, &waiting_result);

	if (buffer != 0 && buffer_size != 0) {
		srl_context->srl_rx_buf_pointer = temp_buf;
		srl_context->srl_rx_buf_ln = temp_ln;
	}

	if (waiting_result == SRL_TIMEOUT) {
		out = SIM800_RECEIVING_TIMEOUT;
	}
	else {
		out = SIM800_OK;
	}

	return out;
//	return waiting_result;
}

sim800_return_t gsm_sim800_tcpip_async_write(uint8_t * data, uint16_t data_len, srl_context_t * srl_context, gsm_sim800_state_t * state) {
	sim800_return_t out = SIM800_OK;

	uint8_t serial_result;

	// check if library is in correct state
	if (*state == SIM800_TCP_CONNECTED && gsm_sim800_tcpip_transmitting == 0) {
		serial_result = srl_send_data(srl_context, (const uint8_t*) data, SRL_MODE_ZERO, data_len, SRL_EXTERNAL);

		if (serial_result != SRL_OK) {
			out = SIM800_SEND_FAILED;
		}

		// this is async transfer so set a flat to block consecutive one
		gsm_sim800_tcpip_transmitting = 1;
	}
	else {
		out = SIM800_WRONG_STATE_TO_TX;
	}

	return out;
}

sim800_return_t gsm_sim800_tcpip_write(uint8_t * data, uint16_t data_len, srl_context_t * srl_context, gsm_sim800_state_t * state) {

	sim800_return_t out = SIM800_OK;

	// check if library is in correct state
	if (*state == SIM800_TCP_CONNECTED && gsm_sim800_tcpip_transmitting == 0) {
		srl_send_data(srl_context, (const uint8_t*) data, SRL_MODE_ZERO, data_len, SRL_EXTERNAL);

		srl_wait_for_tx_completion(srl_context);
	}
	else {
		out = SIM800_WRONG_STATE_TO_TX;
	}

	return out;
}

void gsm_sim800_tcpip_close(srl_context_t * srl_context, gsm_sim800_state_t * state, uint8_t force) {

	uint8_t receive_result = 0;

	if (*state == SIM800_TCP_CONNECTED || force == 1) {

//		do {
//			// set default timeout of 1200msec
//			srl_switch_timeout(srl_context, SRL_TIMEOUT_ENABLE, 2222);
//
//			// send escape sequence to exit connection mode
//			srl_send_data(srl_context, (const uint8_t*) ESCAPE, SRL_MODE_ZERO, strlen(ESCAPE), SRL_INTERNAL);
//
//			// wait for transmission to finish
//			srl_wait_for_tx_completion(srl_context);
//
//			// wait for OK to be received
//			srl_receive_data(srl_context, 4, SRL_NO_START_CHR, SRL_NO_STOP_CHR, SRL_ECHO_DISABLE, 0x7F, 0);
//
//			// start timeout calculation
//			srl_switch_timeout_for_waiting(srl_context, SRL_TIMEOUT_ENABLE);
//
//			// wait for it to finish
//			srl_wait_for_rx_completion_or_timeout(srl_context, & receive_result);
//
//			// check if we escaped from data mode
//			if (strncmp((const char *) (srl_context->srl_rx_buf_pointer), NEWLINE, 1) == 0) {
//				break;
//			}
//			else if (strncmp((const char *) (srl_context->srl_rx_buf_pointer), OK, 2) == 0) {
//				break;
//			}
//			else if (strncmp((const char *) (srl_context->srl_rx_buf_pointer), ESCAPE, 3) == 0) {
//				// if module has already returned to command mode it will echo all input
//				break;
//			}
//			else {
//				if (receive_result == SRL_OK) {
//					delay_fixed(200);
//
//				}
//			}
//
//
//		} while(escape_counter++ < 3);

		io___cntrl_gprs_dtr_low();

		// one second delay
		delay_fixed(500);

		// release DTR
		io___cntrl_gprs_dtr_high();

		delay_fixed(500);

		// send tcp close command
		srl_send_data(srl_context, (const uint8_t*) CLOSE_TCP, SRL_MODE_ZERO, strlen(CLOSE_TCP), SRL_INTERNAL);

		// wait for transmission to finish
		srl_wait_for_tx_completion(srl_context);

		// trigger reception
		srl_receive_data_with_callback(srl_context, gsm_sim800_escape_terminating_callback);

		// start timeout calculation
		srl_context->srl_rx_timeout_calc_started = 1;

		// wait for it to finish
		srl_wait_for_rx_completion_or_timeout(srl_context, & receive_result);

		if (receive_result == SRL_OK) {
			*state = SIM800_ALIVE;
		}

	}
}

void gsm_sim800_tcpip_rx_done_callback(srl_context_t * srl_context, gsm_sim800_state_t * state) {

	gsm_sim800_tcpip_receiving = 0;

	if (srl_context->srl_rx_state == SRL_RX_DONE) {
		gsm_sim800_tcpip_last_receive_done = main_get_master_time();
	}
	else {
		gsm_sim800_tcpip_connection_died = 0;

		*state = SIM800_ALIVE;
	}

	if (gsm_sim800_tcpip_async_receive_cbk != 0) {
		gsm_sim800_tcpip_async_receive_cbk(srl_context);
	}
}

void gsm_sim800_tcpip_tx_done_callback(srl_context_t * srl_context, gsm_sim800_state_t * state) {
	gsm_sim800_tcpip_transmitting = 0;
}

uint8_t gsm_sim800_newline_terminating_callback(uint8_t current_data, const uint8_t * const rx_buffer, uint16_t rx_bytes_counter) {

	sim800_return_t out = SIM800_OK;

	int comparision_result = 0;

	// check if this is maybe an end of 'CLOSED' message
	if (current_data == 'D') {
		// the value of 'rx_buffer' points to the place where 'current_data' is stored within the buffer
		comparision_result = strncmp((const char *) (rx_buffer + (rx_bytes_counter - DISCONNECTED_LN)), DISCONNECTED, DISCONNECTED_LN);

		// if 'CLOSED' has been found
		if (comparision_result == 0) {
			// end the reception as connections is dead
			out = SIM800_RX_TERMINATED;

			// yes, we are not protected against sentence 'CLOSED' appeared among protocol data
			gsm_sim800_tcpip_connection_died = 1;
		}
	}
	else if (current_data == '\n') {
		out = SIM800_RX_TERMINATED;
	}

	return out;
}

