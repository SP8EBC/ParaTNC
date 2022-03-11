/*
 * http_client.h
 *
 *  Created on: Mar 10, 2022
 *      Author: mateusz
 */

#ifndef INCLUDE_GSM_HTTP_CLIENT_H_
#define INCLUDE_GSM_HTTP_CLIENT_H_

#include "gsm/sim800c.h"

#define HTTP_CLIENT_OK					0
#define HTTP_CLIENT_TIMEOUT				-100
#define HTTP_CLIENT_TOO_LONG_RESPONSE	-110

void http_client_init(gsm_sim800_state_t * state, srl_context_t * serial_context);
uint8_t http_client_async_get(char * url, uint8_t url_ln, uint16_t response_ln_limit);
uint8_t http_client_async_post(char * url, uint8_t url_ln, char * data_to_post, uint8_t data_ln);

char * http_client_get_server_response();
uint16_t http_client_get_latest_http_code();

#endif /* INCLUDE_GSM_HTTP_CLIENT_H_ */
