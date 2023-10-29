/*
 * pwr_save.c
 *
 *  Created on: Aug 22, 2021
 *      Author: mateusz
 */

#include "pwr_save.h"
#include "pwr_save_configuration.h"

#include "stm32l4xx.h"
#include "system_stm32l4xx.h"
#include <stdint.h>

#include "wx_pwr_switch.h"
#include "io.h"
#include "LedConfig.h"
#include "packet_tx_handler.h"
#include "wx_handler.h"
#include "main.h"
#include "backup_registers.h"
#include "status.h"
#include "afsk_pr.h"
#include "gsm/sim800c.h"
#include "aprsis.h"
#include "it_handlers.h"

#include "rte_main.h"

#include "drivers/analog_anemometer.h"

#define IN_STOP2_MODE (1 << 1)
#define IN_C0_STATE (1 << 2)
#define IN_C1_STATE (1 << 3)
#define IN_C2_STATE (1 << 4)
#define IN_C3_STATE (1 << 5)
#define IN_M4_STATE (1 << 6)
#define IN_I5_STATE (1 << 7)
#define IN_L6_STATE (1 << 8)
#define IN_L7_STATE (1 << 9)

#define MINIMUM_SENSEFUL_VBATT_VOLTAGE	678u

#if defined(STM32L471xx)

int8_t pwr_save_seconds_to_wx = 0;
int16_t pwr_save_sleep_time_in_seconds = -1;

/**
 * Number of 30 seconds cycles of SLEEP2 in L6 and L7 powersave mode
 */
int8_t pwr_save_number_of_sleep_cycles = -1;

/**
 * Variable stores cutoff state and to save RAM it also keeps a low battery voltage flag
 */
int8_t pwr_save_currently_cutoff = 0;

/**
 * This stores a previous value of 'pwr_save_currently_cutoff' which is required to
 * trigger a status message when controller goes into low battery voltage or cutoff state
 */
int8_t pwr_save_previously_cutoff = 0;

/**
 * This is cutoff voltage at which the power saving subsystem will keep ParaMETEO constantly
 * in L7 mode and wakeup once every 20 minutes to check B+ once again
 */
const uint16_t pwr_save_cutoff_voltage = 			PWR_SAVE_CUTOFF_VOLTAGE_DEF;

/**
 * This is the restore voltage a battery must be charged to for ParaMETEO to restore it's normal operation
 */
const uint16_t pwr_save_startup_restore_voltage =	PWR_SAVE_STARTUP_RESTORE_VOLTAGE_DEF;

/**
 * Below this voltage (and above pwr_save_cutoff_voltage) software will switch powersaving
 * mode to PWSAVE_AGGRESV
 */
const uint16_t pwr_save_aggressive_powersave_voltage = PWR_SAVE_AGGRESIVE_POWERSAVE_VOLTAGE;

static void pwr_save_unclock_rtc_backup_regs(void) {
	// enable access to backup domain
	PWR->CR1 |= PWR_CR1_DBP;
}

static void pwr_save_lock_rtc_backup_regs(void) {
	PWR->CR1 &= (0xFFFFFFFF ^ PWR_CR1_DBP);
}

static void pwr_save_clear_powersave_idication_bits() {
	// unlock access to backup registers
	pwr_save_unclock_rtc_backup_regs();

	// clear all previous powersave indication bits
	backup_reg_reset_all_powersave_states();

	// lock access to backup
	pwr_save_lock_rtc_backup_regs();
}

/**
 * Entering STOP2 power save mode. In this mode all clocks except LSI and LSE are disabled. StaticRAM content
 * is preserved, optionally GPIO and few other peripherals can be kept power up depending on configuration
 */
static void pwr_save_enter_stop2(void) {

	// set 31st monitor bit
	backup_reg_set_monitor(31);

	// reload internal watchdog
	main_reload_internal_wdg();

	// clear main battery voltage to be sure that it'd be updated???
	rte_main_battery_voltage = 0;

	analog_anemometer_deinit();

	// clear previous low power mode selection
	PWR->CR1 &= (0xFFFFFFFF ^ PWR_CR1_LPMS_Msk);

	// select STOP2
	PWR->CR1 |= PWR_CR1_LPMS_STOP2;

	// enable write access to RTC registers by writing two magic words
	RTC->WPR = 0xCA;
	RTC->WPR = 0x53;

	// unlock an access to backup domain
	pwr_save_unclock_rtc_backup_regs();

	// save an information that STOP2 mode has been applied
	RTC->BKP0R |= IN_STOP2_MODE;

	// save a timestamp when micro has been switched to STOP2 mode
	backup_reg_set_last_sleep_timestamp();

	pwr_save_lock_rtc_backup_regs();

	SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

	DBGMCU->CR &= (0xFFFFFFFF ^ (DBGMCU_CR_DBG_SLEEP_Msk | DBGMCU_CR_DBG_STOP_Msk | DBGMCU_CR_DBG_STANDBY_Msk));

	// disabling all IRQs
	//__disable_irq();

	asm("sev");
	asm("wfi");

}

