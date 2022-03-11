#include "gsm/http_client.h"

#include <string.h>

static const char * DISCONNECTED 			= "CLOSED\0";
static const char * CONTENT_LN				= "Content-Length: \0";

#define WAIT_FOR_CONTENT_LENGHT		0xFFFFu

/**
 * Content lenght received from HTTP response headers
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
static char http_client_header_buffer[32];

/**
 * Index used to walk through 'http_client_header_buffer'
 */
static uint8_t http_client_header_index = 0;

/**
 * This static function is used as a termination callback for serial I/O with GPRS module.
 * It ends transmission in one of three cases
 * 	1. All data is received according to size specified by Content-Lenght field in response header
 * 	2. If user set 'response_ln_limit' to non zero value and 'response_ln_limit' characters have been received (it also set HTTP_CLIENT_TOO_LONG_RESPONSE)
 * 	3. If connection has been closed by remote server
 *
 */
static uint8_t http_client_rx_done_callback(uint8_t current_data, const uint8_t * const rx_buffer, uint16_t rx_bytes_counter) {

	uint8_t out = 0;

	int compare_result = 0;

	// check if we wait for content lenght
	if (http_client_content_lenght == WAIT_FOR_CONTENT_LENGHT) {
		// copy current character to temporary buffer
		http_client_header_buffer[http_client_header_index++] = (char)current_data;

		// check if
		if (current_data == '\r') {

		}
	}

	// if this is maybe the last character of 'CLOSED'
	if ((char)current_data == 'D') {
		// check 6 previous characters
		compare_result = strncmp(DISCONNECTED, (const char *) (rx_buffer + rx_bytes_counter - 6), 6);

		// terminate reception if 'CLOSED' has been found.
		if (compare_result == 0) {
			out = 1;
		}
	}

	// if this is maybe an end of content lenght
	else if (http_client_content_lenght == 0 && (char)current_data == 'h') {
		// check 14 previous characters
		compare_result = strncmp(CONTENT_LN, rx_buffer + rx_bytes_counter - 14, 14);

		// terminate reception if 'CLOSED' has been found.
		if (compare_result == 0) {
			// set waiting for
			http_client_content_lenght = WAIT_FOR_CONTENT_LENGHT;

			// clear the buffer where header value will be stored
			memset (http_client_header_buffer, 0x00, sizeof(http_client_header_buffer));
		}
	}

	return out;
}


void http_client_init(gsm_sim800_state_t * state, srl_context_t * serial_context) {

}


uint8_t http_client_async_get(char * url, uint8_t url_ln, uint16_t response_ln_limit) {
	return 0;
}

uint8_t http_client_async_post(char * url, uint8_t url_ln, char * data_to_post, uint8_t data_ln) {
	return 0;
}


char * http_client_get_server_response() {
	return 0;
}

uint16_t http_client_get_latest_http_code() {
	return http_client_http_code;
}
