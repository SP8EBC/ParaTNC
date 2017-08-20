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


	#define FEND	(uint8_t)0xC0
	#define FESC	(uint8_t)0xDB
	#define TFEND	(uint8_t)0xDC
	#define TFESC	(uint8_t)0xDD

  typedef struct KissFrame{
  	char data[CONFIG_AX25_FRAME_BUF_LEN+5];
  	short lng;
  }KissFrame;

  /* C++ detection */
  #ifdef __cplusplus
  extern "C" {
  #endif

  short SendKISSToHost(KissFrame* in, uint8_t* frame, short int frm_len, uint8_t* output);
  short ParseReceivedKISS(uint8_t* SrlRXData, AX25Ctx* ax25, Afsk* a);

  #ifdef __cplusplus
  }
  #endif

#endif /* KISSCOMMUNICATION_H_ */
