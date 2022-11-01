/*
 * nvm.c
 *
 *  Created on: Nov 1, 2022
 *      Author: mateusz
 */

#include "nvm.h"

#define KB *1024

#ifdef STM32L471xx
#include <stm32l4xx_hal_cortex.h>
#include <stm32l4xx.h>
#include "./drivers/l4/flash_stm32l4x.h"

#define NVM_PAGE_SIZE				2048
#define NVM_WRITE_BYTE_ALIGN		8

#endif

#define NVM_MEASUREMENT_OFFSET			0
#define NVM_MEASUREMENT_MAXIMUM_SIZ		(NVM_PAGE_SIZE * 96)

uint32_t nvm_base_address = 0;

/**
 * Start address of flash page used currently for NVM
 */
uint32_t nvm_current_page_address = 0;

/**
 * Pointer to
 */
uint8_t * nvm_data_ptr = 0;

nvm_state_result_t nvm_general_state = NVM_UNINITIALIZED;

#define WAIT_FOR_PGM_COMPLETION			\
	while (1) {\
		flash_status = FLASH_GetBank1Status();				\
															\
		if (flash_status == FLASH_BUSY) {					\
			;												\
		}													\
		else if (flash_status == FLASH_ERROR_PG) {			\
			nvm_general_state = NVM_PGM_ERROR;				\
			break;											\
		}													\
		else {												\
			break;											\
		}													\
	}														\

void nvm_measurement_init(void) {

	uint8_t data = 0;

	// flash operation result
	FLASH_Status flash_status = 0;

#if defined(STM32L471xx)
	// check current flash size
	if (FLASH_SIZE == 1024 KB) {
		// 1024KB
		nvm_base_address = 0x08040000;
	}
	else if (FLASH_SIZE == 512 KB) {
		// 512KB
		nvm_base_address = 0x08080000;
	}
	else {
		// unknown device ??
		nvm_general_state = NVM_INIT_ERROR;

		return;
	}

	// find the first non-erased page
	for (uint32_t i = nvm_base_address; i < (nvm_base_address + NVM_MEASUREMENT_MAXIMUM_SIZ); i += NVM_PAGE_SIZE) {
		// get the content of first byte
		data = *((uint8_t*) i);

		// check if data is in erased state
		if (data == 0xFF) {
			// first byte is erased, set data pointer to start of this page
			nvm_data_ptr = (uint8_t*) i;

			// and break from the loop
			break;
		}

		// get the last byte of flash memory page
		data = *(((uint8_t*) i + NVM_PAGE_SIZE - 1));

		if (data == 0xFF) {
			// last byte is not erased
			nvm_data_ptr = (uint8_t*) i;

			break;
		}
	}

	// if free flash memory page has been found
	if (nvm_data_ptr != 0) {
		// go through memory page and find clean (erased state) 64 bit word
		for (int i = 0; i < NVM_PAGE_SIZE; i += NVM_WRITE_BYTE_ALIGN) {
			// get this byte
			data = *(nvm_data_ptr + i);

			if (data == 0xFF) {
				// rewind data pointer to this byte
				nvm_data_ptr += i;

				// and break the loop
				break;
			}
		}

		// program initialization mark
		// unlock flash memory
		FLASH_Unlock();

		// enable programming
		FLASH->CR |= FLASH_CR_PG;

		*((uint32_t*)(nvm_data_ptr)) = 0xDEADBEEFu;
		WAIT_FOR_PGM_COMPLETION

		*((uint32_t*)(nvm_data_ptr)+ 1) = 0x00000000u;
		WAIT_FOR_PGM_COMPLETION

		if (nvm_general_state != NVM_PGM_ERROR) {
			nvm_data_ptr += NVM_WRITE_BYTE_ALIGN;

			nvm_general_state = NVM_OK;
		}

		FLASH_Lock();
	}
	else {
		// ig no there is no space on the flash memory
		nvm_general_state = NVM_NO_SPACE_LEFT;
	}
#endif
}

nvm_state_result_t nvm_measurement_store(nvm_measurement_t * data) {

	// check if NVM has been initialized and it is ready to work
	if (nvm_general_state != NVM_UNINITIALIZED) {
		// check if there is a room to store this measurement
		if (nvm_general_state != NVM_NO_SPACE_LEFT) {
			// progam this measurement

			// move data pointer

			// and check if an end has been found
		}
	}

}
