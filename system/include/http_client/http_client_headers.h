/*
 * http_client_headers.h
 *
 *  Created on: Mar 25, 2022
 *      Author: mateusz
 */

#ifndef INCLUDE_HTTP_CLIENT_HTTP_CLIENT_HEADERS_H_
#define INCLUDE_HTTP_CLIENT_HTTP_CLIENT_HEADERS_H_

#include <stdint.h>

typedef enum http_client_method {
	HTTP_GET,
	HTTP_POST,
	HTTP_PUT,
	HTTP_DELETE
} http_client_method_t;

/**
 * All these functions appends HTTP headers to output buffer. Every function (except preamble) receives three
 * parameters:
 *
 *  char * output - pointer to the output buffer
 *  uint16_t output_ln - lenght of output buffer
 *  uint16_t offset - an offset from which each function will print its header
 *
 *  functions return an offset of one character after the last character of header. this value can be used
 *  afterwards as the value of 'offset'
 */
uint16_t http_client_headers_preamble(http_client_method_t method, char * url, uint8_t url_ln, char * output, uint16_t output_ln);
uint16_t http_client_headers_host(char * host, uint16_t host_ln, char *output, uint16_t output_ln, uint16_t offset);
uint16_t http_client_headers_user_agent(char * output, uint16_t output_ln, uint16_t offset);
uint16_t http_client_headers_accept(char* output, uint16_t output_ln, uint16_t offset);
uint16_t http_client_headers_terminate(char* output, uint16_t output_ln, uint16_t offset);
uint16_t http_client_headers_content_ln(char* output, uint16_t output_ln, uint16_t offset, uint16_t content_ln);		// used for POST and PUT method
uint16_t http_client_headers_content_type_json(char* output, uint16_t output_ln, uint16_t offset);	// used for POST and PUT method

#endif /* INCLUDE_HTTP_CLIENT_HTTP_CLIENT_HEADERS_H_ */
