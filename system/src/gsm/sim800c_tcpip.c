#include "gsm/sim800c_gprs.h"
#include "gsm/sim800c_tcpip.h"
#include "gsm/sim800c.h"

#include "main.h"

#include <string.h>

#define CONNECT_TCP_LN			18
static const char * CONNECT_TCP 			= "AT+CIPSTART=\"TCP\",\0";
const static char * COMMA = ",\0";
const static char * QUOTATION_MARK = "\"\0";
const static char * NEWLINE = "\r\0";

const static char * CLOSE_TCP			= "AT+CIPCLOSE\r\0";

static const char * ESCAPE					= "+++\0";

static const char * CONNECT = "OK\r\n\r\nCONNECT\0";
static const char * OK = "OK\0";
#define DISCONNECTED_LN		6
static const char * DISCONNECTED = "CLOSED\0";

const char * TCP2 = "TCP2\0";
const char * TCP3 = "TCP3\0";
const char * TCP4 = "TCP4\0";

#define LOCAL_BUFFER_LN		48
static char local_buffer[LOCAL_BUFFER_LN];

/**
 * This is set to one if TCP connection has died or the called party actively disconnected (CLOSED has been received from GSM module)
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

uint8_t gsm_sim800_tcpip_connect(char * ip_or_dns_address, uint8_t address_ln, char * port, uint8_t port_ln, srl_context_t * srl_context, gsm_sim800_state_t * state) {
	// this function has blocking io

	uint8_t out = 0;

	uint8_t receive_result = 0xFF;

	int comparision_result = 0;

	if (*state != SIM800_ALIVE) {
		out = 1;
	}
	else if (address_ln + port_ln >= LOCAL_BUFFER_LN - sizeof(CONNECT_TCP) - 6) {
		out = 2;
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

				out = 0;
			}
			else {
				out = 1;
			}
		}

	}

	return out;
}

uint8_t gsm_sim800_tcpip_async_receive(srl_context_t * srl_context, gsm_sim800_state_t * state, srl_rx_termination_callback_t rx_callback, uint32_t timeout, gsm_sim800_tcpip_receive_callback_t rx_done_callback) {

	uint8_t out = 0;

	// check if connection died
	if (gsm_sim800_tcpip_connection_died == 1) {
		*state = SIM800_ALIVE;

		gsm_sim800_tcpip_connection_died = 0;
	}

	// check if library is in state when reception could be done
	if (*state != SIM800_TCP_CONNECTED && gsm_sim800_tcpip_receiving == 1) {
		out = 1;
	}
	else {

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

	return out;
}

uint8_t gsm_sim800_tcpip_receive(uint8_t * buffer, uint16_t buffer_size, srl_context_t * srl_context, gsm_sim800_state_t * state, srl_rx_termination_callback_t rx_callback, uint32_t timeout) {

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

	return waiting_result;
}

uint8_t gsm_sim800_tcpip_async_write(uint8_t * data, uint16_t data_len, srl_context_t * srl_context, gsm_sim800_state_t * state) {
	uint8_t out = 0;

	if (*state == SIM800_TCP_CONNECTED && gsm_sim800_tcpip_transmitting == 0) {
		srl_send_data(srl_context, (const uint8_t*) data, SRL_MODE_ZERO, data_len, SRL_EXTERNAL);
	}

	return out;
}

uint8_t gsm_sim800_tcpip_write(uint8_t * data, uint16_t data_len, srl_context_t * srl_context, gsm_sim800_state_t * state) {

	uint8_t out = 0;

	if (*state == SIM800_TCP_CONNECTED && gsm_sim800_tcpip_transmitting == 0) {
		srl_send_data(srl_context, (const uint8_t*) data, SRL_MODE_ZERO, data_len, SRL_EXTERNAL);

		srl_wait_for_tx_completion(srl_context);
	}

	return out;
}

void gsm_sim800_tcpip_close(srl_context_t * srl_context, gsm_sim800_state_t * state) {

	uint8_t receive_result = 0;

	if (*state == SIM800_TCP_CONNECTED) {
		// send escape sequence to exit connection mode
		srl_send_data(srl_context, (const uint8_t*) ESCAPE, SRL_MODE_ZERO, strlen(ESCAPE), SRL_INTERNAL);

		// wait for transmission to finish
		srl_wait_for_tx_completion(srl_context);

		// wait for OK to be received
		srl_receive_data_with_callback(srl_context, gsm_sim800_escape_terminating_callback);

		// wait for it to finish
		srl_wait_for_rx_completion_or_timeout(srl_context, & receive_result);

		// send escape sequence to exit connection mode
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

uint8_t gsm_sim800_newline_terminating_callback(uint8_t current_data, const uint8_t * const rx_buffer, uint16_t rx_bytes_counter) {

	uint8_t out = 0;

	int comparision_result = 0;

	// check if this is maybe an end of 'CLOSED' message
	if (current_data == 'D') {
		// the value of 'rx_buffer' points to the place where 'current_data' is stored within the buffer
		comparision_result = strncmp((const char *) (rx_buffer + (rx_bytes_counter - DISCONNECTED_LN)), DISCONNECTED, DISCONNECTED_LN);

		// if 'CLOSED' has been found
		if (comparision_result == 0) {
			// end the reception as connections is dead
			out = 1;

			// yes, we are not protected against sentence 'CLOSED' appeared among protocol data
			gsm_sim800_tcpip_connection_died = 1;
		}
	}
	else if (current_data == '\n') {
		out = 1;
	}

	return out;
}

