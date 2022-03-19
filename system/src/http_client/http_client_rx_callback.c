/*
 * http_client_rx_callback.c
 *
 *  Created on: Mar 19, 2022
 *      Author: mateusz
 */

#include "http_client/http_client_rx_callback.h"
#include "http_client/http_client.h"

#include <string.h>
#include <stdlib.h>

static const char * DISCONNECTED 					= "CLOSED\0";

static const char * CONTENT_LN						= "Content-Length: \0";
#define CONTENT_LN_LN							16
static const char * TRANSFER_ENCODING_CHUNKED 		= "Transfer-Encoding: chunked\0";
#define TRANSFER_ENCODING_CHUNKED_LN			26
static const char * HTTP_HEADER						= "HTTP/\0";
#define HTTP_HEADER_LN							5
static const char * BLANK_NEWLINE					= "\r\n\0";

#define WAIT_FOR_CONTENT_LENGHT		0xFFFFu
#define CONTENT_IN_CHUNKS			0xFFFEu

typedef enum http_client_header_field {

	HEADER_HTTP,
	HEADER_CONTENT_LN,
	HEADER_TRANSFER_ENCODING_CHUNKED,
	HEADER_END,
	HEADER_UNKNOWN
} http_client_header_field_t;

// set to one if we are still parsing HTTP response header
static uint8_t http_client_response_header_processing = 1;


// amount of bytes (octets) of a content of the HTTP response received so far
static uint16_t http_client_content_received_so_far = 0;

// an index where the first byte of response occurs
uint16_t http_client_content_start_index = 0;

/**
 * This function is responsible for checking what HTTP header has been received
 */
static http_client_header_field_t http_client_check_what_field_it_is(char * buffer, uint16_t buffer_ln) {

	http_client_header_field_t out = HEADER_UNKNOWN;

	if (strncmp(CONTENT_LN, buffer, CONTENT_LN_LN) == 0) {
		out = HEADER_CONTENT_LN;
	}
	else if (strncmp(TRANSFER_ENCODING_CHUNKED, buffer, TRANSFER_ENCODING_CHUNKED_LN) == 0) {
		out = HEADER_TRANSFER_ENCODING_CHUNKED;
	}
	else if (strncmp(HTTP_HEADER, buffer, HTTP_HEADER_LN) == 0) {
		out = HEADER_HTTP;
	}
	else if (strncmp(BLANK_NEWLINE, buffer, 2) == 0) {
		out = HEADER_END;
	}

	return out;
}

/**
 * This function resets all variables used by rx done termination callback back to theirs default values
 */
void http_client_rx_done_callback_init() {
	http_client_response_header_processing = 1;
	http_client_content_received_so_far = 0;
	http_client_content_start_index = 0;
	http_client_header_index= 0;
	memset (http_client_header_buffer, 0x0, HEADER_BUFFER_LN);
}

/**
 * This function is used as a termination callback for serial I/O with GPRS module.
 * It ends transmission in one of three cases
 * 	1. All data is received according to size specified by Content-Lenght field in response header
 * 	2. If user set 'response_ln_limit' to non zero value and 'response_ln_limit' characters have been received (it also set HTTP_CLIENT_TOO_LONG_RESPONSE)
 * 	3. If connection has been closed by remote server
 *
 * It assumes that 'http_client_header_buffer' is zeroed
 *
 */
uint8_t http_client_rx_done_callback(uint8_t current_data, const uint8_t * const rx_buffer, uint16_t rx_bytes_counter) {

	uint8_t out = 0;

	int compare_result = 0;

	// local buffer used to fetch HTTP error code or similar short things
	char local_buffer[9];

	// if this is maybe the last character of 'CLOSED'
	if ((char)current_data == 'D') {
		// check 6 previous characters
		compare_result = strncmp(DISCONNECTED, (const char *) (rx_buffer + rx_bytes_counter - 6), 6);

		// terminate reception if 'CLOSED' has been found.
		if (compare_result == 0) {
			out = 1;

			http_client_http_code = HTTP_CLIENT_DISCONNECTED;
		}
	}

	// check if we wait for content length
	if (http_client_response_header_processing == 1) {
		// copy current character to temporary buffer
		http_client_header_buffer[http_client_header_index++] = (char)current_data;

		// we have an overflow during processing HTTP headers
		if (http_client_header_index >= HEADER_BUFFER_LN) {
			out = 1;

			// set an error code also
			http_client_http_code = HTTP_CLIENT_HEADERS_OVERFLOW;
		}
		else {
			// check if this is newline
			if (current_data == '\n') {
				// if yes parse the content of HTTP header buffer
				http_client_header_field_t field = http_client_check_what_field_it_is(http_client_header_buffer, HEADER_BUFFER_LN);

				memset(local_buffer, 0x0, 0x9);

				switch (field) {

					// fetch the HTTP return code
					case HEADER_HTTP: {
						// copy last 6 characters of header >> 0000   48 54 54 50 2f 31 2e 31 20 32 30 30 20 0d 0a      HTTP/1.1 200 .. <<
						memcpy(local_buffer, http_client_header_buffer + http_client_header_index - 6, 6);

						// convert to int
						http_client_http_code = atoi(local_buffer);
						break;
					}

					case HEADER_CONTENT_LN: {
						// content ln may vary, so we need to copy data starting from the begining not the end
						strncpy(local_buffer, http_client_header_buffer + 16, 6);

						// convert to integer value
						http_client_content_lenght = atoi(local_buffer);

						break;
					}

					case HEADER_TRANSFER_ENCODING_CHUNKED: {
						/**
						 * 	0000   54 72 61 6e 73 66 65 72 2d 45 6e 63 6f 64 69 6e   Transfer-Encodin
						 *	0010   67 3a 20 63 68 75 6e 6b 65 64 0d 0a               g: chunked..
						 *
						 */

						// set that the content will be sent in chunks
						http_client_content_lenght = CONTENT_IN_CHUNKS;

						// this is not the end of the header as such as neighter 'Transfer-Encoding' nor
						// 'Content-Lenght' need to occur as the last thing in response
						break;
					}

					case HEADER_END: {
						http_client_response_header_processing = 0;

						break;
					}

					// not all headers needs to be processed
					default:
						break;
				}

				// clear temporary buffer
				memset (http_client_header_buffer, 0x00, HEADER_BUFFER_LN);
				http_client_header_index = 0;
			}
		}

	}
	else {
		// we know the lenght of response or we are waiting for first chunk
		if (http_client_content_lenght == CONTENT_IN_CHUNKS) {

			// save content data until we get chunk size
			http_client_header_buffer[http_client_header_index++] = (char)current_data;

			// if this is a first newline
			if ((char)current_data == '\n') {
				// everything what is now in 'http_client_header_buffer' is the size of chunk in hex!!
				http_client_content_lenght = strtol(http_client_header_buffer, 0, 16);

				// data starts from the next byte
				http_client_content_start_index = rx_bytes_counter + 1;
			}
		}
		else {
			// chunk size is known so count data
			http_client_content_received_so_far++;

			// check if all bytes defined by chunk size or 'HEADER_CONTENT_LN' have been received
			if (http_client_content_received_so_far >= http_client_content_lenght) {
				out = 1;
			}
			else if (http_client_max_content_ln != 0 && (http_client_content_received_so_far > http_client_max_content_ln)) {
				out = 1;
			}
		}
	}


	return out;
}
