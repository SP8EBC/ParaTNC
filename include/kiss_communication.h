/*
 * KissCommunication.h
 *
 *  Created on: 29.04.2017
 *      Author: mateusz
 */

#ifndef KISSCOMMUNICATION_H_
#define KISSCOMMUNICATION_H_

#include "aprs/config.h"
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

#define KISS_DATA	 				(uint8_t) 0x00

#define KISS_GET_RUNNING_CONFIG 	(uint8_t) 0x20
#define KISS_RUNNING_CONFIG			(uint8_t) 0x70

#define KISS_GET_VERSION_AND_ID		(uint8_t) 0x21
#define KISS_VERSION_AND_ID			(uint8_t) 0x71

#define KISS_ERASE_STARTUP_CFG		(uint8_t) 0x22
#define KISS_ERASE_STARTUP_CFG_RESP	(uint8_t) 0x72

#define KISS_PROGRAM_STARTUP_CFG		(uint8_t) 0x23
#define KISS_PROGRAM_STARTUP_CFG_RESP	(uint8_t) 0x73

#define KISS_CONFIG_CRC			(uint8_t) 0x24
#define KISS_CONFIG_CRC_RESP	(uint8_t) 0x74

#define KISS_RESTART			(uint8_t) 0x25
#define KISS_RESTART_RESP		(uint8_t) 0x75

#define KISS_TOGGLE_PTT			(uint8_t) 0x26
//#define KISS_RESTART_RESP		(uint8_t) 0x76

#define KISS_CONTROL_VOLTAGE			(uint8_t) 0x27
#define KISS_CONTROL_VOLTAGE_RESP		(uint8_t) 0x77

#define KISS_RETURN_IDLE		1

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
