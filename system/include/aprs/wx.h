/*
 * wx.h
 *
 *  Created on: 28.05.2017
 *      Author: mateusz
 */

#ifndef INCLUDE_APRS_WX_H_
#define INCLUDE_APRS_WX_H_

#include "drivers/tx20.h"
#include "station_config.h"
#include <stdint.h>

#ifdef _ANEMOMETER_TX20

void SendWXFrame(Anemometer* input, float temperatura, float cisnienie);
void SendWXFrameToBuffer(Anemometer* input, float temperatura, float cisnienie, uint8_t* buffer, uint16_t buffer_ln, uint16_t* output_ln);
#else

void SendWXFrame(uint16_t windspeed, uint16_t windgusts, uint16_t winddirection, float temperatura, float cisnienie);
void SendWXFrameToBuffer(uint16_t windspeed, uint16_t windgusts, uint16_t winddirection, float temperatura, float cisnienie, uint8_t* buffer, uint16_t buffer_ln, uint16_t* output_ln);

#endif

#endif /* INCLUDE_APRS_WX_H_ */
