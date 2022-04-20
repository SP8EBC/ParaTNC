/*
 * rte_main.c
 *
 *  Created on: 16.04.2019
 *      Author: mateusz
 */

#include "rte_main.h"

uint8_t rte_main_reboot_req = 0;

uint8_t rte_main_boot_cycles = 0, rte_main_hard_faults = 0;

uint8_t rte_main_trigger_status = 0;
uint8_t rte_main_trigger_modbus_status = 0;

uint8_t rte_main_trigger_wx_packet = 0;


#ifdef PARAMETEO
uint16_t rte_main_battery_voltage;

uint16_t rte_main_average_battery_voltage = 0;

uint16_t rte_main_wakeup_count = 0;

uint16_t rte_main_going_sleep_count = 0;

uint32_t rte_main_last_sleep_master_time = 0;
#endif

