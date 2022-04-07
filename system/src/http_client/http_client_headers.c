
/*
 * http_client_headers.c
 *
 *  Created on: Mar 25, 2022
 *      Author: mateusz
 */

#include "http_client/http_client_headers.h"

#include <stdio.h>
#include <string.h>

#include "main.h"	// for sofrware version

const char * const http_client_get 		= "GET";
const char * const http_client_post 	= "POST";
const char * const http_client_put 		= "PUT";
const char * const http_client_delete 	= "DELETE";


uint16_t http_client_headers_preamble(http_client_method_t method, char *url,
		uint8_t url_ln, char *output, uint16_t output_ln) {

	// lenght of header
	uint16_t out = 0;

	// pointer to string with method name
	const char * method_string = 0;

	// choose correct
	switch (method) {
	case HTTP_POST:
		method_string = http_client_post;
		break;
	case HTTP_PUT:
		method_string = http_client_put;
		break;
	case HTTP_DELETE:
		method_string = http_client_delete;
		break;
	case HTTP_GET:
	default:
		method_string = http_client_get;
		break;
	}

	snprintf(output, output_ln, "%s %s HTTP/1.1\r\n", method_string, url);

	out = strlen (output);

	return out;

}

uint16_t http_client_headers_user_agent(char *output, uint16_t output_ln,
		uint16_t offset) {

	uint16_t out = 0;

	snprintf(output + offset, output_ln - offset, "User-Agent: ParaMETEO %s-%s\r\n", SW_VER, SW_DATE);

	out = strlen (output);

	return out;
}

uint16_t http_client_headers_accept(char *output, uint16_t output_ln,
		uint16_t offset) {

	uint16_t out = 0;

	snprintf(output + offset, output_ln - offset, "Accept: application/json\r\n");

	out = strlen (output);

	return out;
}

uint16_t http_client_headers_terminate(char* output, uint16_t output_ln, uint16_t offset) {
	uint16_t out = 0;

	snprintf(output + offset, output_ln - offset, "\r\n\r\n");

	out = strlen (output);

	return out;
}
