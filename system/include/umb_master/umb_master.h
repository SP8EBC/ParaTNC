/*
 * umb_client.h
 *
 *  Created on: 22.02.2020
 *      Author: mateusz
 */

#ifndef INCLUDE_UMB_MASTER_UMB_MASTER_H_
#define INCLUDE_UMB_MASTER_UMB_MASTER_H_

#include <umb_master/umb_frame_t.h>
#include <umb_master/umb_retval_t.h>
#include <umb_master/umb_context_t.h>
#include <umb_master/umb_call_reason.h>
#include "station_config.h"


#ifdef _UMB_MASTER

extern umb_context_t umb_context;

void umb_master_init(umb_context_t* ctx);
umb_retval_t umb_parse_serial_buffer_to_frame(uint8_t* serial_buffer, uint16_t buffer_ln, umb_frame_t* frame);
umb_retval_t umb_parse_frame_to_serial_buffer(uint8_t* serial_buffer, uint16_t buffer_ln, umb_frame_t* frame, uint16_t* target_ln);
uint16_t umb_calc_crc(uint16_t crc_buff, uint8_t input);
umb_retval_t umb_pooling_handler(umb_context_t* ctx, umb_call_reason_t r);
umb_retval_t umb_master_callback(umb_frame_t* frame, umb_context_t* ctx);

#endif

#endif /* INCLUDE_UMB_MASTER_UMB_MASTER_H_ */
