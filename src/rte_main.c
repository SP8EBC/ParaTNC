/*
 * rte_main.c
 *
 *  Created on: 16.04.2019
 *      Author: mateusz
 */

#include "rte_main.h"

uint8_t rte_main_reboot_req = 0;

uint8_t rte_main_boot_cycles = 0, rte_main_hard_faults = 0;
uint32_t rte_main_hardfault_lr = 0, rte_main_hardfault_pc = 0;

uint8_t rte_main_trigger_status = 0;
uint8_t rte_main_trigger_modbus_status = 0;

