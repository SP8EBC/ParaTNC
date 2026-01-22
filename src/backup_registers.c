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
#define REGISTER_RESET_CHECK_FAIL	RTC->BKP8R
#define REGISTER_ASSERT				RTC->BKP9R
#define REGISTER_LAST_RESTART		RTC->BKP10R
#define REGISTER_PERSISTENT_STATUS	RTC->BKP11R
#endif

#define BACKUP_REG_INHIBIT_PWR_SWITCH_PERIODIC_H 	1u
#define BACKUP_REG_ALL_PWRSAVE_STATES_BITMASK 		(0xFFu << 2)

#define BACKUP_REG_PERSISTENT_APRSIS_LOG_REPORT			(1 << 0)
#define BACKUP_REG_PERSISTENT_RADIO_LOG_REPORT			(1 << 1)
#define BACKUP_REG_PERSISTENT_INHIBIT_API_LOG_REPORT	(1 << 2)

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
// 8 -> counters of resets caused by validation checks failures
// 9 -> assert register
// 10-> last restart RTC date
// 11-> persistent program status

// 7th register map
// xxxxyyAA - telemetry frames counter
// xxxxyAyy - value of packet_tx_meteo_counter limited to 15
// xxxxAyyy - value of packet_tx_meteo_gsm_counter limited to 15
// xxAAyyyy - value of packet_tx_beacon_counter
// Axxxyyyy - checksum

// 8th register map
// xxxxyyAA - resets caused by 'aprsis_check_connection_attempt_alive()'
// xxxxAAyy - resets caused by 'rte_wx_check_weather_measurements()'
// xxAAyyyy - resets caused by value of 'rte_wx_dallas_degraded_counter'
// AAxxyyyy - resets caused by 'system_is_rtc_ok()'

// 11th register map
// 		lsb
//			0 - event log report has been sent over APRS-IS in this hour
//			1 - event log report has been sent over radio in this hour
//		msb

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

	if (variant_validate_is_within_ram((void*)reg) != 0) {
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

	out = RTC->BKP3R;

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

	RTC->BKP3R = value;

	backup_reg_lock();

}

/**
 * Set certain bit in the register containing current configuration state.
 * This is used to store a state of all configuration sectors.
 * @param value
 */
void backup_reg_set_bits_configuration(uint32_t value) {
	// enable access to backup domain
	backup_reg_unclock();

	RTC->BKP3R |= value;

	backup_reg_lock();
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

	RTC->BKP3R &= (0xFFFFFFFFu ^ value);

	backup_reg_lock();

}

/**
 * Resets all powersave mode flags.
 */
void backup_reg_reset_all_powersave_states(void) {

	backup_reg_unclock();

	REGISTER &= 0xFFFFFFFFu ^ BACKUP_REG_ALL_PWRSAVE_STATES_BITMASK;

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

	if ((REGISTER & BACKUP_REG_ALL_PWRSAVE_STATES_BITMASK) == state) {
		out = 1;
	}
	return out;
}

/**
 * Set that controller is currently in given powersave state, it should be
 * used along with \link #backup_reg_reset_all_powersave_states
 * @param state
 */
void backup_reg_set_powersave_state(uint32_t state) {
	backup_reg_unclock();

	REGISTER |= state;

	backup_reg_lock();
}

/**
 * Returns current powersave state
 * @return
 */
uint16_t backup_reg_get_powersave_state(void) {
	int out = 0;

	out = (uint16_t)(REGISTER & BACKUP_REG_ALL_PWRSAVE_STATES_BITMASK);

	return out;
}

/**
 * Return counter value with current number of wakeup events
 * @return
 */
uint32_t backup_reg_get_wakeup_counter(void) {

	uint32_t out = 0;

	out = (uint32_t)((REGISTER_COUNTERS & 0xFFFF0000u) >> 16);

	return out;
}

/**
 * Set current value of wakeup events
 * @param in
 */
void backup_reg_set_wakeup_counter(uint32_t in) {
	backup_reg_unclock();

	REGISTER_COUNTERS = (REGISTER_COUNTERS & 0x0000FFFFu) | ((in & 0xFFFFu) << 16);

	backup_reg_lock();
}

/**
 * Return counter value with current number of sleep events
 * @return
 */
uint32_t backup_reg_get_sleep_counter(void) {
	uint32_t out = 0;

	out = (uint16_t)(REGISTER_COUNTERS & 0xFFFFu);

	return out;
}

/**
 * Set current value of sleep events
 * @param in
 */
void backup_reg_set_sleep_counter(uint32_t in) {
	backup_reg_unclock();

	REGISTER_COUNTERS = (REGISTER_COUNTERS & 0xFFFF0000u) | (uint16_t)(in & 0xFFFFu);

	backup_reg_lock();
}


