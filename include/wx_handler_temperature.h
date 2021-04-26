/*
 * wx_handler_temperature.h
 *
 *  Created on: Apr 14, 2021
 *      Author: mateusz
 */

#ifndef WX_HANDLER_TEMPERATURE_H_
#define WX_HANDLER_TEMPERATURE_H_

#include "config_data.h"

int32_t wx_get_temperature_measurement(const config_data_wx_sources_t * const config_sources, const config_data_mode_t * const config_mode, const config_data_umb_t * const config_umb, const config_data_rtu_t * const config_rtu);
int32_t wx_get_temperature_dallas(void);
int32_t wx_get_temperature_ms5611(float * const temperature);
int32_t wx_get_temperature_bme280(float * const temperature);

#endif /* WX_HANDLER_TEMPERATURE_H_ */
