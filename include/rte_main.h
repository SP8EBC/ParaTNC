
#ifndef RTE_MAIN_H_
#define RTE_MAIN_H_

#include <stdint.h>
#include "stored_configuration_nvm/config_data.h"
#include "message.h"
#include "etc/misc_config.h"
#include "event_log_t.h"
#include "nvm/nvm_t.h"

#include <FreeRTOS.h>
#include <task.h>

//! Lenght of a buffer for KISS diagnostic request
#define MAIN_KISS_FROM_MESSAGE_LEN		33

//!< Set immediately after waking up in RTC interrupt handler
#define RTE_MAIN_WOKEN_UP_RTC_INTERRUPT			1u

//!< Set after exiting from RTC interrupt, but before reinitializing clocks
#define RTE_MAIN_WOKEN_UP_AFTER_LAST_SLEEP		2u

//!< Set after everything was reinitialized from
#define RTE_MAIN_WOKEN_UP_EXITED				4u

#define RTE_MAIN_GO_TO_INTERMEDIATE_SLEEP		8u

#define RTE_MAIN_REBOOT_SCHEDULED_APRSMSG	1u

// clang-format off
#define RTE_MAIN_GET_FOR_ASSERT()	(rte_main_trigger_message_ack & 0x1u) | \
									((rte_main_trigger_send_message & 0x1u) << 1) | \
									((rte_main_trigger_gsm_aprsis_counters_packet & 0x1u) << 2) | \
									((rte_main_trigger_gsm_loginstring_packet & 0x1u) << 3) | \
									((rte_main_trigger_gsm_status & 0x1u) << 4) | \
									((rte_main_trigger_gsm_event_log & 0x1u) << 5) | \
									((rte_main_trigger_radio_event_log & 0x1u) << 6) | \
									((rte_main_trigger_wx_packet & 0x1u) << 7) \
// clang-format on

typedef struct rte_main_tasks_load_t
{
	uint32_t task_main;
	uint32_t task_powersave;
	uint32_t task_one_sec;
	uint32_t task_two_sec;
	uint32_t task_ten_sec;
	uint32_t task_one_min;
	uint32_t tev_serial_kiss;
	uint32_t tev_serial_kiss_tx;
	uint32_t tev_serial_gsm_rx;
	uint32_t tev_serial_gsm_tx;
	uint32_t tev_apris_trig;
	uint32_t tev_ntp_api;
	uint32_t tev_radio_message;
	uint32_t idle;
}rte_main_tasks_cpuload_t;

typedef struct rte_main_tasks_state_t
{
	uint32_t master_time;
	eTaskState task_main;
	eTaskState task_powersave;
	eTaskState task_one_sec;
	eTaskState task_two_sec;
	eTaskState task_ten_sec;
	eTaskState task_one_min;
	eTaskState tev_serial_kiss;
	eTaskState tev_serial_kiss_tx;
	eTaskState tev_serial_gsm_rx;
	eTaskState tev_serial_gsm_tx;
	eTaskState tev_apris_trig;
	eTaskState tev_ntp_api;
	eTaskState tev_radio_message;
	eTaskState idle;
}rte_main_tasks_state_t;

extern message_t rte_main_received_message;

extern message_t rte_main_message_for_transmitting;

//!< Trigger preparing and sending ACK
extern uint8_t rte_main_trigger_message_ack;

extern uint8_t rte_main_trigger_send_message;

extern uint8_t rte_main_trigger_gsm_aprsis_counters_packet;

extern uint8_t rte_main_trigger_gsm_loginstring_packet;

extern uint8_t rte_main_trigger_gsm_telemetry_values;

extern uint8_t rte_main_trigger_gsm_telemetry_descriptions;

extern uint8_t rte_main_trigger_gsm_status;

extern uint8_t rte_main_trigger_gsm_event_log;

extern uint8_t rte_main_trigger_radio_event_log;

extern uint8_t rte_main_trigger_wx_packet;

//!< Triggers additional check if ADC has properly reinitialized and conversion is working
extern uint8_t rte_main_check_adc;

//!< Trigger some reinitialization after waking up from deep sleep
extern uint8_t rte_main_woken_up;

extern uint8_t rte_main_woken_up_for_telemetry;

extern uint8_t rte_main_reboot_req;

extern uint8_t rte_main_reboot_scheduled_diag;

extern uint8_t rte_main_boot_cycles, rte_main_hard_faults;
extern uint32_t rte_main_hardfault_lr, rte_main_hardfault_pc;

extern uint8_t rte_main_trigger_status;

extern uint8_t rte_main_trigger_wx_packet;

extern uint16_t rte_main_battery_voltage;
extern uint16_t rte_main_average_battery_voltage;

extern uint16_t rte_main_wakeup_count;

extern uint16_t rte_main_going_sleep_count;

extern uint32_t rte_main_last_sleep_master_time;

extern uint8_t rte_main_reset_gsm_modem;

extern uint8_t rte_main_reset_modbus_rtu;

extern config_data_powersave_mode_t rte_main_curret_powersave_mode;

//!< Array to extract events from NVM into. *2 is applied to have more room for data sent to API
extern event_log_exposed_t rte_main_exposed_events[MAIN_HOW_MANY_EVENTS_SEND_REPORT * 3];

extern nvm_event_result_stats_t rte_main_events_extracted_for_api_stat;

//! KISS (diagnostic) request decoded from APRS message
extern uint8_t rte_main_kiss_from_message[MAIN_KISS_FROM_MESSAGE_LEN];

extern uint8_t rte_main_kiss_from_message_ln;

//! binary response to DID request from APRS message
extern uint8_t rte_main_kiss_response_message[32];

extern rte_main_tasks_cpuload_t rte_main_load;

extern rte_main_tasks_state_t rte_main_state;

#endif