/**
 * Returns a timestamp of last sleep event
 */
uint32_t backup_reg_get_last_sleep_timestamp(void) {

	uint32_t out = 0;

	out = REGISTER_LAST_SLEEP;

	return out;
}

/**
 * Stores a timestamp of a sleep event from current RTC time
 */
void backup_reg_set_last_sleep_timestamp(void) {

	backup_reg_unclock();

	REGISTER_LAST_SLEEP = RTC->TR;

	backup_reg_lock();

}

/**
 * Disables an inhibition of VBATT_S switching, when controller is in
 * powersave mode in which weather sensors shall be kept powered down.
 */
void backup_reg_reset_inhibit_periodic_pwr_switch(void) {
	backup_reg_unclock();

	REGISTER &= 0xFFFFFFFFu ^ BACKUP_REG_INHIBIT_PWR_SWITCH_PERIODIC_H;

	backup_reg_lock();
}

/**
 * Enables an inhibition of VBATT_S switching, when controller is in
 * powersave mode in which weather sensors shall be kept powered down.
 */
void backup_reg_inhibit_periodic_pwr_switch(void) {
	backup_reg_unclock();

	REGISTER |= BACKUP_REG_INHIBIT_PWR_SWITCH_PERIODIC_H;

	backup_reg_lock();

}

/**
 * Returns if VBATT_S switching inhibition is currently enabled
 * @return
 */
uint32_t backup_reg_is_periodic_pwr_switch_inhibited(void) {

	int out = 0;

	if ((REGISTER & BACKUP_REG_INHIBIT_PWR_SWITCH_PERIODIC_H) != 0) {
		out = 1u;
	}
	return out;
}

/**
 * gets last wakeup timestamp
 * @return
 */
uint32_t backup_reg_get_last_wakeup_timestamp(void) {

	uint32_t out = 0;
	out = REGISTER_LAST_WKUP;
	return out;
}

/**
 * Stores a timestamp of a wakeup event from current RTC time
 */
void backup_reg_set_last_wakeup_timestamp(void) {

	backup_reg_unclock();

	REGISTER_LAST_WKUP = RTC->TR;

	backup_reg_lock();
}

/**
 *
 * @return
 */
uint32_t backup_reg_get_last_sleep_duration(void) {
	uint32_t out = 0;

	out = REGISTER_LAST_SLTIM;

	return out;
}

/**
 *
 * @param in
 */
void backup_reg_set_last_sleep_duration(uint32_t in) {
	backup_reg_unclock();

	REGISTER_LAST_SLTIM = in;

	backup_reg_lock();
}

/**
 * Set register containing packet counters, used when a configuration
 * is reset to default
 */
void backup_reg_reset_counters(void) {
	backup_reg_unclock();

	REGISTER_PACKET_COUNTERS = 0u;

	backup_reg_lock();
}

/**
 * Gets last telemetry frames counter stored in backup registers
 * @return last telemetry counter or zero if backup registers contains crap
 */
uint8_t backup_reg_get_telemetry(void) {

	uint8_t out = 0u;

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

	backup_reg_lock();

}

/**
 *
 * @param beacon_counter
 * @param meteo_counter
 * @param meteo_gsm_counter
 */
void backup_reg_get_packet_counters(uint8_t * beacon_counter, uint8_t * meteo_counter, uint8_t * meteo_gsm_counter) {
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
}

/**
 *
 * @param beacon_counter
 * @param meteo_counter
 * @param meteo_gsm_counter
 */
void backup_reg_set_packet_counters(uint8_t beacon_counter, uint8_t meteo_counter, uint8_t meteo_gsm_counter) {
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
}

void backup_reg_increment_aprsis_check_reset(void) {
	// REGISTER_RESET_CHECK_FAIL
	volatile uint32_t reg_value = REGISTER_RESET_CHECK_FAIL;

	// get existing value
	uint8_t counter = (uint8_t)(reg_value & 0xFFU);

	// increment it
	counter++;

	// clear existing value from register
	reg_value &= 0xFFFFFF00U;

	// add incremented counter value
	reg_value |= counter;

	backup_reg_unclock();

	REGISTER_RESET_CHECK_FAIL = reg_value;

	backup_reg_lock();
}

void backup_reg_increment_weather_measurements_check_reset(void) {
	// REGISTER_RESET_CHECK_FAIL
	volatile uint32_t reg_value = REGISTER_RESET_CHECK_FAIL;

	// get existing value
	uint8_t counter = (uint8_t)((reg_value & 0xFF00U) >> 8U);

	// increment it
	counter++;

	// clear existing value from register
	reg_value &= 0xFFFF00FFU;

	// add incremented counter value
	reg_value |= ((uint32_t)counter << 8U);

	backup_reg_unclock();

	REGISTER_RESET_CHECK_FAIL = reg_value;

	backup_reg_lock();
}