/**
 * Used after each of 30 seconds long STOP2 sleep, to check
 * how many sleeps the micro must be put in, to complete
 * L6/L7 powersave mode
 */
static void pwr_save_check_stop2_cycles(void) {

	while(1) {
		// decrement stop2 cycles for current L7 or L6 powersave mode
		pwr_save_number_of_sleep_cycles--;

		// if there is time left to exit from depp sleep
		if (pwr_save_number_of_sleep_cycles > 0) {
			backup_reg_set_monitor(15);

			// go back to sleep
			// configure how long micro should sleep
			system_clock_configure_auto_wakeup_l4(PWR_SAVE_STOP2_CYCLE_LENGHT_SEC);

			pwr_save_enter_stop2();
		}
		else {
			backup_reg_set_monitor(14);

			// we are done sleeping so exit from this loop
			break;
		}
	}
}

/**
 * This function has to be called after last 30 second long cycle of STOP2 sleep,
 * to bounce all frames transmission counters.
 */
static void pwr_save_exit_after_last_stop2_cycle(void) {

	uint32_t counter = 0;

	// set 30th minitor bit
	backup_reg_set_monitor(30);

	// unlock access to backup registers
	pwr_save_unclock_rtc_backup_regs();

	// save a timestamp of this wakeup event
	//REGISTER_LAST_WKUP = RTC->TR;
	backup_reg_set_last_wakeup_timestamp();

	// increase wakeup counter
	counter = backup_reg_get_wakeup_counter();

	counter++;

	// store current wakeup counter in RTE
	rte_main_wakeup_count = counter;

	// check counter overflow conditions
	if (counter > 0xFFFF) {
		counter = 0;
	}

	backup_reg_set_wakeup_counter(counter);

	pwr_save_lock_rtc_backup_regs();

	// packet tx timers values
	packet_tx_counter_values_t timers;

	// check power saving mode set before switching uC to SLEEP2
	uint16_t powersave_mode = backup_reg_get_powersave_state();//(uint16_t)(REGISTER & ALL_STATES_BITMASK);

	// check if sleep time is valid
	if (pwr_save_sleep_time_in_seconds <= 0) {
		// if for some reason the value is not valid change is to something meaningful
		pwr_save_sleep_time_in_seconds = 60;
	}

	main_reset_pooling_timers();

	switch(powersave_mode) {
	case IN_L6_STATE:
	case IN_L7_STATE:

		// get all timers values
		packet_tx_get_current_counters(&timers);

		// rewind all timers in packet tx handler as they were no updated when micro was sleeping
		// sleep shall be always set as wx packet interval minus one minute
		timers.wx_counter += (pwr_save_sleep_time_in_seconds / 60);
		timers.beacon_counter += (pwr_save_sleep_time_in_seconds / 60);
		timers.kiss_counter += (pwr_save_sleep_time_in_seconds / 60);
		timers.telemetry_counter += (pwr_save_sleep_time_in_seconds / 60);
		timers.telemetry_desc_counter += (pwr_save_sleep_time_in_seconds / 60);

		if ((pwr_save_currently_cutoff & CURRENTLY_CUTOFF) == 0) {
			// set counters back
			packet_tx_set_current_counters(&timers);
		}
		else {
			packet_tx_set_current_counters(0);
		}

		break;

	// something is screwed horribly as in all other modes a micro shall not be placed in STOP2 mode
	default:
		break;
	}

	// reinitialize LEDs when controller goes out from sleep
	it_handlers_inhibit_radiomodem_dcd_led = 0;
	led_init();

	backup_reg_set_monitor(29);
}

/**
 * This function is called in two places within a pooler.
 * 1st: just after the micro wakes up from STOP2 deep sleep caused by low battery
 * voltage and returns from an interrupt
 * from RTC interrupt handler.
 * 2nd: just after the micro wakes up from STOP2 caues by aggressive powersave
 * configuration.
 */
