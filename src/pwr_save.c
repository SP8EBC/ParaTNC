/*
 * pwr_save.c
 *
 *  Created on: Aug 22, 2021
 *      Author: mateusz
 */

#include "pwr_save.h"

#include "stm32l4xx.h"
#include "system_stm32l4xx.h"
#include <stdint.h>

#include "pwr_switch.h"
#include "io.h"
#include "LedConfig.h"
#include "packet_tx_handler.h"
#include "wx_handler.h"
#include "main.h"

#include "drivers/analog_anemometer.h"


#define REGISTER RTC->BKP0R

#define INHIBIT_PWR_SWITCH_PERIODIC_H 1
#define IN_STOP2_MODE (1 << 1)
#define IN_C0_STATE (1 << 2)
#define IN_C1_STATE (1 << 3)
#define IN_C2_STATE (1 << 4)
#define IN_C3_STATE (1 << 5)
#define IN_M4_STATE (1 << 6)
#define IN_I5_STATE (1 << 7)
#define IN_L6_STATE (1 << 8)
#define IN_L7_STATE (1 << 9)

#define ALL_STATES_BITMASK (0xFF << 2)

#if defined(STM32L471xx)

int8_t pwr_save_seconds_to_wx = 0;
int16_t pwr_save_sleep_time_in_seconds = -1;

static void pwr_save_unclock_rtc_backup_regs(void) {
	// enable access to backup domain
	PWR->CR1 |= PWR_CR1_DBP;
}

static void pwr_save_lock_rtc_backup_regs(void) {
	PWR->CR1 &= (0xFFFFFFFF ^ PWR_CR1_DBP);
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

	// check if IWDG_STDBY and IWDG_STOP is not set in ''User and read protection option bytes''
	// at 0x1FFF7800
	if ((option_byte_content & IWDG_STBY_STOP) == IWDG_STBY_STOP) {

		// unlock write/erase operations on flash memory
		FLASH->KEYR = 0x45670123;
		FLASH->KEYR = 0xCDEF89AB;

		// wait for any possible flash operation to finish (rather impossible here, but ST manual recommend doing this)
		while((FLASH->SR & FLASH_SR_BSY) != 0);

		// unlock operations on option bytes
		FLASH->OPTKEYR = 0x08192A3B;
		FLASH->OPTKEYR = 0x4C5D6E7F;

		// set the flash option register (in RAM!!)
		FLASH->OPTR &= (0xFFFFFFFF ^ (FLASH_OPTR_IWDG_STDBY | FLASH_OPTR_IWDG_STOP));

		// trigger an update of flash option bytes with values from RAM (from FLASH->OPTR)
		FLASH->CR |= FLASH_CR_OPTSTRT;

		// wait for option bytes to be updated
		while((FLASH->SR & FLASH_SR_BSY) != 0);

		// lock flash memory
		FLASH-> CR |= FLASH_CR_LOCK;

		// forcre reloading option bytes
		FLASH->CR |= FLASH_CR_OBL_LAUNCH;

	}

	pwr_save_unclock_rtc_backup_regs();

	// reset a status register
	REGISTER = 0;

	// switch power switch handler inhibition if it is needed
	switch (mode) {
		case PWSAVE_NONE:
			break;
		case PWSAVE_NORMAL:
		case PWSAVE_AGGRESV:
			REGISTER |= INHIBIT_PWR_SWITCH_PERIODIC_H;
			break;
	}

	pwr_save_lock_rtc_backup_regs();

}

/**
 * Entering STOP2 power save mode. In this mode all clocks except LSI and LSE are disabled. StaticRAM content
 * is preserved, optionally GPIO and few other peripherals can be kept power up depending on configuration
 */
