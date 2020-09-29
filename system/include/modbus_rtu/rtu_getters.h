/*
 * rtu_getters.h
 *
 *  Created on: 29.09.2020
 *      Author: mateusz
 */

#ifndef INCLUDE_MODBUS_RTU_RTU_GETTERS_H_
#define INCLUDE_MODBUS_RTU_RTU_GETTERS_H_

#define RTU_GETTERS_F1_NAME rte_wx_modbus_rtu_f1
#define RTU_GETTERS_F2_NAME rte_wx_modbus_rtu_f2
#define RTU_GETTERS_F3_NAME rte_wx_modbus_rtu_f3
#define RTU_GETTERS_F4_NAME rte_wx_modbus_rtu_f4

int32_t rtu_get_temperature(float* out);
int32_t rtu_get_pressure(float* out);
int32_t rtu_get_wind_direction(uint16_t* out);
int32_t rtu_get_wind_speed(uint16_t* out);
int32_t rtu_get_humidity(int8_t* out);

#endif /* INCLUDE_MODBUS_RTU_RTU_GETTERS_H_ */
