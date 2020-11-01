/*
 * rte_rtu.h
 *
 *  Created on: Oct 30, 2020
 *      Author: mateusz
 */

#ifndef RTE_RTU_H_
#define RTE_RTU_H_

#include "station_config.h"

#ifdef _MODBUS_RTU
#include "modbus_rtu/rtu_configuration.h"
#include "modbus_rtu/rtu_register_data_t.h"
#include "modbus_rtu/rtu_exception_t.h"
#include "modbus_rtu/rtu_getters.h"
#include "modbus_rtu/rtu_pool_queue_t.h"
#endif

extern uint8_t rte_rtu_number_of_serial_io_errors;

extern uint16_t rte_rtu_number_of_successfull_serial_comm;

#ifdef _MODBUS_RTU

	#if defined(_RTU_SLAVE_ID_1) && (_RTU_SLAVE_FUNC_1 == 0x03 || _RTU_SLAVE_FUNC_1 == 0x04)
		extern rtu_register_data_t RTU_GETTERS_F1_NAME;
	#endif

	#if defined(_RTU_SLAVE_ID_2) && (_RTU_SLAVE_FUNC_2 == 0x03 || _RTU_SLAVE_FUNC_2 == 0x04)
		extern rtu_register_data_t RTU_GETTERS_F2_NAME;
	#endif

	#if defined(_RTU_SLAVE_ID_3) && (_RTU_SLAVE_FUNC_3 == 0x03 || _RTU_SLAVE_FUNC_3 == 0x04)
		extern rtu_register_data_t RTU_GETTERS_F3_NAME;
	#endif

	#if defined(_RTU_SLAVE_ID_4) && (_RTU_SLAVE_FUNC_4 == 0x03 || _RTU_SLAVE_FUNC_4 == 0x04)
		extern rtu_register_data_t RTU_GETTERS_F4_NAME;
	#endif

	#if defined(_RTU_SLAVE_ID_5) && (_RTU_SLAVE_FUNC_5 == 0x03 || _RTU_SLAVE_FUNC_5 == 0x04)
		extern rtu_register_data_t RTU_GETTERS_F5_NAME;
	#endif

	#if defined(_RTU_SLAVE_ID_6) && (_RTU_SLAVE_FUNC_6 == 0x03 || _RTU_SLAVE_FUNC_6 == 0x04)
		extern rtu_register_data_t RTU_GETTERS_F6_NAME;
	#endif

extern rtu_exception_t rte_rtu_last_modbus_exception;
extern uint32_t rte_rtu_last_modbus_rx_error_timestamp;
extern uint32_t rte_rtu_last_modbus_exception_timestamp;
extern rtu_pool_queue_t rte_rtu_pool_queue;

#endif

void rte_rtu_init(void);

#endif /* RTE_RTU_H_ */
