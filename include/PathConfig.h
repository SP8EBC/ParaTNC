/*
 * path.h
 *
 *  Created on: 02.08.2017
 *      Author: mateusz
 */

#ifndef PATH_H_
#define PATH_H_

#include "aprs/ax25.h"

  /* C++ detection */
  #ifdef __cplusplus
  extern "C" {
  #endif

  uint8_t ConfigPath(AX25Call* p);

  #ifdef __cplusplus
  }
  #endif


#endif /* PATH_H_ */
