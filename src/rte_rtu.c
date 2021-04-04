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

uint16_t rte_rtu_number_of_successfull_serial_comm = 0;

rtu_register_data_t RTU_GETTERS_F1_NAME;

rtu_register_data_t RTU_GETTERS_F2_NAME;

rtu_register_data_t RTU_GETTERS_F3_NAME;

rtu_register_data_t RTU_GETTERS_F4_NAME;

rtu_register_data_t RTU_GETTERS_F5_NAME;

rtu_register_data_t RTU_GETTERS_F6_NAME;

rtu_exception_t rte_rtu_last_modbus_exception;
uint32_t rte_rtu_last_modbus_exception_timestamp;
uint32_t rte_rtu_last_modbus_rx_error_timestamp;
rtu_pool_queue_t rte_rtu_pool_queue;


void rte_rtu_init(void) {
	rte_rtu_last_modbus_exception_timestamp = 0;
	rte_rtu_last_modbus_rx_error_timestamp = 0;
	rte_rtu_last_modbus_exception = RTU_EXCEPTION_OK;
}
