/*
 * wx.h
 *
 *  Created on: 28.05.2017
 *      Author: mateusz
 */

#ifndef INCLUDE_APRS_WX_H_
#define INCLUDE_APRS_WX_H_

#include "drivers/tx20.h"

void SendWXFrame(Anemometer* input, float temperatura, unsigned cisnienie);


#endif /* INCLUDE_APRS_WX_H_ */
