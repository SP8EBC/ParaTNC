/*
 * backup_registers.cpp
 *
 *  Created on: Oct 19, 2023
 *      Author: mateusz
 */

#include "backup_registers.h"

#include "variant.h"

#ifdef STM32L471xx
#define REGISTER 					RTC->BKP0R
#define REGISTER_LAST_SLEEP			RTC->BKP1R
#define REGISTER_LAST_WKUP			RTC->BKP2R
#define REGISTER_COUNTERS			RTC->BKP4R
#define REGISTER_LAST_SLTIM			RTC->BKP6R
#define REGISTER_PACKET_COUNTERS	RTC->BKP7R
#endif

#define BACKUP_REG_INHIBIT_PWR_SWITCH_PERIODIC_H 	1u
#define BACKUP_REG_ALL_PWRSAVE_STATES_BITMASK 		(0xFFu << 2)

// backup registers (ParaTNC)
// 0 ->
// 2 -> boot and hard fault count
// 3 -> controller configuration status
// 4 -> telemetry counter
// 5 ->
// 6 -> weather counters


// backup registers (ParaMETEO)
// 0 -> powersave status
// 1 -> last sleep rtc time
// 2 -> last wakeup rtc time
// 3 -> controller configuration status
// 4 -> wakeup events MSB, sleep events LSB
// 5 -> monitor
// 6 -> last sleep time
// 7 -> weather and telemetry timers & counters

// 7th register map
// xxxxyyAA - telemetry frames counter
// xxxxyAyy - value of packet_tx_meteo_counter limited to 15
// xxxxAyyy - value of packet_tx_meteo_gsm_counter limited to 15
// xxAAyyyy - value of packet_tx_beacon_counter
// Axxxyyyy - checksum

static void backup_reg_unclock(void) {
	// enable access to backup domain
	PWR->CR1 |= PWR_CR1_DBP;
}

static void backup_reg_lock(void) {
	PWR->CR1 &= (0xFFFFFFFFu ^ PWR_CR1_DBP);
}

/**
 * Calculates checksum from register content
 * @param reg
 * @return
 */
inline static uint8_t backup_reg_calculate_checksum(uint32_t reg) {

	uint8_t out = 0u;

	uint32_t temp = 0u;

	temp += (reg & 0xFu);
	temp += ((reg & 0xF0u) >> 4);
	temp += ((reg & 0xF00u) >> 8);
	temp += ((reg & 0xF000u) >> 12);
	temp += ((reg & 0xF0000u) >> 16);
	temp += ((reg & 0xF00000u) >> 20);
	temp += ((reg & 0xF000000u) >> 24);

	temp = ~temp;

	out = (temp & 0xFu);

	return out;
}

inline static uint8_t backup_reg_get_checksum(uint32_t reg) {

	uint8_t out = 0u;

	out = ((reg & 0xF0000000u) >> 28);

	return out;
}

inline static void backup_reg_set_checksum(volatile uint32_t * reg, const uint8_t checksum) {

	if (variant_validate_is_within_ram((const uint32_t)reg) != 0) {
		// clear existing checksum
		(*reg) &= (0xFFFFFFFF ^ 0xF0000000);

		// store new checksum
		(*reg) |= ((checksum & 0xF) << 28);
	}
}

/**
 * Get the value of a register keeping a status which configuration
 * sector is valid, and which is loaded
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

/**
 * Set a register, which contains an information which configuration sector
 * is valid, and which one has been loaded. It is mostly used to reset
 * the register back to zero.
 * @param value
 */
void  backup_reg_set_configuration(uint32_t value) {
	backup_reg_unclock();

#ifdef PARATNC
	BKP->DR3 = value;
#endif

#ifdef PARAMETEO
	RTC->BKP3R = value;
#endif

	backup_reg_lock();

}

/**
 * Set certain bit in the register containing current configuration state.
 * This is used to store a state of all configuration sectors.
 * @param value
 */
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

/**
 * Clears certain bit in the register containing current configuration state.
 * This is used to keep information that CRC checksum of a sector is no longer
 * valid.
 * @param value
 */
void backup_reg_clear_bits_configuration(uint32_t value) {
	// enable access to backup domain
	backup_reg_unclock();

#ifdef PARATNC
	BKP->DR3 &= (0xFFFF ^ value);
#endif

#ifdef PARAMETEO
	RTC->BKP3R &= (0xFFFFFFFFu ^ value);
#endif

	backup_reg_lock();

}

