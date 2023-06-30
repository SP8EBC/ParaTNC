/*
 * wx_handler_pressure.h
 *
 *  Created on: Apr 15, 2021
 *      Author: mateusz
 */

#ifndef WX_HANDLER_PRESSURE_H_
#define WX_HANDLER_PRESSURE_H_

#include <stored_configuration_nvm/config_data.h>

int32_t wx_get_pressure_measurement(const config_data_wx_sources_t * const config_sources, const config_data_mode_t * const config_mode, const config_data_umb_t * const config_umb, const config_data_rtu_t * const config_rtu);
int32_t wx_get_pressure_ms5611(float * const pressure);
int32_t wx_get_pressure_bme280(float * const pressure);

#endif /* WX_HANDLER_PRESSURE_H_ */
