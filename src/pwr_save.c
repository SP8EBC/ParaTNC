/*
 * pwr_save.c
 *
 *  Created on: Aug 22, 2021
 *      Author: mateusz
 */

#include "pwr_save.h"

#include "stm32l4xx.h"
#include <stdint.h>

#include "pwr_switch.h"
#include "io.h"

#define IN_STOP2_MODE (1 << 1)
#define IN_C0_STATE (1 << 2)
#define IN_C1_STATE (1 << 3)
#define IN_C2_STATE (1 << 4)
#define IN_C3_STATE (1 << 5)
#define IN_M4_STATE (1 << 6)
#define IN_I5_STATE (1 << 7)
#define IN_L6_STATE (1 << 8)
#define IN_L7_STATE (1 << 9)

#define CLEAR_ALL_STATES_BITMASK (0xFF << 2)

#if defined(STM32L471xx)

/**
 * This function initializes everything related to power saving features
 * including programming Flash memory option bytes
 */
void pwr_save_init(void) {

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

}

/**
 * Entering STOP2 power save mode. In this mode all clocks except LSI and LSE are disabled. StaticRAM content
 * is preserved, optionally GPIO and few other peripherals can be kept power up depending on configuration
 */
void pwr_save_enter_stop2(void) {

	// clear previous low power mode selection
	PWR->CR1 &= (0xFFFFFFFF ^ PWR_CR1_LPMS_Msk);

	// select STOP2
	PWR->CR1 |= PWR_CR1_LPMS_STOP2;

	// enable write access to RTC registers by writing two magic words
	RTC->WPR = 0xCA;
	RTC->WPR = 0x53;

	// save an information that STOP2 mode has been applied
	RTC->BKP0R |= IN_STOP2_MODE;

	SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

	DBGMCU->CR &= (0xFFFFFFFF ^ (DBGMCU_CR_DBG_SLEEP_Msk | DBGMCU_CR_DBG_STOP_Msk | DBGMCU_CR_DBG_STANDBY_Msk));

	// disabling all IRQs
	//__disable_irq();

	asm("sev");
	asm("wfi");

}

void pwr_save_switch_mode_to_c0(void) {

	// turn ON +5V_S (and internal VHF radio module in HW-RevB)
	io_5v_isol_sw___cntrl_vbat_s_enable();

	// turn ON +5V_R and VBATT_SW_R
	io___cntrl_vbat_r_enable();

	// turn ON +4V_G
	io_12v_sw___cntrl_vbat_g_enable();

	// clear all previous powersave indication bits
	RTC->BKP0R &= 0xFFFFFFFF ^ CLEAR_ALL_STATES_BITMASK;

	// set for C0 mode
	RTC->BKP0R |= IN_C0_STATE;

}

// in HW-RevB this will disable external VHF radio!!
void pwr_save_switch_mode_to_c1(void) {
	// turn ON +5V_S (and internal VHF radio module in HW-RevB)
	io_5v_isol_sw___cntrl_vbat_s_enable();

	// turn ON +5V_R and VBATT_SW_R
	io___cntrl_vbat_r_enable();

	// turn OFF +4V_G
	io_12v_sw___cntrl_vbat_g_disable();

	// clear all previous powersave indication bits
	RTC->BKP0R &= (0xFFFFFFFF ^ CLEAR_ALL_STATES_BITMASK);

	// set for C0 mode
	RTC->BKP0R |= IN_C1_STATE;
}

// this mode is not avaliable in HW Revision B as internal radio
// is powered from +5V_S and external one is switched on with the same
// line which controls +4V_G
void pwr_save_switch_mode_to_c2(void) {
	// turn OFF +5V_S (and internal VHF radio module in HW-RevB)
	io_5v_isol_sw___cntrl_vbat_s_disable();

	// turn ON +5V_R and VBATT_SW_R
	io___cntrl_vbat_r_enable();

	// turn OFF +4V_G
	io_12v_sw___cntrl_vbat_g_disable();

	// clear all previous powersave indication bits
	RTC->BKP0R &= (0xFFFFFFFF ^ CLEAR_ALL_STATES_BITMASK);

	// set for C2 mode
	RTC->BKP0R |= IN_C2_STATE;
}

void pwr_save_switch_mode_to_c3(void) {
	// turn OFF +5V_S (and internal VHF radio module in HW-RevB)
	io_5v_isol_sw___cntrl_vbat_s_disable();

	// turn ON +5V_R and VBATT_SW_R
	io___cntrl_vbat_r_enable();

	// turn ON +4V_G
	io_12v_sw___cntrl_vbat_g_enable();

	// clear all previous powersave indication bits
	RTC->BKP0R &= (0xFFFFFFFF ^ CLEAR_ALL_STATES_BITMASK);

	// set for C3 mode
	RTC->BKP0R |= IN_C3_STATE;
}

// in HW-RevB this will keep internal VHF radio module working!
void pwr_save_switch_mode_to_m4(void) {
	// turn ON +5V_S (and internal VHF radio module in HW-RevB)
	io_5v_isol_sw___cntrl_vbat_s_enable();

	// turn OFF +5V_R and VBATT_SW_R
	io___cntrl_vbat_r_disable();

	// turn OFF +4V_G
	io_12v_sw___cntrl_vbat_g_disable();

	// clear all previous powersave indication bits
	RTC->BKP0R &= (0xFFFFFFFF ^ CLEAR_ALL_STATES_BITMASK);

	// set for C3 mode
	RTC->BKP0R |= IN_M4_STATE;
}

void pwr_save_switch_mode_to_i5(void) {
	// turn OFF +5V_S (and internal VHF radio module in HW-RevB)
	io_5v_isol_sw___cntrl_vbat_s_disable();

	// turn OFF +5V_R and VBATT_SW_R
	io___cntrl_vbat_r_disable();

	// turn OFF +4V_G
	io_12v_sw___cntrl_vbat_g_disable();

	// clear all previous powersave indication bits
	RTC->BKP0R &= (0xFFFFFFFF ^ CLEAR_ALL_STATES_BITMASK);

	// set for C3 mode
	RTC->BKP0R |= IN_I5_STATE;

}

// this will keep external VHF radio working in HW-RevB
void pwr_save_switch_mode_to_l6(void) {
	// turn OFF +5V_S (and internal VHF radio module in HW-RevB)
	io_5v_isol_sw___cntrl_vbat_s_disable();

	// turn OFF +5V_R and VBATT_SW_R
	io___cntrl_vbat_r_disable();

	// turn ON +4V_G
	io_12v_sw___cntrl_vbat_g_enable();

	// clear all previous powersave indication bits
	RTC->BKP0R &= (0xFFFFFFFF ^ CLEAR_ALL_STATES_BITMASK);

	// set for C3 mode
	RTC->BKP0R |= IN_L6_STATE;

	pwr_save_enter_stop2();

}

void pwr_save_switch_mode_to_l7(void) {
	// turn OFF +5V_S (and internal VHF radio module in HW-RevB)
	io_5v_isol_sw___cntrl_vbat_s_disable();

	// turn OFF +5V_R and VBATT_SW_R
	io___cntrl_vbat_r_disable();

	// turn OFF +4V_G
	io_12v_sw___cntrl_vbat_g_disable();

	// clear all previous powersave indication bits
	RTC->BKP0R &= (0xFFFFFFFF ^ CLEAR_ALL_STATES_BITMASK);

	// set for C3 mode
	RTC->BKP0R |= IN_L7_STATE;

	pwr_save_enter_stop2();
}

#endif

