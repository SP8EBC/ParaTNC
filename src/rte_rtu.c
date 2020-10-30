/*
 * rte_rtu.c
 *
 *  Created on: Oct 30, 2020
 *      Author: mateusz
 */

#include <rte_rtu.h>

/**
 * This counts the consecutive serial I/O errors to trigger the modbur-rtu status frame
 */
uint8_t rte_rtu_number_of_serial_io_errors = 0;

uint16_t rte_rtu_number_of_serial_successfull_comm = 0;

#ifdef _MODBUS_RTU

	#if defined(_RTU_SLAVE_ID_1) && (_RTU_SLAVE_FUNC_1 == 0x03 || _RTU_SLAVE_FUNC_1 == 0x04)
		rtu_register_data_t RTU_GETTERS_F1_NAME;
	#endif

	#if defined(_RTU_SLAVE_ID_2) && (_RTU_SLAVE_FUNC_2 == 0x03 || _RTU_SLAVE_FUNC_2 == 0x04)
		rtu_register_data_t RTU_GETTERS_F2_NAME;
	#endif

	#if defined(_RTU_SLAVE_ID_3) && (_RTU_SLAVE_FUNC_3 == 0x03 || _RTU_SLAVE_FUNC_3 == 0x04)
		rtu_register_data_t RTU_GETTERS_F3_NAME;
	#endif

	#if defined(_RTU_SLAVE_ID_4) && (_RTU_SLAVE_FUNC_4 == 0x03 || _RTU_SLAVE_FUNC_4 == 0x04)
		rtu_register_data_t RTU_GETTERS_F4_NAME;
	#endif

	#if defined(_RTU_SLAVE_ID_5) && (_RTU_SLAVE_FUNC_5 == 0x03 || _RTU_SLAVE_FUNC_5 == 0x04)
		rtu_register_data_t RTU_GETTERS_F5_NAME;
	#endif

	#if defined(_RTU_SLAVE_ID_6) && (_RTU_SLAVE_FUNC_6 == 0x03 || _RTU_SLAVE_FUNC_6 == 0x04)
		rtu_register_data_t RTU_GETTERS_F6_NAME;
	#endif

rtu_exception_t rte_rtu_last_modbus_exception;
uint32_t rte_rtu_last_modbus_exception_timestamp;
uint32_t rte_rtu_last_modbus_rx_error_timestamp;
rtu_pool_queue_t rte_rtu_pool_queue;

#endif

void rte_rtu_init(void) {
#ifdef _MODBUS_RTU
	rte_rtu_last_modbus_exception_timestamp = 0;
	rte_rtu_last_modbus_rx_error_timestamp = 0;
	rte_rtu_last_modbus_exception = RTU_EXCEPTION_OK;
#endif
}
