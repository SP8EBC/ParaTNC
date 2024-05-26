/*
 * variant.c
 *
 *  Created on: Oct 24, 2023
 *      Author: mateusz
 */

#include "variant.h"

#ifdef STM32L471xx
#include <stm32l4xx.h>
#endif

int variant_validate_is_within_ram(void * address) {

#ifdef STM32L471xx
	uint32_t addr_value = (uint32_t)address;

	if (addr_value > SRAM_BASE &&
		addr_value < SRAM_BASE + SRAM1_SIZE_MAX) {
		return 1;
	}
	else {
		return 0;
	}
#else
	return 1;
#endif
}

int variant_validate_is_within_flash(void * address) {
#ifdef STM32L471xx
	uint32_t addr_value = (uint32_t)address;

	if (addr_value > FLASH_BASE &&
		addr_value < FLASH_BANK1_END) {
		return 1;
	}
	else {
		return 0;
	}
#else
	return 1;
#endif
}