/**
 * Resets all powersave mode flags.
 */
void backup_reg_reset_all_powersave_states(void) {

	backup_reg_unclock();

#ifdef PARAMETEO
	REGISTER &= 0xFFFFFFFFu ^ BACKUP_REG_ALL_PWRSAVE_STATES_BITMASK;
#endif

	backup_reg_lock();

}

/**
 * Checks if controller is currently in given powersave mode using
 * information from dedicated backup register
 * @param state
 * @return one if controller is in given powersave state
 */
int backup_reg_is_in_powersave_state(uint32_t state) {
	int out = 0;
#ifdef PARAMETEO

	if ((REGISTER & BACKUP_REG_ALL_PWRSAVE_STATES_BITMASK) == state) {
		out = 1;
	}
#endif
	return out;
}

/**
 * Set that controller is currently in given powersave state, it should be
 * used along with \link #backup_reg_reset_all_powersave_states
 * @param state
 */
void backup_reg_set_powersave_state(uint32_t state) {
#ifdef PARAMETEO
	backup_reg_unclock();

	REGISTER |= state;

	backup_reg_lock();
#endif
}

/**
 * Returns current powersave state
 * @return
 */
uint16_t backup_reg_get_powersave_state(void) {
	int out = 0;

#ifdef PARAMETEO
	out = (uint16_t)(REGISTER & BACKUP_REG_ALL_PWRSAVE_STATES_BITMASK);
#endif

	return out;
}

/**
 * Return counter value with current number of wakeup events
 * @return
 */
uint32_t backup_reg_get_wakeup_counter(void) {

	uint32_t out = 0;

#ifdef PARAMETEO
	out = (uint32_t)((REGISTER_COUNTERS & 0xFFFF0000u) >> 16);
#endif

	return out;
}

/**
 * Set current value of wakeup events
 * @param in
 */
void backup_reg_set_wakeup_counter(uint32_t in) {
	backup_reg_unclock();

#ifdef PARAMETEO
	REGISTER_COUNTERS = (REGISTER_COUNTERS & 0x0000FFFFu) | ((in & 0xFFFFu) << 16);
#endif

	backup_reg_lock();
}

/**
 * Return counter value with current number of sleep events
 * @return
 */
uint32_t backup_reg_get_sleep_counter(void) {
	uint32_t out = 0;

#ifdef PARAMETEO
	out = (uint16_t)(REGISTER_COUNTERS & 0xFFFFu);
#endif

	return out;
}

/**
 * Set current value of sleep events
 * @param in
 */
void backup_reg_set_sleep_counter(uint32_t in) {
	backup_reg_unclock();

#ifdef PARAMETEO
	REGISTER_COUNTERS = (REGISTER_COUNTERS & 0xFFFF0000u) | (uint16_t)(in & 0xFFFFu);
#endif

	backup_reg_lock();
}


/**
 * Returns a timestamp of last sleep event
 */
uint32_t backup_reg_get_last_sleep_timestamp(void) {

	uint32_t out = 0;

#ifdef PARAMETEO
	out = REGISTER_LAST_SLEEP;
#endif

	return out;
}

/**
 * Stores a timestamp of a sleep event from current RTC time
 */
void backup_reg_set_last_sleep_timestamp(void) {

#ifdef PARAMETEO
	backup_reg_unclock();

	REGISTER_LAST_SLEEP = RTC->TR;

	backup_reg_lock();
#endif

}

/**
 * Disables an inhibition of VBATT_S switching, when controller is in
 * powersave mode in which weather sensors shall be kept powered down.
 */
void backup_reg_reset_inhibit_periodic_pwr_switch(void) {
	backup_reg_unclock();

#ifdef PARAMETEO
	REGISTER &= 0xFFFFFFFFu ^ BACKUP_REG_INHIBIT_PWR_SWITCH_PERIODIC_H;
#endif

	backup_reg_lock();
}

/**
 * Enables an inhibition of VBATT_S switching, when controller is in
 * powersave mode in which weather sensors shall be kept powered down.
 */
void backup_reg_inhibit_periodic_pwr_switch(void) {
	backup_reg_unclock();

#ifdef PARAMETEO
	REGISTER |= BACKUP_REG_INHIBIT_PWR_SWITCH_PERIODIC_H;
#endif

	backup_reg_lock();

}

/**
 * Returns if VBATT_S switching inhibition is currently enabled
 * @return
 */
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
 * gets last wakeup timestamp
 * @return
 */