void backup_reg_increment_dallas_degraded_reset(void) {
	// REGISTER_RESET_CHECK_FAIL
	volatile uint32_t reg_value = REGISTER_RESET_CHECK_FAIL;

	// get existing value
	uint8_t counter = (uint8_t)((reg_value & 0xFF0000U) >> 16U);

	// increment it
	counter++;

	// clear existing value from register
	reg_value &= 0xFF00FFFFU;

	// add incremented counter value
	reg_value |= ((uint32_t)counter << 16U);

	backup_reg_unclock();

	REGISTER_RESET_CHECK_FAIL = reg_value;

	backup_reg_lock();
}

void backup_reg_increment_is_rtc_ok_check_reset(void) {
	// REGISTER_RESET_CHECK_FAIL
	volatile uint32_t reg_value = REGISTER_RESET_CHECK_FAIL;

	// get existing value
	uint8_t counter = (uint8_t)((reg_value & 0xFF000000U) >> 24U);

	// increment it
	counter++;

	// clear existing value from register
	reg_value &= 0x00FFFFFFU;

	// add incremented counter value
	reg_value |= ((uint32_t)counter << 24U);

	backup_reg_unclock();

	REGISTER_RESET_CHECK_FAIL = reg_value;

	backup_reg_lock();
}

uint32_t backup_reg_get_register_reset_check_fail(void)
{
	return REGISTER_RESET_CHECK_FAIL;
}

void backup_assert(uint32_t assert) {
	backup_reg_unclock();

	REGISTER_ASSERT |= assert;

	backup_reg_lock();

	NVIC_SystemReset();
}

uint32_t backup_reg_get_last_restart_date(void) {
	return REGISTER_LAST_RESTART;
}

void backup_reg_set_last_restart_date(void) {
	backup_reg_unclock();

	REGISTER_LAST_RESTART = RTC->DR;

	backup_reg_lock();

}

void backup_reg_set_event_log_report_sent_aprsis(void) {
	backup_reg_unclock();

	REGISTER_PERSISTENT_STATUS |= BACKUP_REG_PERSISTENT_APRSIS_LOG_REPORT;

	backup_reg_lock();

}

void backup_reg_reset_event_log_report_sent_aprsis(void) {
	backup_reg_unclock();

	REGISTER_PERSISTENT_STATUS &= (0xFFFFFFFF ^ BACKUP_REG_PERSISTENT_APRSIS_LOG_REPORT);

	backup_reg_lock();
}

uint8_t backup_reg_get_event_log_report_sent_aprsis(void) {

	if ((REGISTER_PERSISTENT_STATUS & BACKUP_REG_PERSISTENT_APRSIS_LOG_REPORT) != 0) {
		return 1;
	}
	else {
		return 0;
	}
}


void backup_reg_set_event_log_report_sent_radio(void) {
	backup_reg_unclock();

	REGISTER_PERSISTENT_STATUS |= BACKUP_REG_PERSISTENT_RADIO_LOG_REPORT;

	backup_reg_lock();
}

void backup_reg_reset_event_log_report_sent_radio(void) {
	backup_reg_unclock();

	REGISTER_PERSISTENT_STATUS &= (0xFFFFFFFF ^ BACKUP_REG_PERSISTENT_RADIO_LOG_REPORT);

	backup_reg_lock();
}

uint8_t backup_reg_get_event_log_report_sent_radio(void) {
	if ((REGISTER_PERSISTENT_STATUS & BACKUP_REG_PERSISTENT_RADIO_LOG_REPORT) != 0) {
		return 1;
	}
	else {
		return 0;
	}
}

void backup_reg_set_inhibit_log_report_send_api(void) {
	backup_reg_unclock();

	REGISTER_PERSISTENT_STATUS |= BACKUP_REG_PERSISTENT_RADIO_LOG_REPORT;

	backup_reg_lock();
}

void backup_reg_reset_inhibit_log_report_send_api(void) {
	backup_reg_unclock();

	REGISTER_PERSISTENT_STATUS &= (0xFFFFFFFF ^ BACKUP_REG_PERSISTENT_INHIBIT_API_LOG_REPORT);

	backup_reg_lock();
}

uint8_t backup_reg_get_inhibit_log_report_send_api(void) {
	if ((REGISTER_PERSISTENT_STATUS & BACKUP_REG_PERSISTENT_INHIBIT_API_LOG_REPORT) != 0) {
		return 1;
	}
	else {
		return 0;
	}
}

