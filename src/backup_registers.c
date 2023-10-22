/*
 * backup_registers.cpp
 *
 *  Created on: Oct 19, 2023
 *      Author: mateusz
 */

#include "backup_registers.h"

#ifdef STM32L471xx
#define REGISTER 			RTC->BKP0R
#define REGISTER_LAST_SLEEP	RTC->BKP1R
#define REGISTER_LAST_WKUP	RTC->BKP2R
#define REGISTER_COUNTERS	RTC->BKP4R
#define REGISTER_LAST_SLTIM	RTC->BKP6R
#endif

#define BACKUP_REG_INHIBIT_PWR_SWITCH_PERIODIC_H 	1u
#define BACKUP_REG_ALL_PWRSAVE_STATES_BITMASK 		(0xFFu << 2)

static void backup_reg_unclock(void) {
	// enable access to backup domain
	PWR->CR1 |= PWR_CR1_DBP;
}

static void backup_reg_lock(void) {
	PWR->CR1 &= (0xFFFFFFFFu ^ PWR_CR1_DBP);
}

/**
 *
 * @return
 */
uint32_t backup_reg_get_configuration(void) {

	uint32_t out = 0;

#ifdef PARATNC
	out = BKP->DR3;
#endif

#ifdef PARAMETEO
	out = RTC->BKP3R;

#endif

	return out;
}

void  backup_reg_set_configuration(uint32_t value) {
#ifdef PARATNC
	BKP->DR3 = value;
#endif

#ifdef PARAMETEO
	backup_reg_unclock();

	RTC->BKP3R = value;

	backup_reg_lock();
#endif
}

void backup_reg_set_bits_configuration(uint32_t value) {
#ifdef PARATNC
	BKP->DR3 |= value;
#endif

#ifdef PARAMETEO
	// enable access to backup domain
	backup_reg_unclock();

	RTC->BKP3R |= value;

	backup_reg_lock();

#endif
}

void backup_reg_clear_bits_configuration(uint32_t value) {
#ifdef PARATNC
	BKP->DR3 &= (0xFFFF ^ value);
#endif

#ifdef PARAMETEO
	// enable access to backup domain
	backup_reg_unclock();

	RTC->BKP3R &= (0xFFFFFFFFu ^ value);

	backup_reg_lock();

#endif
}

/**
 *
 */
void backup_reg_reset_all_powersave_states(void) {

	backup_reg_unclock();

#ifdef PARAMETEO
	REGISTER &= 0xFFFFFFFFu ^ BACKUP_REG_ALL_PWRSAVE_STATES_BITMASK;
#endif

	backup_reg_lock();

}

int backup_reg_is_in_powersave_state(uint32_t state) {
	int out = 0;
#ifdef PARAMETEO

	if ((REGISTER & BACKUP_REG_ALL_PWRSAVE_STATES_BITMASK) == state) {
		out = 1;
	}
#endif
	return out;
}

void backup_reg_set_powersave_state(uint32_t state) {
#ifdef PARAMETEO
	backup_reg_unclock();

	REGISTER |= state;

	backup_reg_lock();
#endif
}

uint16_t backup_reg_get_powersave_state(void) {
	int out = 0;

#ifdef PARAMETEO
	out = (uint16_t)(REGISTER & BACKUP_REG_ALL_PWRSAVE_STATES_BITMASK);
#endif

	return out;
}

/**
 *
 * @return
 */
uint32_t backup_reg_get_wakeup_counter(void) {

	uint32_t out = 0;

#ifdef PARAMETEO
	out = (uint32_t)((REGISTER_COUNTERS & 0xFFFF0000u) >> 16);
#endif

	return out;
}

void backup_reg_set_wakeup_counter(uint32_t in) {
	backup_reg_unclock();

#ifdef PARAMETEO
	REGISTER_COUNTERS = (REGISTER_COUNTERS & 0x0000FFFFu) | ((in & 0xFFFFu) << 16);
#endif

	backup_reg_lock();
}

/**
 *
 * @return
 */
uint32_t backup_reg_get_sleep_counter(void) {
	uint32_t out = 0;

#ifdef PARAMETEO
	out = (uint16_t)(REGISTER_COUNTERS & 0xFFFFu);
#endif

	return out;
}

void backup_reg_set_sleep_counter(uint32_t in) {
	backup_reg_unclock();

#ifdef PARAMETEO
	REGISTER_COUNTERS = (REGISTER_COUNTERS & 0xFFFF0000u) | (uint16_t)(in & 0xFFFFu);
#endif

	backup_reg_lock();
}


/**
 *
 */
uint32_t backup_reg_get_last_sleep_timestamp(void) {

	uint32_t out = 0;

#ifdef PARAMETEO
	out = REGISTER_LAST_SLEEP;
#endif

	return out;
}

void backup_reg_set_last_sleep_timestamp(void) {

#ifdef PARAMETEO
	backup_reg_unclock();

	REGISTER_LAST_SLEEP = RTC->TR;

	backup_reg_lock();
#endif

}

/**
 *
 */

void backup_reg_reset_inhibit_periodic_pwr_switch(void) {
	backup_reg_unclock();

#ifdef PARAMETEO
	REGISTER &= 0xFFFFFFFFu ^ BACKUP_REG_INHIBIT_PWR_SWITCH_PERIODIC_H;
#endif

	backup_reg_lock();
}

void backup_reg_inhibit_periodic_pwr_switch(void) {
	backup_reg_unclock();

#ifdef PARAMETEO
	REGISTER |= BACKUP_REG_INHIBIT_PWR_SWITCH_PERIODIC_H;
#endif

	backup_reg_lock();

}

uint32_t backup_reg_is_periodic_pwr_switch_inhibited(void) {

	int out = 0;
#ifdef PARAMETEO

	if ((REGISTER & BACKUP_REG_INHIBIT_PWR_SWITCH_PERIODIC_H) != 0) {
		out = 1u;
	}
#endif
	return out;
}

/**
 *
 * @return
 */
uint32_t backup_reg_get_last_wakeup_timestamp(void) {

	uint32_t out = 0;
#ifdef PARAMETEO
	out = REGISTER_LAST_WKUP;
#endif
	return out;
}

void backup_reg_set_last_wakeup_timestamp(void) {

#ifdef PARAMETEO
	backup_reg_unclock();

	REGISTER_LAST_WKUP = RTC->TR;

	backup_reg_lock();
#endif
}

/**
 *
 * @return
 */
uint32_t backup_reg_get_last_sleep_duration(void) {
	uint32_t out = 0;

#ifdef PARAMETEO
	out = REGISTER_LAST_SLTIM;
#endif

	return out;
}


void backup_reg_set_last_sleep_duration(uint32_t in) {
#ifdef PARAMETEO
	backup_reg_unclock();

	REGISTER_LAST_SLTIM = in;

	backup_reg_lock();
#endif
}

