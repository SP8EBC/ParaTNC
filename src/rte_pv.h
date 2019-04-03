/*
 * rte_pv.h
 *
 *  Created on: 18.03.2019
 *      Author: mateusz
 */

#ifndef RTE_PV_H_
#define RTE_PV_H_

#include <stdint.h>
#include "./ve_direct_protocol/average_struct.h"
#include "./ve_direct_protocol/raw_struct.h"

extern ve_direct_average_struct rte_pv_average;
extern uint8_t rte_pv_sys_voltage;
extern ve_direct_raw_struct rte_pv_struct;

extern int16_t rte_pv_battery_current;
extern uint16_t rte_pv_battery_voltage;
extern uint16_t rte_pv_cell_voltage;
extern uint16_t rte_pv_load_current;

#endif /* RTE_PV_H_ */