void pwr_save_enter_stop2(void) {

	analog_anemometer_deinit();

	// clear previous low power mode selection
	PWR->CR1 &= (0xFFFFFFFF ^ PWR_CR1_LPMS_Msk);

	// select STOP2
	PWR->CR1 |= PWR_CR1_LPMS_STOP2;

	// enable write access to RTC registers by writing two magic words
	RTC->WPR = 0xCA;
	RTC->WPR = 0x53;

	pwr_save_unclock_rtc_backup_regs();

	// save an information that STOP2 mode has been applied
	RTC->BKP0R |= IN_STOP2_MODE;

	pwr_save_lock_rtc_backup_regs();

	SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

	DBGMCU->CR &= (0xFFFFFFFF ^ (DBGMCU_CR_DBG_SLEEP_Msk | DBGMCU_CR_DBG_STOP_Msk | DBGMCU_CR_DBG_STANDBY_Msk));

	// disabling all IRQs
	//__disable_irq();

	asm("sev");
	asm("wfi");

}

/**
 * This function has to be called within RTC wakepup interrupt.
 */
void pwr_save_exit_from_stop2(void) {

	// packet tx timers values
	packet_tx_counter_values_t timers;

	// check power saving mode set before switching uC to SLEEP2
	uint16_t powersave_mode = (uint16_t)(REGISTER & ALL_STATES_BITMASK);

	// check if sleep time is valid
	if (pwr_save_sleep_time_in_seconds <= 0) {
		// if for some reason the value is not valid change is to something meaningful
		pwr_save_sleep_time_in_seconds = 60;
	}


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

		// set counters back
		packet_tx_set_current_counters(&timers);

		break;

	// something is screwed horribly as in all other modes a micro shall not be placed in STOP2 mode
	default:
		break;
	}
}

int pwr_save_switch_mode_to_c0(void) {

	if ((REGISTER & ALL_STATES_BITMASK) == IN_C0_STATE) {
		return 0;
	}

	// turn ON +5V_S (and internal VHF radio module in HW-RevB)
	io_5v_isol_sw___cntrl_vbat_s_enable();

	// turn ON +5V_R and VBATT_SW_R
	io___cntrl_vbat_r_enable();

	// turn ON +4V_G
	io_12v_sw___cntrl_vbat_g_enable();

	// unlock access to backup registers
	pwr_save_unclock_rtc_backup_regs();

	// clear all previous powersave indication bits
	REGISTER &= 0xFFFFFFFF ^ ALL_STATES_BITMASK;

	// set for C0 mode
	REGISTER |= IN_C0_STATE;

	// lock access to backup
	pwr_save_lock_rtc_backup_regs();

	return 1;

}

// in HW-RevB this will disable external VHF radio!!
int pwr_save_switch_mode_to_c1(void) {

	if ((REGISTER & ALL_STATES_BITMASK) == IN_C1_STATE) {
		return 0;
	}

	// turn ON +5V_S (and internal VHF radio module in HW-RevB)
	io_5v_isol_sw___cntrl_vbat_s_enable();

	// turn ON +5V_R and VBATT_SW_R
	io___cntrl_vbat_r_enable();

	// turn OFF +4V_G
	io_12v_sw___cntrl_vbat_g_disable();

	// unlock access to backup registers
	pwr_save_unclock_rtc_backup_regs();

	// clear all previous powersave indication bits
	REGISTER &= (0xFFFFFFFF ^ ALL_STATES_BITMASK);

	// set for C0 mode
	REGISTER |= IN_C1_STATE;

	// lock access to backup
	pwr_save_lock_rtc_backup_regs();

	return 1;
}

// this mode is not avaliable in HW Revision B as internal radio
// is powered from +5V_S and external one is switched on with the same
// line which controls +4V_G
void pwr_save_switch_mode_to_c2(void) {

	if ((REGISTER & ALL_STATES_BITMASK) == IN_C2_STATE) {
		return;
	}

	// turn OFF +5V_S (and internal VHF radio module in HW-RevB)
	io_5v_isol_sw___cntrl_vbat_s_disable();

	// turn ON +5V_R and VBATT_SW_R
	io___cntrl_vbat_r_enable();

	// turn OFF +4V_G
	io_12v_sw___cntrl_vbat_g_disable();

	// unlock access to backup registers
	pwr_save_unclock_rtc_backup_regs();

	// clear all previous powersave indication bits
	REGISTER &= (0xFFFFFFFF ^ ALL_STATES_BITMASK);

	// set for C2 mode
	REGISTER |= IN_C2_STATE;

	// lock access to backup
	pwr_save_lock_rtc_backup_regs();

}

