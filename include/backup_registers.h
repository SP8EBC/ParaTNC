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
	return REGISTER_MONITOR;
}

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



#endif /* BACKUP_REGISTERS_H_ */
