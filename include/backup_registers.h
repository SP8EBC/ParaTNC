/*
 * backup_registers.h
 *
 *  Created on: Oct 19, 2023
 *      Author: mateusz
 */

#ifndef BACKUP_REGISTERS_H_
#define BACKUP_REGISTERS_H_

#ifdef STM32L471xx
#include <stm32l4xx.h>
#endif

#ifdef STM32F10X_MD_VL
#include <stm32f10x.h>
#endif

#define REGISTER_MONITOR	RTC->BKP5R

/**
 * Inline used to trace an execution flow across main for(;;) loop and some
 * powersaving functions. In case of software fault it's value may help to trace
 * at witch point the crash has occured
 */
inline void backup_reg_set_monitor(int8_t bit) {
#ifdef STM32L471xx
	// enable access to backup domain
	PWR->CR1 |= PWR_CR1_DBP;

	if (bit >= 0) {
		REGISTER_MONITOR |= (1 << bit);

	}
	else {
		REGISTER_MONITOR = 0;
	}

	PWR->CR1 &= (0xFFFFFFFF ^ PWR_CR1_DBP);
#endif
}

inline uint32_t backup_reg_get_monitor(void) {
#ifdef STM32L471xx
	return REGISTER_MONITOR;
#else
	return 0;
#endif
}

#define BACKUP_REG_ASSERT_CONCURENT_ACCES_APRSIS_WX					(1U)
#define BACKUP_REG_ASSERT_CONCURENT_ACCES_APRSIS_BEACON				(1U << 1U)
#define BACKUP_REG_ASSERT_CONCURENT_ACCES_APRSIS_TELEMETRY			(1U << 2U)
#define BACKUP_REG_ASSERT_CONCURENT_ACCES_APRSIS_DESCR				(1U << 3U)
#define BACKUP_REG_ASSERT_CONCURENT_ACCES_APRSIS_IGATE				(1U << 4U)
#define BACKUP_REG_ASSERT_CONCURENT_ACCES_APRSIS_CNTRS				(1U << 5U)
#define BACKUP_REG_ASSERT_CONCURENT_ACCES_APRSIS_LOGINSTRING		(1U << 6U)
#define BACKUP_REG_ASSERT_CONCURENT_ACCES_APRSIS_OTHER				(1U << 7U)
#define BACKUP_REG_ASSERT_ERASE_FAIL_WHILE_STORING_EVENT			(1U << 8U)
#define BACKUP_REG_ASSERT_GENERAL_FAIL_FROM_NVM_EVENT				(1U << 9U)
#define BACKUP_REG_ASSERT_GET_VBAT_SYNCHRONOUS_TOO_LONG				(1U << 10U)
#define BACKUP_REG_ASSERT_GPRS_CONFIG								(1U << 12U)

uint32_t backup_reg_get_configuration(void);
void  backup_reg_set_configuration(uint32_t value);
void backup_reg_set_bits_configuration(uint32_t value);
void backup_reg_clear_bits_configuration(uint32_t value);

void backup_reg_reset_all_powersave_states(void);
int backup_reg_is_in_powersave_state(uint32_t state);
void backup_reg_set_powersave_state(uint32_t state);
uint16_t backup_reg_get_powersave_state(void);

uint32_t backup_reg_get_wakeup_counter(void);
void backup_reg_set_wakeup_counter(uint32_t in);

uint32_t backup_reg_get_sleep_counter(void);
void backup_reg_set_sleep_counter(uint32_t in);

uint32_t backup_reg_get_last_sleep_timestamp(void);
void backup_reg_set_last_sleep_timestamp(void);

uint32_t backup_reg_get_last_wakeup_timestamp(void);
void backup_reg_set_last_wakeup_timestamp(void);

void backup_reg_reset_inhibit_periodic_pwr_switch(void);
void backup_reg_inhibit_periodic_pwr_switch(void);
uint32_t backup_reg_is_periodic_pwr_switch_inhibited(void);

uint32_t backup_reg_get_last_sleep_duration(void);
void backup_reg_set_last_sleep_duration(uint32_t);

void backup_reg_reset_counters(void);

uint8_t backup_reg_get_telemetry(void);
void backup_reg_set_telemetry(uint16_t);

void backup_reg_get_packet_counters(uint8_t * beacon_counter, uint8_t * meteo_counter, uint8_t * meteo_gsm_counter);
void backup_reg_set_packet_counters(uint8_t beacon_counter, uint8_t meteo_counter, uint8_t meteo_gsm_counter);

void backup_reg_increment_aprsis_check_reset(void);
void backup_reg_increment_weather_measurements_check_reset(void);
void backup_reg_increment_dallas_degraded_reset(void);
void backup_reg_increment_is_rtc_ok_check_reset(void);

uint32_t backup_reg_get_register_reset_check_fail(void);

void backup_assert(uint32_t assert);

uint32_t backup_reg_get_last_restart_date(void);
void backup_reg_set_last_restart_date(void);

void backup_reg_set_event_log_report_sent_aprsis(void);
void backup_reg_reset_event_log_report_sent_aprsis(void);
uint8_t backup_reg_get_event_log_report_sent_aprsis(void);

void backup_reg_set_event_log_report_sent_radio(void);
void backup_reg_reset_event_log_report_sent_radio(void);
uint8_t backup_reg_get_event_log_report_sent_radio(void);

void backup_reg_set_inhibit_log_report_send_api(void);
void backup_reg_reset_inhibit_log_report_send_api(void);
uint8_t backup_reg_get_inhibit_log_report_send_api(void);


#endif /* BACKUP_REGISTERS_H_ */