void pwr_save_switch_mode_to_c3(void) {

	if ((REGISTER & ALL_STATES_BITMASK) == IN_C3_STATE) {
		return;
	}

	// turn OFF +5V_S (and internal VHF radio module in HW-RevB)
	io_5v_isol_sw___cntrl_vbat_s_disable();

	// turn ON +5V_R and VBATT_SW_R
	io___cntrl_vbat_r_enable();

	// turn ON +4V_G
	io_12v_sw___cntrl_vbat_g_enable();

	// unlock access to backup registers
	pwr_save_unclock_rtc_backup_regs();

	// clear all previous powersave indication bits
	REGISTER &= (0xFFFFFFFF ^ ALL_STATES_BITMASK);

	// set for C3 mode
	REGISTER |= IN_C3_STATE;

	// lock access to backup
	pwr_save_lock_rtc_backup_regs();

}

// in HW-RevB this will keep internal VHF radio module working!
int pwr_save_switch_mode_to_m4(void) {

	if ((REGISTER & ALL_STATES_BITMASK) == IN_M4_STATE) {
		return 0;
	}

	// turn ON +5V_S (and internal VHF radio module in HW-RevB)
	io_5v_isol_sw___cntrl_vbat_s_enable();

	// turn OFF +5V_R and VBATT_SW_R
	io___cntrl_vbat_r_disable();

	// turn OFF +4V_G
	io_12v_sw___cntrl_vbat_g_disable();

	// unlock access to backup registers
	pwr_save_unclock_rtc_backup_regs();

	// clear all previous powersave indication bits
	REGISTER &= (0xFFFFFFFF ^ ALL_STATES_BITMASK);

	// set for C3 mode
	REGISTER |= IN_M4_STATE;

	// lock access to backup
	pwr_save_lock_rtc_backup_regs();

	return 1;
}

void pwr_save_switch_mode_to_i5(void) {

	if ((REGISTER & ALL_STATES_BITMASK) == IN_I5_STATE) {
		return;
	}

	// turn OFF +5V_S (and internal VHF radio module in HW-RevB)
	io_5v_isol_sw___cntrl_vbat_s_disable();

	// turn OFF +5V_R and VBATT_SW_R
	io___cntrl_vbat_r_disable();

	// turn OFF +4V_G
	io_12v_sw___cntrl_vbat_g_disable();

	// unlock access to backup registers
	pwr_save_unclock_rtc_backup_regs();

	// clear all previous powersave indication bits
	REGISTER &= (0xFFFFFFFF ^ ALL_STATES_BITMASK);

	// set for C3 mode
	REGISTER |= IN_I5_STATE;

	// lock access to backup
	pwr_save_lock_rtc_backup_regs();

}

// this will keep external VHF radio working in HW-RevB
void pwr_save_switch_mode_to_l6(uint16_t sleep_time) {

	if ((REGISTER & ALL_STATES_BITMASK) == IN_L6_STATE) {
		return;
	}

	// turn OFF +5V_S (and internal VHF radio module in HW-RevB)
	io_5v_isol_sw___cntrl_vbat_s_disable();

	// turn OFF +5V_R and VBATT_SW_R
	io___cntrl_vbat_r_disable();

	// turn ON +4V_G
	io_12v_sw___cntrl_vbat_g_enable();

	// unlock access to backup registers
	pwr_save_unclock_rtc_backup_regs();

	// clear all previous powersave indication bits
	REGISTER &= (0xFFFFFFFF ^ ALL_STATES_BITMASK);

	// set for C3 mode
	REGISTER |= IN_L6_STATE;

	// lock access to backup
	pwr_save_lock_rtc_backup_regs();

	system_clock_configure_auto_wakeup_l4(sleep_time);

	// save how long the micro will sleep - required for handling wakeup event
	pwr_save_sleep_time_in_seconds = sleep_time;

	// turn off leds to save power
	led_control_led1_upper(false);
	led_control_led2_bottom(false);

	pwr_save_enter_stop2();


}

