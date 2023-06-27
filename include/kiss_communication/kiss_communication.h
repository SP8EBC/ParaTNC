/*
 * KissCommunication.h
 *
 *  Created on: 29.04.2017
 *      Author: mateusz
 */

#ifndef KISSCOMMUNICATION_H_
#define KISSCOMMUNICATION_H_

#include "etc/ax25_config.h"
#include "aprs/ax25.h"
#include "aprs/afsk.h"

#include "stdint.h"

#include "drivers/serial.h"

#define KISS_TOO_LONG_FRM -1

	#define FEND	(uint8_t)0xC0
	#define FESC	(uint8_t)0xDB
	#define TFEND	(uint8_t)0xDC
	#define TFESC	(uint8_t)0xDD

	#define NONSTANDARD	(uint8_t)0x0F



extern uint8_t kiss_current_async_message;

  /* C++ detection */
  #ifdef __cplusplus
  extern "C" {
  #endif

  uint8_t kiss_async_pooler(uint8_t* output, uint16_t output_len );

  int32_t kiss_send_ax25_to_host(uint8_t* input_frame, uint16_t input_frame_len, uint8_t* output, uint16_t output_len);
  int32_t kiss_parse_received(uint8_t* input_frame_from_host, uint16_t input_len, AX25Ctx* ax25, Afsk* a, uint8_t * response_buffer, uint16_t resp_buf_ln );

  void kiss_reset_buffer(uint8_t* output, uint16_t output_len, uint16_t* current_len);
  uint8_t kiss_put_char(uint8_t c, uint8_t* output, uint16_t output_len, uint16_t* current_len, uint16_t* crc);
  uint8_t kiss_put_char_nocheck(uint8_t c, uint8_t* output, uint16_t output_len, uint16_t* current_len, uint16_t* crc);
  void kiss_put_call(const AX25Call *addr, uint8_t is_last, uint8_t* output, uint16_t output_len, uint16_t* current_len, uint16_t* crc);
  void kiss_finalize_buffer(uint8_t* output, uint16_t output_len, uint16_t* current_len);

  #ifdef __cplusplus
  }
  #endif

#endif /* KISSCOMMUNICATION_H_ */