static void pwr_save_after_stop2_rtc_wakeup_it(void) {
	// check if we are just after waking up from STOP2 mode
	if (rte_main_woken_up == RTE_MAIN_WOKEN_UP_RTC_INTERRUPT) {

		// if yes set curent state
		rte_main_woken_up = RTE_MAIN_WOKEN_UP_AFTER_RTC_IT;

		// check if this is an intermediate wakeup from STOP2
		pwr_save_check_stop2_cycles();

		system_clock_configure_l4();

		pwr_save_exit_after_last_stop2_cycle();

		rte_main_woken_up = RTE_MAIN_WOKEN_UP_EXITED;
	}

}

int pwr_save_switch_mode_to_c0(void) {

	if (backup_reg_is_in_powersave_state(IN_C0_STATE) != 0) {
		return 0;
	}
	//backup_reg_is_in_powersave_state

	// turn ON +5V_S
	io___cntrl_vbat_s_enable();

	io___cntrl_vbat_m_enable();

	// turn ON +5V_R and VBATT_SW_R
	io___cntrl_vbat_r_enable();

	// turn ON +4V_G
	io___cntrl_vbat_g_enable();

	// turn ON +5V_C (SD card, PT100 interface and Op Amplifier)
	io___cntrl_vbat_c_enable();

	// deinhibit GSM modem
	gsm_sim800_inhibit(0);

	// unlock access to backup registers
	pwr_save_unclock_rtc_backup_regs();

	// clear all previous powersave indication bits
	backup_reg_reset_all_powersave_states();

	// set for C0 mode
	backup_reg_set_powersave_state(IN_C0_STATE);

	// lock access to backup
	pwr_save_lock_rtc_backup_regs();

	return 1;

}

// in HW-RevB this will disable external VHF radio!!
int pwr_save_switch_mode_to_c1(void) {

	if (backup_reg_is_in_powersave_state(IN_C0_STATE) != 0) {
		return 0;
	}

	// disconnect APRS-IS connection if it is established
	aprsis_disconnect();

	// turn ON +5V_S (and internal VHF radio module in HW-RevB)
	io___cntrl_vbat_s_enable();

	io___cntrl_vbat_m_enable();

	// turn ON +5V_R and VBATT_SW_R
	io___cntrl_vbat_r_enable();

	// turn OFF +4V_G
	io___cntrl_vbat_g_disable();

	// turn ON +5V_C (SD card, PT100 interface and Op Amplifier)
	io___cntrl_vbat_c_enable();

	// inhibit GSM modem
	gsm_sim800_inhibit(1);

	// unlock access to backup registers
	pwr_save_unclock_rtc_backup_regs();

	// clear all previous powersave indication bits
	backup_reg_reset_all_powersave_states();

	// set for C0 mode
	backup_reg_set_powersave_state(IN_C1_STATE);

	// lock access to backup
	pwr_save_lock_rtc_backup_regs();

	return 1;
}

// this mode is not avaliable in HW Revision B as internal radio
// is powered from +5V_S and external one is switched on with the same
// line which controls +4V_G
void pwr_save_switch_mode_to_c2(void) {

	if (backup_reg_is_in_powersave_state(IN_C0_STATE) != 0) {
		return;
	}

	// disconnect APRS-IS connection if it is established
	aprsis_disconnect();

	// turn OFF +5V_S (and internal VHF radio module in HW-RevB)
	io___cntrl_vbat_s_disable();

	io___cntrl_vbat_m_disable();

	// turn ON +5V_R and VBATT_SW_R
	io___cntrl_vbat_r_enable();

	// turn OFF +4V_G
	io___cntrl_vbat_g_disable();

	// turn ON +5V_C (SD card, PT100 interface and Op Amplifier)
	io___cntrl_vbat_c_enable();

	// inhibit GSM modem
	gsm_sim800_inhibit(1);

	// unlock access to backup registers
	pwr_save_unclock_rtc_backup_regs();

	// clear all previous powersave indication bits
	backup_reg_reset_all_powersave_states();

	// set for C2 mode
	backup_reg_set_powersave_state(IN_C2_STATE);

	// lock access to backup
	pwr_save_lock_rtc_backup_regs();

}

void pwr_save_switch_mode_to_c3(void) {

	if (backup_reg_is_in_powersave_state(IN_C3_STATE) != 0) {
		return;
	}

	// turn OFF +5V_S (and internal VHF radio module in HW-RevB)
	io___cntrl_vbat_s_disable();

	io___cntrl_vbat_m_disable();

	// turn ON +5V_R and VBATT_SW_R
	io___cntrl_vbat_r_enable();

	// turn ON +4V_G
	io___cntrl_vbat_g_enable();

	// turn ON +5V_C (SD card, PT100 interface and Op Amplifier)
	io___cntrl_vbat_c_enable();

	// deinhibit GSM modem
	gsm_sim800_inhibit(0);

	// unlock access to backup registers
	pwr_save_unclock_rtc_backup_regs();

	// clear all previous powersave indication bits
	backup_reg_reset_all_powersave_states();

	// set for C3 mode
	backup_reg_set_powersave_state(IN_C3_STATE);

	// lock access to backup
	pwr_save_lock_rtc_backup_regs();

}

