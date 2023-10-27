/*
 * wx.h
 *
 *  Created on: 28.05.2017
 *      Author: mateusz
 */

#ifndef INCLUDE_APRS_WX_H_
#define INCLUDE_APRS_WX_H_

#include "station_config.h"
#include <stdint.h>


void SendWXFrame(uint16_t windspeed, uint16_t windgusts, uint16_t winddirection, float temperatura, float cisnienie, uint8_t humidity);
void SendWXFrameToKissBuffer(uint16_t windspeed, uint16_t windgusts, uint16_t winddirection, float temperatura, float cisnienie, uint8_t humidity, uint8_t* buffer, uint16_t buffer_ln, uint16_t* output_ln);

#endif /* INCLUDE_APRS_WX_H_ */
