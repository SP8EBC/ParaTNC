/*
 * rte_rtu.h
 *
 *  Created on: Oct 30, 2020
 *      Author: mateusz
 */

#ifndef RTE_RTU_H_
#define RTE_RTU_H_

#include "station_config.h"

//#ifdef _MODBUS_RTU
#include "modbus_rtu/rtu_configuration.h"
#include "modbus_rtu/rtu_register_data_t.h"
#include "modbus_rtu/rtu_exception_t.h"
#include "modbus_rtu/rtu_getters.h"
#include "modbus_rtu/rtu_pool_queue_t.h"
//#endif

#include <stdint.h>

extern uint8_t rte_rtu_number_of_serial_io_errors;

extern uint16_t rte_rtu_number_of_successfull_serial_comm;

//#ifdef _MODBUS_RTU

extern rtu_register_data_t rte_wx_modbus_rtu_f1;

extern rtu_register_data_t rte_wx_modbus_rtu_f2;

extern rtu_register_data_t rte_wx_modbus_rtu_f3;

extern rtu_register_data_t rte_wx_modbus_rtu_f4;

extern rtu_register_data_t rte_wx_modbus_rtu_f5;

extern rtu_register_data_t rte_wx_modbus_rtu_f6;

extern rtu_exception_t rte_rtu_last_modbus_exception;
extern uint32_t rte_rtu_last_modbus_rx_error_timestamp;
extern uint32_t rte_rtu_last_modbus_exception_timestamp;
extern rtu_pool_queue_t rte_rtu_pool_queue;

//#endif

void rte_rtu_init(void);

#endif /* RTE_RTU_H_ */
