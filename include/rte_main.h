
#ifndef RTE_MAIN_H_
#define RTE_MAIN_H_

#include <stdint.h>
#include "stored_configuration_nvm/config_data.h"
#include "message.h"

//!< Set immediately after waking up in RTC interrupt handler
#define RTE_MAIN_WOKEN_UP_RTC_INTERRUPT		1u

//!< Set after exiting from RTC interrupt, but before reinitializing clocks
#define RTE_MAIN_WOKEN_UP_AFTER_RTC_IT		2u

//!< Set after everything was reinitialized from
#define RTE_MAIN_WOKEN_UP_EXITED			4u

extern message_t rte_main_received_message;
extern message_source_t rte_main_received_message_source;

//!< Trigger preparing and sending ACK
extern uint8_t rte_main_trigger_message_ack;

extern uint8_t rte_main_trigger_gsm_aprsis_counters_packet;

extern uint8_t rte_main_trigger_gsm_loginstring_packet;

extern uint8_t rte_main_trigger_gsm_telemetry_values;

extern uint8_t rte_main_trigger_gsm_telemetry_descriptions;

extern uint8_t rte_main_trigger_gsm_status;

//!< Trigger some reinitialization after waking up from deep sleep
extern uint8_t rte_main_woken_up;

extern uint8_t rte_main_reboot_req;

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

extern config_data_powersave_mode_t rte_main_curret_powersave_mode;

#endif
