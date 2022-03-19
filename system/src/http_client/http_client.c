#include "http_client/http_client.h"
#include "http_client/http_client_rx_callback.h"

#include <string.h>

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
