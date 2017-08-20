/*
 * digi.h
 *
 *  Created on: 28.05.2017
 *      Author: mateusz
 */

#ifndef INCLUDE_APRS_DIGI_H_
#define INCLUDE_APRS_DIGI_H_

#include "aprs/ax25.h"

#ifdef __cplusplus
extern "C"
{
#endif

char Digi(struct AX25Msg *msg);

#ifdef __cplusplus
}
#endif


#endif /* INCLUDE_APRS_DIGI_H_ */
