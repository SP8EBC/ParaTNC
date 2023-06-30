/*
 * rtu_getters.h
 *
 *  Created on: 29.09.2020
 *      Author: mateusz
 */

#ifndef INCLUDE_MODBUS_RTU_RTU_GETTERS_H_
#define INCLUDE_MODBUS_RTU_RTU_GETTERS_H_

#include <stdint.h>
#include <stored_configuration_nvm/config_data.h>

int32_t rtu_get_temperature(int16_t* out, const config_data_rtu_t * const config);
int32_t rtu_get_pressure(float* out, const config_data_rtu_t * const config);
int32_t rtu_get_wind_direction(uint16_t* out, const config_data_rtu_t * const config);
int32_t rtu_get_wind_speed(uint16_t* out, const config_data_rtu_t * const config);
int32_t rtu_get_wind_gusts(uint16_t* out, const config_data_rtu_t * const config);
int32_t rtu_get_humidity(int8_t* out, const config_data_rtu_t * const config);

void rtu_get_raw_values_string(char* out, uint16_t out_buffer_ln, uint8_t* generated_string_ln);

#endif /* INCLUDE_MODBUS_RTU_RTU_GETTERS_H_ */
