/*
 * rtu_configuration.h
 *
 *  Created on: 30.09.2020
 *      Author: mateusz
 */

#ifndef INCLUDE_MODBUS_RTU_RTU_CONFIGURATION_H_
#define INCLUDE_MODBUS_RTU_RTU_CONFIGURATION_H_

#define RTU_GETTERS_F1_NAME rte_wx_modbus_rtu_f1
#define RTU_GETTERS_F2_NAME rte_wx_modbus_rtu_f2
#define RTU_GETTERS_F3_NAME rte_wx_modbus_rtu_f3
#define RTU_GETTERS_F4_NAME rte_wx_modbus_rtu_f4

#define RTU_POOL_QUEUE_LENGHT 4

#define MODBUS_RTU_MAX_REGISTERS_AT_ONCE	8	// according to RTU standard 125

#define RTU_MAXIMUM_DATA_LN 64

#define RTU_MAXIMUM_VALUE_AGE	600000

#endif /* INCLUDE_MODBUS_RTU_RTU_CONFIGURATION_H_ */
