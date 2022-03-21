#include "http_client/http_client.h"
#include "http_client/http_client_rx_callback.h"

#include <string.h>

typedef enum http_client_state {
	HTTP_CLIENT_UNITIALIZED,
	HTTP_CLIENT_READY,
	HTTP_CLIENT_CONNECTED_IDLE,
	HTTP_CLIENT_WAITING_POST,
	HTTP_CLIENT_WAITING_GET
} http_client_state_t;

http_client_state_t http_client_state = HTTP_CLIENT_UNITIALIZED;

/**
 * Content lenght received from HTTP response headers or chunked encoding
 */
uint16_t http_client_content_lenght = 0;

/**
 *	HTTP code returned by the latest query. It is zeroed after each successful call to async
 *	function. This indicate that a request is currently in progress. Negative values means some
 *	non HTTP error, like communication timeout or response longer than expected
 */
int16_t http_client_http_code = 0;

/**
 * Temporary buffer for processing
 */
char http_client_header_buffer[HEADER_BUFFER_LN];

/**
 * Index used to walk through 'http_client_header_buffer'
 */
uint8_t http_client_header_index = 0;

/**
 * SIM800 state and serial context used to communication with gsm module.
 */
gsm_sim800_state_t * http_client_deticated_sim800_state;
srl_context_t * http_client_deticated_serial_context;

/**
 * If this is set to non zero, the library will stop response processing after HTTP code different from <200, 299>
 */
uint8_t http_client_ignore_content_on_http_error;

/**
 * Default port for http
 */
const char * http_client_default_port = "80";

/**
 * This functions splits the URL string into hostname and path
 *
 * It return a split point index which in case of
 * http://pogoda.cc:8080/meteo_backend/station/z_gss_zar/summary
 * will return an index of '/' after 8080
 *
 *  */
static uint16_t http_client_split_hostname_and_path(char * input, uint16_t input_ln) {

	uint16_t out = 0xFFFF;

	uint16_t iterator = 7;

	// check if URL starts correctly
	if (*input == 'h' && *(input + 1) == 't'  && *(input + 2) == 't'  && *(input + 3) == 'p'  && *(input + 4) == ':'  && *(input + 5) == '/'  && *(input + 6) == '/') {
		for (; iterator < input_ln; iterator++) {
			if (*(input + iterator) == '/') {

				out = iterator;

				break;
			}
		}
	}

	return out;
}

static void http_client_get_port_from_url(char * input, uint16_t input_ln, char * port, uint16_t port_ln) {

	uint16_t split_point = http_client_split_hostname_and_path(input, input_ln);

}

void http_client_init(gsm_sim800_state_t * state, srl_context_t * serial_context, uint8_t ignore_content_on_http_error) {

	http_client_deticated_sim800_state = state;

	http_client_state = HTTP_CLIENT_READY;
}


uint8_t http_client_async_get(char * url, uint8_t url_ln, uint16_t response_ln_limit, uint8_t force_disconnect_on_busy) {

	uint16_t split_point = http_client_split_hostname_and_path(url, url_ln);

	uint8_t out = 0;

	// simple check if url seems to be corrected or not
	if (split_point != 0xFFFF && http_client_state == HTTP_CLIENT_READY ) {

		// check if module is busy on other TCP/IP connection
		if (*http_client_deticated_sim800_state == SIM800_TCP_CONNECTED && force_disconnect_on_busy != 0) {
			// if client is connected end a user wants to force disconnect
			gsm_sim800_tcpip_close(http_client_deticated_serial_context, http_client_deticated_sim800_state);
		}
		else if (*http_client_deticated_sim800_state == SIM800_TCP_CONNECTED && force_disconnect_on_busy == 0) {
			out = HTTP_CLIENT_RET_TCPIP_BSY;
		}

		gsm_sim800_tcpip_connect(url, url_ln, url, http_client_http_code, http_client_deticated_serial_context, http_client_deticated_sim800_state);
	}
	else if (split_point == 0xFFFF) {
		out = HTTP_CLIENT_RET_WRONG_URL;
	}
	else if (http_client_state != HTTP_CLIENT_READY) {
		out = HTTP_CLIENT_RET_UNITIALIZED;
	}

	return out;
}

uint8_t http_client_async_post(char * url, uint8_t url_ln, char * data_to_post, uint8_t data_ln, uint8_t force_disconnect_on_busy) {
	return 0;
}


char * http_client_get_server_response() {
	return 0;
}

uint16_t http_client_get_latest_http_code() {
	return http_client_http_code;
}