uint32_t backup_reg_get_last_wakeup_timestamp(void) {

	uint32_t out = 0;
#ifdef PARAMETEO
	out = REGISTER_LAST_WKUP;
#endif
	return out;
}

/**
 * Stores a timestamp of a wakeup event from current RTC time
 */
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

/**
 * Set register containing packet counters, used when a configuration
 * is reset to default
 */
void backup_reg_reset_counters(void) {
#ifdef PARAMETEO
	backup_reg_unclock();

	REGISTER_PACKET_COUNTERS = 0u;

	backup_reg_lock();
#endif
}

/**
 * Gets last telemetry frames counter stored in backup registers
 * @return last telemetry counter or zero if backup registers contains crap
 */
uint8_t backup_reg_get_telemetry(void) {

	uint8_t out = 0u;

#ifdef PARAMETEO

	uint32_t reg_value = REGISTER_PACKET_COUNTERS;

	// calculate checksum from register value
	uint8_t calculated_checksum = backup_reg_calculate_checksum(reg_value);

	uint8_t checksum_from_reg = backup_reg_get_checksum(reg_value);

	// check if checksum is ok
	if (calculated_checksum == checksum_from_reg) {
		out = (uint8_t)(reg_value & 0xFFu);
	}
	else {
		; // return zero if checksum is wrong
	}

#endif

	return out;
}

/**
 *
 */
void backup_reg_set_telemetry(uint16_t in) {
	backup_reg_unclock();

	if (in > 255) {
		in = 0;
	}

	const uint8_t narrowed_in = (uint8_t)(in & 0xFFu);

#ifdef PARAMETEO

	// get current value
	uint32_t reg_value = REGISTER_PACKET_COUNTERS;

	// clear current telemetry counter using the bitmask
	reg_value &= (0xFFFFFFFFu ^ 0xFFu);

	// store updated value
	reg_value |= narrowed_in;

	// recalculate checksum
	const uint8_t new_checksum = backup_reg_calculate_checksum(reg_value);

	// store new checksum
	backup_reg_set_checksum(&reg_value, new_checksum);

	REGISTER_PACKET_COUNTERS = reg_value;
#endif

	backup_reg_lock();

}

void backup_reg_get_packet_counters(uint8_t * beacon_counter, uint8_t * meteo_counter, uint8_t * meteo_gsm_counter) {
#ifdef PARAMETEO
	uint32_t reg_value = REGISTER_PACKET_COUNTERS;

	// calculate checksum from register value
	uint8_t calculated_checksum = backup_reg_calculate_checksum(reg_value);

	uint8_t checksum_from_reg = backup_reg_get_checksum(reg_value);

	// check if checksum is ok
	if (calculated_checksum == checksum_from_reg) {
		*meteo_counter 		= ((reg_value & 0x00000F00u) >> 8);
		*meteo_gsm_counter 	= ((reg_value & 0x0000F000u) >> 12);
		*beacon_counter 	= ((reg_value & 0x00FF0000u) >> 16);
	}
	else {
		// if it is not ok revert to default values
		*beacon_counter = 0u;
		*meteo_counter = 2u;
		*meteo_gsm_counter = 0u;

		// and save it back into backup register
		backup_reg_set_packet_counters(*beacon_counter, *meteo_counter, *meteo_gsm_counter);
	}
#endif
}

void backup_reg_set_packet_counters(uint8_t beacon_counter, uint8_t meteo_counter, uint8_t meteo_gsm_counter) {
#ifdef PARAMETEO
	volatile uint32_t reg_value = REGISTER_PACKET_COUNTERS;

	// clear existing content
	reg_value &= (0xFFFFFFFFu ^ 0x00FFFF00u);

	// check if meteo_counter doesn't overflow
	if (meteo_counter > 15) {
		meteo_counter = 15;
	}

	if (meteo_gsm_counter > 15) {
		meteo_gsm_counter = 15;
	}

	// put new values
	reg_value |= ((beacon_counter << 16) | (meteo_gsm_counter << 12) | (meteo_counter << 8));

	// calculate new checksum
	const uint8_t new_checksum = backup_reg_calculate_checksum(reg_value);

	// put new checksum value
	backup_reg_set_checksum(&reg_value, new_checksum);

	backup_reg_unclock();

	REGISTER_PACKET_COUNTERS = reg_value;

	backup_reg_lock();

#endif
}
