/*
 * http_client.h
 *
 *  Created on: Mar 10, 2022
 *      Author: mateusz
 */

#ifndef INCLUDE_GSM_HTTP_CLIENT_H_
#define INCLUDE_GSM_HTTP_CLIENT_H_

/**
 * This library supports only HTTP/1.1 and chunked Trasfer-Encoding as described here:
 * https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Transfer-Encoding
 */

#include "gsm/sim800c.h"

#define HTTP_CLIENT_OK					0
#define HTTP_CLIENT_TIMEOUT				-100
#define HTTP_CLIENT_TOO_LONG_RESPONSE	-110
#define HTTP_CLIENT_HEADERS_OVERFLOW	-120
#define HTTP_CLIENT_DISCONNECTED		-130			// this is set if remote server disconnected in unexpected moment

#define HEADER_BUFFER_LN 	64

/**
 *	HTTP code returned by the latest query. It is zeroed after each successful call to async
 *	function. This indicate that a request is currently in progress. Negative values means some
 *	non HTTP error, like communication timeout or response longer than expected
 */
extern int16_t http_client_http_code;

/**
 * Content lenght received from HTTP response headers or chunked encoding
 */
extern uint16_t http_client_content_lenght;

/**
 * Maximum content lenght which should be received by the client. Please bear in mind that THIS NOT include
 * HTTP headers lenght
 */
extern uint16_t http_client_max_content_ln;

/**
 * Temporary buffer for processing
 */
extern char http_client_header_buffer[HEADER_BUFFER_LN];

/**
 * Index used to walk through 'http_client_header_buffer'
 */
extern uint8_t http_client_header_index;

void http_client_init(gsm_sim800_state_t * state, srl_context_t * serial_context);
uint8_t http_client_async_get(char * url, uint8_t url_ln, uint16_t response_ln_limit);
uint8_t http_client_async_post(char * url, uint8_t url_ln, char * data_to_post, uint8_t data_ln);

char * http_client_get_server_response();
uint16_t http_client_get_latest_http_code();

#endif /* INCLUDE_GSM_HTTP_CLIENT_H_ */
