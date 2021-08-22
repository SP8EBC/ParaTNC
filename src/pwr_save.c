/*
 * pwr_save.c
 *
 *  Created on: Aug 22, 2021
 *      Author: mateusz
 */

#include "pwr_save.h"

#include "stm32l4xx.h"
#include <stdint.h>

#define IN_STOP2_MODE (1 << 1)

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
	RTC->BKP4R |= IN_STOP2_MODE;

	SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

	DBGMCU->CR &= (0xFFFFFFFF ^ (DBGMCU_CR_DBG_SLEEP_Msk | DBGMCU_CR_DBG_STOP_Msk | DBGMCU_CR_DBG_STANDBY_Msk));

	// disabling all IRQs
	__disable_irq();

	asm ("wfe");

}

#endif