void pwr_save_switch_mode_to_l7(uint16_t sleep_time) {

	if ((REGISTER & ALL_STATES_BITMASK) == IN_L7_STATE) {
		return;
	}

	// turn OFF +5V_S (and internal VHF radio module in HW-RevB)
	io_5v_isol_sw___cntrl_vbat_s_disable();

	// turn OFF +5V_R and VBATT_SW_R
	io___cntrl_vbat_r_disable();

	// turn OFF +4V_G
	io_12v_sw___cntrl_vbat_g_disable();

	// unlock access to backup registers
	pwr_save_unclock_rtc_backup_regs();

	// clear all previous powersave indication bits
	REGISTER &= (0xFFFFFFFF ^ ALL_STATES_BITMASK);

	// set for C3 mode
	REGISTER |= IN_L7_STATE;

	// lock access to backup
	pwr_save_lock_rtc_backup_regs();

	// configure how long micro should sleep
	system_clock_configure_auto_wakeup_l4(sleep_time);

	// save how long the micro will sleep - required for handling wakeup event
	pwr_save_sleep_time_in_seconds = sleep_time;

	// turn off leds to save power
	led_control_led1_upper(false);
	led_control_led2_bottom(false);

	pwr_save_enter_stop2();
}

void pwr_save_pooling_handler(const config_data_mode_t * config, const config_data_basic_t * timers, int16_t minutes_to_wx) {
	// this function should be called from 10 seconds pooler

	int reinit_sensors = 0;

	packet_tx_counter_values_t counters;

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
	switch (config->powersave) {
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
							reinit_sensors = pwr_save_switch_mode_to_m4();
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
							reinit_sensors = pwr_save_switch_mode_to_m4();
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
							// if there is more than two minutes to send wx packet
							pwr_save_switch_mode_to_l7((timers->wx_transmit_period * 60) - 120);
						}
						else {
							reinit_sensors= pwr_save_switch_mode_to_c1();
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
							// if stations is configured to send wx packet less often than every 5 minutes

							if (minutes_to_wx > 1) {
								// if there is more than one minute to wx packet
								pwr_save_switch_mode_to_l7((timers->wx_transmit_period * 60) - 60);				// TODO: !!!
							}
							else {
								if (pwr_save_seconds_to_wx <= 30) {
									// if there is 30 seconds or less to next wx packet
									reinit_sensors = pwr_save_switch_mode_to_c0();
								}
								else {
									// if there is 30 to 60 seconds to next wx packet
									reinit_sensors = pwr_save_switch_mode_to_m4();
								}
							}
						}
						else {
							// if station is configured to sent wx packet in every 5 minutes or more often

							if (minutes_to_wx > 1) {
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
							// if there is more than one minute to send wx packet
							pwr_save_switch_mode_to_l7((timers->wx_transmit_period * 60) - 60);
						}
						else {
							if (pwr_save_seconds_to_wx <= 30) {
								pwr_save_switch_mode_to_c1();

								// do not reinitialize everything as reinitialization had been done when switching to m4 mode
								reinit_sensors = 0;
							}
							else {
								reinit_sensors= pwr_save_switch_mode_to_m4();
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

	if (reinit_sensors != 0) {
		// reinitialize all i2c sensors
		wx_force_i2c_sensor_reset = 1;

		// reinitialize everything realted to anemometer
		analog_anemometer_init(main_config_data_mode->wx_anemometer_pulses_constant, 38, 100, 1);

//		// reset anemometer direction handler
//		analog_anemometer_direction_reset();
//
//		// reset anemometer windspeed handler
//		analog_anemometer_timer_irq();
	}

}

uint8_t pwr_save_get_inhibit_pwr_switch_periodic(void) {

	if ((REGISTER & INHIBIT_PWR_SWITCH_PERIODIC_H) != 0){
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

