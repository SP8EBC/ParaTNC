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

#define KISS_TOO_LONG_FRM -1

	#define FEND	(uint8_t)0xC0
	#define FESC	(uint8_t)0xDB
	#define TFEND	(uint8_t)0xDC
	#define TFESC	(uint8_t)0xDD

//	typedef struct KissFrame{
//		uint8_t data[CONFIG_AX25_FRAME_BUF_LEN+5];
//		uint16_t lng;
//	}KissFrame;

  /* C++ detection */
  #ifdef __cplusplus
  extern "C" {
  #endif

  int32_t SendKISSToHost(uint8_t* input_frame, uint16_t input_frame_len, uint8_t* output, uint16_t output_len);
  short ParseReceivedKISS(uint8_t* input_frame_from_host, uint16_t input_len, AX25Ctx* ax25, Afsk* a);

  #ifdef __cplusplus
  }
  #endif

#endif /* KISSCOMMUNICATION_H_ */