// in HW-RevB this will keep internal VHF radio module working!
int pwr_save_switch_mode_to_m4(void) {

	if (backup_reg_is_in_powersave_state(IN_M4_STATE) != 0) {
		return 0;
	}

	// disconnect APRS-IS connection if it is established
	aprsis_disconnect();

	// turn ON +5V_S (and internal VHF radio module in HW-RevB)
	io___cntrl_vbat_s_enable();

	io___cntrl_vbat_m_enable();

	// turn OFF +5V_R and VBATT_SW_R
	io___cntrl_vbat_r_disable();

	// turn OFF +4V_G
	io___cntrl_vbat_g_disable();

	// turn ON +5V_C (SD card, PT100 interface and Op Amplifier)
	io___cntrl_vbat_c_enable();

	// inhibit GSM modem
	gsm_sim800_inhibit(1);

	// unlock access to backup registers
	pwr_save_unclock_rtc_backup_regs();

	// clear all previous powersave indication bits
	backup_reg_reset_all_powersave_states();

	// set for C3 mode
	backup_reg_set_powersave_state(IN_M4_STATE);

	// lock access to backup
	pwr_save_lock_rtc_backup_regs();

	return 1;
}

int pwr_save_switch_mode_to_m4a(void) {

	if (backup_reg_is_in_powersave_state(IN_M4_STATE) != 0) {
		return 0;
	}

	// turn ON +5V_S (and internal VHF radio module in HW-RevB)
	io___cntrl_vbat_s_enable();

	io___cntrl_vbat_m_enable();

	// turn OFF +5V_R and VBATT_SW_R
	io___cntrl_vbat_r_disable();

	// turn OFF +4V_G
	io___cntrl_vbat_g_enable();

	// turn ON +5V_C (SD card, PT100 interface and Op Amplifier)
	io___cntrl_vbat_c_enable();

	// deinhibit GSM modem
	gsm_sim800_inhibit(0);

	// unlock access to backup registers
	pwr_save_unclock_rtc_backup_regs();

	// clear all previous powersave indication bits
	backup_reg_reset_all_powersave_states();

	// set for C3 mode
	backup_reg_set_powersave_state(IN_M4_STATE);

	// lock access to backup
	pwr_save_lock_rtc_backup_regs();

	return 1;
}

void pwr_save_switch_mode_to_i5(void) {

	if (backup_reg_is_in_powersave_state(IN_I5_STATE) != 0) {
		return;
	}

	// disconnect APRS-IS connection if it is established
	aprsis_disconnect();

	// turn OFF +5V_S (and internal VHF radio module in HW-RevB)
	io___cntrl_vbat_s_disable();

	io___cntrl_vbat_m_disable();

	// turn OFF +5V_R and VBATT_SW_R
	io___cntrl_vbat_r_disable();

	// turn OFF +4V_G
	io___cntrl_vbat_g_disable();

	// turn OFF +5V_C (SD card, PT100 interface and Op Amplifier)
	io___cntrl_vbat_c_disable();

	// inhibit GSM modem
	gsm_sim800_inhibit(1);

	// unlock access to backup registers
	pwr_save_unclock_rtc_backup_regs();

	// clear all previous powersave indication bits
	backup_reg_reset_all_powersave_states();

	// set for C3 mode
	backup_reg_set_powersave_state(IN_I5_STATE);

	// lock access to backup
	pwr_save_lock_rtc_backup_regs();

}

