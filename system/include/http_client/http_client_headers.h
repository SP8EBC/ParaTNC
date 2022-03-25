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

void http_client_headers_preamble(http_client_method_t method, char * url, uint8_t url_ln, char * output, uint8_t output_ln);
void http_client_headers_user_agent(char * output, uint8_t output_ln);


#endif /* INCLUDE_HTTP_CLIENT_HTTP_CLIENT_HEADERS_H_ */
