/*
 * rte_main.c
 *
 *  Created on: 16.04.2019
 *      Author: mateusz
 */

#include "rte_main.h"

#include "station_config_target_hw.h"

uint8_t rte_main_reboot_req = 0;

uint8_t rte_main_boot_cycles = 0, rte_main_hard_faults = 0;

uint8_t rte_main_trigger_status = 0;

uint8_t rte_main_trigger_wx_packet = 0;

#ifdef PARAMETEO
uint8_t rte_main_trigger_gsm_status_packet = 0;

//!< Trigger sending status packet with received APRS is login string
uint8_t rte_main_trigger_gsm_loginstring_packet = 0;

//!< Trigger packet with telemetry value AFTER it is prepared
uint8_t rte_main_trigger_gsm_telemetry_values = 0;

//!< Trigger set of packets with telemetry description
uint8_t rte_main_trigger_gsm_telemetry_descriptions = 0;

//!<
uint8_t rte_main_trigger_gsm_status_gsm = 0;

//!< Trigger some reinitialization after waking up from deep sleep
uint8_t rte_main_woken_up = 0;

//!< Current battery voltage as 10mV increments
uint16_t rte_main_battery_voltage;

//!< Average battery voltage as 10mV increments, lenght configured by VBATT_HISTORY_LN
uint16_t rte_main_average_battery_voltage = 0;

uint16_t rte_main_wakeup_count = 0;

uint16_t rte_main_going_sleep_count = 0;

uint32_t rte_main_last_sleep_master_time = 0;

//!< Set to one after waking up from L7 / L6 powersave mode and
uint8_t rte_main_reset_gsm_modem = 0;

config_data_powersave_mode_t rte_main_curret_powersave_mode;
#endif