// this will keep external VHF radio working in HW-RevB
void pwr_save_switch_mode_to_l6(uint16_t sleep_time) {

	uint16_t counter = 0;

	if (sleep_time > 3000u) {
		// this is an error situation
		sleep_time = 3000u;
	}

	if (system_is_rtc_ok() == 0) {
		pwr_save_switch_mode_to_i5();

		return;
	}

	if (backup_reg_is_in_powersave_state(IN_L6_STATE) != 0) {
		return;
	}

	// calculate amount of STOP2 cycles
	pwr_save_number_of_sleep_cycles = (int8_t)(sleep_time / PWR_SAVE_STOP2_CYCLE_LENGHT_SEC) & 0x7Fu;

	backup_reg_set_monitor(28);

	// disable ADC used for vbat measurement
	io_vbat_meas_disable();

	// stop DAC and ADC used for APRS
	ADCStop();
	DACStop();

	// turn OFF +5V_S (and internal VHF radio module in HW-RevB)
	io___cntrl_vbat_s_disable();

	io___cntrl_vbat_m_disable();

	// turn OFF +5V_R and VBATT_SW_R
	io___cntrl_vbat_r_disable();

	// turn ON +4V_G
	io___cntrl_vbat_g_enable();

	// turn OFF +5V_C (SD card, PT100 interface and Op Amplifier)
	io___cntrl_vbat_c_disable();

	// de inhibit GSM modem
	gsm_sim800_inhibit(0);

	// unlock access to backup registers
	pwr_save_unclock_rtc_backup_regs();

	// clear all previous powersave indication bits
	backup_reg_reset_all_powersave_states();

	// set for C3 mode
	backup_reg_set_powersave_state(IN_L6_STATE);

	backup_reg_set_last_sleep_timestamp();

	// increment the STOP2 sleep counters
	counter = backup_reg_get_sleep_counter();//(uint16_t)(REGISTER_COUNTERS & 0xFFFF);

	counter++;

	rte_main_going_sleep_count = counter;

	backup_reg_set_sleep_counter(counter);

	// lock access to backup
	pwr_save_lock_rtc_backup_regs();

	system_clock_configure_auto_wakeup_l4(PWR_SAVE_STOP2_CYCLE_LENGHT_SEC);

	// save how long the micro will sleep - required for handling wakeup event
	pwr_save_sleep_time_in_seconds = sleep_time;

	// turn off leds to save power
	it_handlers_inhibit_radiomodem_dcd_led = 1;
	led_control_led1_upper(false);
	led_control_led2_bottom(false);
	led_deinit();

	pwr_save_enter_stop2();

	backup_reg_set_monitor(27);


}

void pwr_save_switch_mode_to_l7(uint16_t sleep_time) {

	uint16_t counter = 0;

	if (sleep_time > 3000u) {
		// this is an error situation
		sleep_time = 3000u;
	}

	///////////
	if (system_is_rtc_ok() == 0) {
		pwr_save_switch_mode_to_i5();

		return;
	}

	if (backup_reg_is_in_powersave_state(IN_L7_STATE) != 0) {
		return;
	}

	// calculate amount of STOP2 cycles
	pwr_save_number_of_sleep_cycles = (int8_t)(sleep_time / PWR_SAVE_STOP2_CYCLE_LENGHT_SEC) & 0x7Fu;

	backup_reg_set_monitor(26);

	// disconnect APRS-IS connection if it is established
	aprsis_disconnect();

	// disable ADC used for vbat measurement
	io_vbat_meas_disable();

	// stop DAC and ADC used for APRS
	ADCStop();
	DACStop();

	// turn OFF +5V_S (and internal VHF radio module in HW-RevB)
	io___cntrl_vbat_s_disable();

	io___cntrl_vbat_m_disable();

	// turn OFF +5V_R and VBATT_SW_R
	io___cntrl_vbat_r_disable();

	// turn OFF +4V_G
	io___cntrl_vbat_g_disable();

	// turn OFF +5V_C (SD card, PT100 interface and Op Amplifier)
	io___cntrl_vbat_c_disable();

	// inhibit GSM modem
	gsm_sim800_inhibit(1);

	// clear all previous powersave indication bits
	backup_reg_reset_all_powersave_states();

	// set for C3 mode
	backup_reg_set_powersave_state(IN_L7_STATE);

	//REGISTER_LAST_SLTIM = sleep_time;
	backup_reg_set_last_sleep_timestamp();

	// increment the STOP2 sleep counters
	counter = backup_reg_get_sleep_counter();//(uint16_t)(REGISTER_COUNTERS & 0xFFFF);

	counter++;

	rte_main_going_sleep_count = counter;

	backup_reg_set_sleep_counter(counter);

	// configure how long micro should sleep
	system_clock_configure_auto_wakeup_l4(PWR_SAVE_STOP2_CYCLE_LENGHT_SEC);

	// save how long the micro will sleep - required for handling wakeup event
	pwr_save_sleep_time_in_seconds = sleep_time;

	// turn off leds to save power
	it_handlers_inhibit_radiomodem_dcd_led = 1;
	led_control_led1_upper(false);
	led_control_led2_bottom(false);
	led_deinit();

	pwr_save_enter_stop2();

	backup_reg_set_monitor(25);
}

/**
 * This function initializes everything related to power saving features
 * including programming Flash memory option bytes
 */
