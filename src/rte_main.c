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

//!< message received from APRS-IS or RF radio network
message_t rte_main_received_message;

//!< message to be send via APRS-IS or RF radio network
message_t rte_main_message_for_transmitting;

//!< Trigger message ACK preparing and sending
uint8_t rte_main_trigger_message_ack = 0;

uint8_t rte_main_trigger_send_message = 0;

#ifdef PARAMETEO
//!< Set to one if reboot is scheduled by diagnostics
uint8_t rte_main_reboot_scheduled_diag = 0;

uint8_t rte_main_trigger_gsm_aprsis_counters_packet = 0;

//!< Trigger sending status packet with received APRS is login string
uint8_t rte_main_trigger_gsm_loginstring_packet = 0;

//!< Trigger packet with telemetry value AFTER it is prepared
uint8_t rte_main_trigger_gsm_telemetry_values = 0;

//!< Trigger set of packets with telemetry description
uint8_t rte_main_trigger_gsm_telemetry_descriptions = 0;

//!<
uint8_t rte_main_trigger_gsm_status = 0;

//!<
uint8_t rte_main_trigger_gsm_event_log = 0;

//!<
uint8_t rte_main_trigger_radio_event_log = 0;

//!< Trigger some reinitialization after waking up from deep sleep
uint8_t rte_main_woken_up = 0;

uint8_t rte_main_woken_up_for_telemetry = 0;

//!< Current battery voltage as 10mV increments
uint16_t rte_main_battery_voltage;

//!< Average battery voltage as 10mV increments, lenght configured by VBATT_HISTORY_LN
uint16_t rte_main_average_battery_voltage = 0;

uint16_t rte_main_wakeup_count = 0;

uint16_t rte_main_going_sleep_count = 0;

uint32_t rte_main_last_sleep_master_time = 0;

//!< Set to one after waking up from L7 / L6 powersave mode and
uint8_t rte_main_reset_gsm_modem = 0xFFu;

uint8_t rte_main_reset_modbus_rtu = 0xFFu;

config_data_powersave_mode_t rte_main_curret_powersave_mode;

//!< Array to extract events from NVM into. *2 is applied to have more room for data sent to API
event_log_exposed_t rte_main_exposed_events[MAIN_HOW_MANY_EVENTS_SEND_REPORT * 3];

nvm_event_result_stats_t rte_main_events_extracted_for_api_stat = {0u};

#endif

