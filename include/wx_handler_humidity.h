/*
 * wx_handler_humidity.h
 *
 *  Created on: Apr 15, 2021
 *      Author: mateusz
 */

#ifndef WX_HANDLER_HUMIDITY_H_
#define WX_HANDLER_HUMIDITY_H_

#include "config_data.h"

int32_t wx_get_humidity_measurement(const config_data_wx_sources_t * const config_sources, const config_data_mode_t * const config_mode, const config_data_umb_t * const config_umb);
int32_t wx_get_humidity_bme280(int8_t * const humidity);

#endif /* WX_HANDLER_HUMIDITY_H_ */