void pwr_save_init(config_data_powersave_mode_t mode) {

	// make a pointer to option byte
	uint32_t* option_byte = (uint32_t*)0x1FFF7800;

	// content of option byte read from the flash memory
	uint32_t option_byte_content = *option_byte;

	// definition of bitmask
	#define IWDG_STBY_STOP (0x3 << 17)

	// check if IWDG_STDBY and IWDG_STOP is set in ''User and read protection option bytes''
	// at 0x1FFF7800
	if ((option_byte_content & IWDG_STBY_STOP) != IWDG_STBY_STOP) {

		// unlock write/erase operations on flash memory
		FLASH->KEYR = 0x45670123;
		FLASH->KEYR = 0xCDEF89AB;

		// wait for any possible flash operation to finish (rather impossible here, but ST manual recommend doing this)
		while((FLASH->SR & FLASH_SR_BSY) != 0);

		// unlock operations on option bytes
		FLASH->OPTKEYR = 0x08192A3B;
		FLASH->OPTKEYR = 0x4C5D6E7F;

		// set the flash option register (in RAM!!)
		FLASH->OPTR |= FLASH_OPTR_IWDG_STDBY;
		FLASH->OPTR |= FLASH_OPTR_IWDG_STOP;

		// trigger an update of flash option bytes with values from RAM (from FLASH->OPTR)
		FLASH->CR |= FLASH_CR_OPTSTRT;

		// wait for option bytes to be updated
		while((FLASH->SR & FLASH_SR_BSY) != 0);

		// lock flash memory
		FLASH-> CR |= FLASH_CR_LOCK;

		// forcre reloading option bytes
		FLASH->CR |= FLASH_CR_OBL_LAUNCH;

	}

	//pwr_save_unclock_rtc_backup_regs();

	// reset a status register
	backup_reg_reset_all_powersave_states();
	backup_reg_reset_inhibit_periodic_pwr_switch();

	// switch power switch handler inhibition if it is needed
	switch (mode) {
		case PWSAVE_NONE:
			break;
		case PWSAVE_NORMAL:
		case PWSAVE_AGGRESV:
			backup_reg_inhibit_periodic_pwr_switch();
			break;
	}


}

