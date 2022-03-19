/*
 * http_client_rx_callback.h
 *
 *  Created on: Mar 19, 2022
 *      Author: mateusz
 */

#ifndef INCLUDE_HTTP_CLIENT_HTTP_CLIENT_RX_CALLBACK_H_
#define INCLUDE_HTTP_CLIENT_HTTP_CLIENT_RX_CALLBACK_H_

#include <stdint.h>

uint8_t http_client_rx_done_callback(uint8_t current_data, const uint8_t * const rx_buffer, uint16_t rx_bytes_counter);

#endif /* INCLUDE_HTTP_CLIENT_HTTP_CLIENT_RX_CALLBACK_H_ */
