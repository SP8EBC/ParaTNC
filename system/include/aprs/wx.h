/*
 * wx.h
 *
 *  Created on: 28.05.2017
 *      Author: mateusz
 */

#ifndef INCLUDE_APRS_WX_H_
#define INCLUDE_APRS_WX_H_

#include "drivers/tx20.h"
#include <stdint.h>

void SendWXFrame(Anemometer* input, float temperatura, unsigned cisnienie);
void SendWXFrameToBuffer(Anemometer* input, float temperatura, unsigned cisnienie, uint8_t* buffer, uint16_t buffer_ln, uint16_t* output_ln);


#endif /* INCLUDE_APRS_WX_H_ */
