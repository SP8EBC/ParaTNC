/*
 * rte_pv.c
 *
 *  Created on: 18.03.2019
 *      Author: mateusz
 */

#include "rte_pv.h"

ve_direct_average_struct rte_pv_average;
uint8_t rte_pv_sys_voltage;
ve_direct_raw_struct rte_pv_struct;

int16_t rte_pv_battery_current;
uint16_t rte_pv_battery_voltage;
uint16_t rte_pv_cell_voltage;
uint16_t rte_pv_load_current;
