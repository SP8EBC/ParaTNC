/*
 * umb_client.h
 *
 *  Created on: 22.02.2020
 *      Author: mateusz
 */

#ifndef INCLUDE_UMB_CLIENT_UMB_CLIENT_H_
#define INCLUDE_UMB_CLIENT_UMB_CLIENT_H_

#include "station_config.h"
#include "umb_frame_t.h"
#include "umb_retval_t.h"

#ifdef _UMB_CLIENT

void umb_client_init();
umb_retval_t umb_parse_serial_buffer_to_frame(uint8_t* serial_buffer, uint16_t buffer_ln, umb_frame_t* frame);
umb_retval_t umb_parse_frame_to_serial_buffer(uint8_t* serial_buffer, uint16_t buffer_ln, umb_frame_t* frame);
uint16_t umb_calc_crc(uint16_t crc_buff, uint8_t input);

#endif

#endif /* INCLUDE_UMB_CLIENT_UMB_CLIENT_H_ */
