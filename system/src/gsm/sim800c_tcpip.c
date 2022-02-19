#include "gsm/sim800c_gprs.h"
#include "gsm/sim800c_tcpip.h"
#include "gsm/sim800c.h"

#include <string.h>

static const char * CONNECT_TCP 			= "AT+CIPSTART=\0";

static const char * CONNECT = "CONNECT\0";
static const char * OK = "OK\0";

const char * TCP3 = "TCP3\0";
const char * TCP4 = "TCP4\0";

static char local_buffer[32];

uint8_t gsm_sim800_tcpip_connect(char * ip_address, uint8_t ip_address_ln, char * port, uint8_t port_ln, srl_context_t * srl_context, gsm_sim800_state_t * state) {
	// this function has blocking io

	uint8_t out = 0;

	uint8_t receive_result = 0xFF;

	int comparision_result = 0;

	if (*state != SIM800_ALIVE) {
		out = 1;
	}
	else {
//		// enable transparent mode
//		srl_send_data(srl_context, (const uint8_t*) TRANSPARENT_MODE_ON, SRL_MODE_ZERO, strlen(TRANSPARENT_MODE_ON), SRL_INTERNAL);
//
//		gsm_at_command_sent_last = TCP3;
//
//		// wait for transmission completion
//		srl_wait_for_tx_completion(srl_context);
//
//		// wait for response from
//		srl_receive_data_with_callback(srl_context, gsm_sim800_rx_terminating_callback);
//
//		// start timeout calculation
//		srl_context->srl_rx_timeout_calc_started = 1;
//
//		// wait for modem response
//		srl_wait_for_rx_completion_or_timeout(srl_context, & receive_result);
//
//		// check if transparent mode has been enabled
//		comparision_result = strncmp(OK, (const char *)(srl_context->srl_rx_buf_pointer + gsm_sim800_get_response_start_idx()), 2);
//
//		if (comparision_result == 0) {
//			memset(local_buffer, 0x00, sizeof(local_buffer));
//		}

	}

	return out;
}
