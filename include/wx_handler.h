/*
 * wx_handler.h
 *
 *  Created on: 26.01.2019
 *      Author: mateusz
 */

#ifndef WX_HANDLER_H_
#define WX_HANDLER_H_

#include <stdint.h>
#include "config_data.h"

extern uint32_t wx_last_good_temperature_time;
extern uint32_t wx_last_good_wind_time;
extern uint8_t wx_force_i2c_sensor_reset;

#define WX_HANDLER_PARAMETER_RESULT_TEMPERATURE		(1 << 1)
#define WX_HANDLER_PARAMETER_RESULT_PRESSURE		(1 << 2)
#define WX_HANDLER_PARAMETER_RESULT_HUMIDITY		(1 << 3)
#define WX_HANDLER_PARAMETER_RESULT_WIND			(1 << 4)
#define WX_HANDLER_PARAMETER_RESULT_TEMP_INTERNAL	(1 << 5)

void wx_get_all_measurements(const config_data_wx_sources_t * const config_sources, const config_data_mode_t * const config_mode, const config_data_umb_t * const config_umb, const config_data_rtu_t * const config_rtu);
int32_t wx_get_bme280_temperature_pressure_humidity(float * const temperature, float * const pressure, int8_t * const humidity);
void wx_pool_anemometer(const config_data_wx_sources_t * const config_sources, const config_data_mode_t * const config_mode, const config_data_umb_t * const config_umb, const config_data_rtu_t * const config_rtu);
void wx_check_force_i2c_reset(void);


#endif /* WX_HANDLER_H_ */