config_data_powersave_mode_t pwr_save_pooling_handler(	const config_data_mode_t * config,
														const config_data_basic_t * timers,
														int16_t minutes_to_wx,
														uint16_t vbatt_average,
														uint16_t vbatt_current) {
	// this function should be called from 10 seconds pooler

	int reinit_sensors = 0;

	packet_tx_counter_values_t counters;

	// by default use powersave mode from controller configuration
	config_data_powersave_mode_t psave_mode = config->powersave;

	backup_reg_set_monitor(24);

	// save previous state
	pwr_save_previously_cutoff = pwr_save_currently_cutoff;

	// check if battery voltage measurement is done and senseful
	if (vbatt_average < MINIMUM_SENSEFUL_VBATT_VOLTAGE) {
		// inhibit both cutoff and aggresive powersave if vbatt measurement is either not
		// done at all or scaling factor are really screwed
		vbatt_average = 0xFFFFu;
	}

	#ifdef INHIBIT_CUTOFF
	vbatt_average = 0xFFFFu;		// TODO:: THis shall not be uncommented on production!!!
	#endif

	if (vbatt_average > PWR_SAVE_STARTUP_RESTORE_VOLTAGE_DEF) {
		pwr_save_currently_cutoff = 0;

		backup_reg_set_monitor(23);
	}
	else {
		if (vbatt_current <= PWR_SAVE_CUTOFF_VOLTAGE_DEF && vbatt_average <= PWR_SAVE_AGGRESIVE_POWERSAVE_VOLTAGE) {
			backup_reg_set_monitor(22);

			// if the battery voltage is below cutoff level and the ParaMETEO controller is currently not cut off
			pwr_save_currently_cutoff |= CURRENTLY_CUTOFF;
		}
		// check if battery voltage is below low voltage level
		else if (vbatt_average <= PWR_SAVE_AGGRESIVE_POWERSAVE_VOLTAGE) {
			backup_reg_set_monitor(21);

			// if battery voltage is low swtich to aggressive powersave mode
			pwr_save_currently_cutoff |= CURRENTLY_VBATT_LOW;

		}

	}

	backup_reg_set_monitor(20);

	// check if cutoff status has changed
	if (pwr_save_currently_cutoff != pwr_save_previously_cutoff) {
		status_send_powersave_cutoff(vbatt_average, pwr_save_previously_cutoff, pwr_save_currently_cutoff);
	}


	if ((pwr_save_currently_cutoff & CURRENTLY_CUTOFF) != 0) {
		backup_reg_set_monitor(19);

		// clear all previous powersave indication bits as we want to go sleep being already in L7 state
		pwr_save_clear_powersave_idication_bits();

		// go sleep immediately and periodically check if battery has been charged above restore level
		pwr_save_switch_mode_to_l7(60 * PWR_SAVE_CUTOFF_SLEEP_TIME_IN_MINUTES);

		// RTC interrupt is between this call and previous one (switching to l7)
		pwr_save_after_stop2_rtc_wakeup_it();

		return psave_mode;
	}

	if ((pwr_save_currently_cutoff & CURRENTLY_VBATT_LOW) != 0) {
		backup_reg_set_monitor(18);

		psave_mode = PWSAVE_AGGRESV;
	}


	// get current counter values
	packet_tx_get_current_counters(&counters);

	// decrement seconds in last minute
	if (pwr_save_seconds_to_wx != -1) {
		pwr_save_seconds_to_wx -= 10;
	}

	// if there is more than one minute to next frame
	if (minutes_to_wx > 1) {
		// reset counter as we dont
		pwr_save_seconds_to_wx = -1;
	}
	else if (minutes_to_wx == 1 && pwr_save_seconds_to_wx == -1) {
		// if this is the last second to wx frame
		pwr_save_seconds_to_wx = 60;
	}

	// handle depends on current powersave configuration
	switch (psave_mode) {
		/**
		 * 	PWSAVE_NONE = 0,
			PWSAVE_NORMAL = 1,
			PWSAVE_AGGRESV = 3
		 */
		case PWSAVE_NONE : {

			// if weather station is enabled
			if (config->wx == 1) {

				// if GSM modem is enabled in configuration
				if (config->gsm == 1) {

					// if digipeater is enabled
					if (config->digi == 1) {		// DIGI + WX + GSM
						reinit_sensors = pwr_save_switch_mode_to_c0();
					}
					else {		// WX + GSM
						reinit_sensors = pwr_save_switch_mode_to_c0();
					}
				}
				else {
					// if digipeater is enabled
					if (config->digi == 1) {		// DIGI + WX
						reinit_sensors = pwr_save_switch_mode_to_c1();
					}
					else {		// WX
						if (minutes_to_wx > 2) {
							if (config->powersave_keep_gsm_always_enabled == 0){
								reinit_sensors = pwr_save_switch_mode_to_m4();
							}
							else {
								reinit_sensors = pwr_save_switch_mode_to_m4a();
							}
						}
						else {
							reinit_sensors = pwr_save_switch_mode_to_c0();
						}


					}
				}
			}
			else {		// DIGI
				// if weather station is not enabled just stay in C2 mode
				// as this is default state for DIGI operation. Of course
				// DIGI might not be enabled (which has no sense) but for
				// sake of simplicity just agree that it is.
				pwr_save_switch_mode_to_c2();
			}

			break;
		}

		case PWSAVE_NORMAL : {

			// if weather station is enabled
			if (config->wx == 1) {

				// if GSM modem is enabled in configuration
				if (config->gsm == 1) {

					// if digipeater is enabled
					if (config->digi == 1) {		// DIGI + WX + GSM
						// if weather packets are send 5 minutes or less often
						if (timers->wx_transmit_period >= 5) {
							if (minutes_to_wx > 1) {
								pwr_save_switch_mode_to_c2();
							}
							else {
								reinit_sensors = pwr_save_switch_mode_to_c0();
							}
						}
						else {
							if (minutes_to_wx > 1) {
								pwr_save_switch_mode_to_c3();
							}
							else {
								reinit_sensors = pwr_save_switch_mode_to_c0();
							}
						}
					}
					else {		// WX + GSM
						if (minutes_to_wx > 1) {
							if (config->powersave_keep_gsm_always_enabled == 0){
								reinit_sensors = pwr_save_switch_mode_to_m4();
							}
							else {
								reinit_sensors = pwr_save_switch_mode_to_m4a();
							}
						}
						else {
							reinit_sensors = pwr_save_switch_mode_to_c0();
						}
					}
				}
				else {
					// if digipeater is enabled
					if (config->digi == 1) {		// DIGI + WX
						if (minutes_to_wx > 1) {
							pwr_save_switch_mode_to_c2();
						}
						else {
							reinit_sensors = pwr_save_switch_mode_to_c1();
						}
					}
					else {		// WX
						if (minutes_to_wx > 2) {
							backup_reg_set_monitor(17);

							// if there is more than two minutes to send wx packet
							pwr_save_switch_mode_to_l7((timers->wx_transmit_period * 60) - 120);
						}
						else {
							// TODO: Workaround here for HW-RevB!!!
							//reinit_sensors= pwr_save_switch_mode_to_c1();
							reinit_sensors = pwr_save_switch_mode_to_c0();
						}
					}
				}
			}
			else {		// DIGI
				pwr_save_switch_mode_to_c2();
			}


			break;
		}

		case PWSAVE_AGGRESV : {

			// if weather station is enabled
			if (config->wx == 1) {

				// if GSM modem is enabled in configuration
				if (config->gsm == 1) {

					// if digipeater is enabled
					if (config->digi == 1) {		// DIGI + WX + GSM
						if (minutes_to_wx > 1) {
							pwr_save_switch_mode_to_c2();
						}
						else {
							reinit_sensors = pwr_save_switch_mode_to_c0();
						}

					}
					else {		// WX + GSM (only)
						if (timers->wx_transmit_period >= 5) {
							// if stations is configured to send wx packet less frequent than every 5 minutes

							if (minutes_to_wx > 1) {
								backup_reg_set_monitor(17);

								// if there is more than one minute to wx packet
								pwr_save_switch_mode_to_l7((timers->wx_transmit_period * 60) - 60);				// TODO: !!!
							}
							else {
								if (pwr_save_seconds_to_wx <= 50) {
									// if there is 30 seconds or less to next wx packet
									reinit_sensors = pwr_save_switch_mode_to_c0();
								}
								else {
									// if there is 30 to 60 seconds to next wx packet
									if (config->powersave_keep_gsm_always_enabled == 0){
										reinit_sensors = pwr_save_switch_mode_to_m4();
									}
									else {
										reinit_sensors = pwr_save_switch_mode_to_m4a();
									}
								}
							}
						}
						else {
							// if station is configured to sent wx packet in every 5 minutes or more often

							if (minutes_to_wx > 1) {
								backup_reg_set_monitor(17);

								pwr_save_switch_mode_to_l6((timers->wx_transmit_period * 60) - 60);				// TODO: !!!
							}
							else {
								reinit_sensors = pwr_save_switch_mode_to_c0();
							}
						}
					}
				}
				else {	// gsm is not enabled
					// if digipeater is enabled
					if (config->digi == 1) {		// DIGI + WX
						if (minutes_to_wx > 1) {
							pwr_save_switch_mode_to_c2();
						}
						else {
							reinit_sensors = pwr_save_switch_mode_to_c1();
						}
					}
					else {		// WX
						if (minutes_to_wx > 1) {
							backup_reg_set_monitor(17);

							// if there is more than one minute to send wx packet
							pwr_save_switch_mode_to_l7((timers->wx_transmit_period * 60) - 60);
						}
						else {
							if (pwr_save_seconds_to_wx <= 30) {
								// TODO: Workaround here for HW-RevB!!!
								reinit_sensors= pwr_save_switch_mode_to_c1();
								//pwr_save_switch_mode_to_c0();

								// do not reinitialize everything as reinitialization had been done when switching to m4 mode
								reinit_sensors = 0;
							}
							else {
								if (config->powersave_keep_gsm_always_enabled == 0){
									reinit_sensors = pwr_save_switch_mode_to_m4();
								}
								else {
									reinit_sensors = pwr_save_switch_mode_to_m4a();
								}
							}
						}
					}
				}
			}
			else {		// DIGI
				pwr_save_switch_mode_to_c2();
			}

			break;
		}
	}

	backup_reg_set_monitor(16);

	pwr_save_after_stop2_rtc_wakeup_it();

	backup_reg_set_monitor(13);

	if (reinit_sensors != 0) {
		// reinitialize all i2c sensors
		wx_force_i2c_sensor_reset = 1;

		// reset GSM modem, internally this also check if GSM modem is inhibited or not
		rte_main_reset_gsm_modem = 1;

		// reinitialize everything realted to anemometer
		analog_anemometer_init(main_config_data_mode->wx_anemometer_pulses_constant, 38, 100, 1);
	}

	return psave_mode;
}

int pwr_save_is_currently_cutoff(void) {
	int out = 0;

	if ((pwr_save_currently_cutoff & CURRENTLY_CUTOFF) != 0) {
		out = 1;
	}

	return out;
}

uint8_t pwr_save_get_inhibit_pwr_switch_periodic(void) {

	if (backup_reg_is_periodic_pwr_switch_inhibited() != 0){
		return 1;
	}
	else if ((pwr_save_currently_cutoff & CURRENTLY_CUTOFF) != 0) {
		return 1;
	}
	else {
		return 0;
	}
}

#else

uint8_t pwr_save_get_inhibit_pwr_switch_periodic(void) {
	return 0;
}


#endif

