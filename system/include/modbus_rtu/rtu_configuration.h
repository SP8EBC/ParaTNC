/*
 * rtu_configuration.h
 *
 *  Created on: 30.09.2020
 *      Author: mateusz
 */

#ifndef INCLUDE_MODBUS_RTU_RTU_CONFIGURATION_H_
#define INCLUDE_MODBUS_RTU_RTU_CONFIGURATION_H_

#include "station_config.h"

//#ifdef _MODBUS_RTU

#define RTU_GETTERS_F1_NAME rte_wx_modbus_rtu_f1
#define RTU_GETTERS_F2_NAME rte_wx_modbus_rtu_f2
#define RTU_GETTERS_F3_NAME rte_wx_modbus_rtu_f3
#define RTU_GETTERS_F4_NAME rte_wx_modbus_rtu_f4
#define RTU_GETTERS_F5_NAME rte_wx_modbus_rtu_f5
#define RTU_GETTERS_F6_NAME rte_wx_modbus_rtu_f6

//#endif

#define RTU_NUMBER_OF_ERRORS_TO_TRIG_STATUS	32

#define RTU_POOL_QUEUE_LENGHT 6

#define MODBUS_RTU_MAX_REGISTERS_AT_ONCE	2	// according to RTU standard 125

#define RTU_MAXIMUM_DATA_LN 64

#define RTU_MAXIMUM_VALUE_AGE	300000

#define RTU_MAX_VALID_WINDSPEED	400	// 40.0m/s
#define RTU_MAX_VALID_TEMPERATURE	60
#define RTU_MIN_VALID_TEMPERATURE	-30
#define RTU_MAX_VALID_PRESSURE	1100
#define RTU_MIN_VALID_PRESSURE	900

#endif /* INCLUDE_MODBUS_RTU_RTU_CONFIGURATION_H_ */
