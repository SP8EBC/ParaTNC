/*
 * nvm.c
 *
 *  Created on: Nov 1, 2022
 *      Author: mateusz
 */

#include "./nvm/nvm.h"
#include "./nvm/nvm_internals.h"
#include "nvm_configuration.h"
#include "memory_map.h"
#include "backup_registers.h"

#define KB *1024

#ifdef STM32L471xx
//#include <stm32l4xx_hal_cortex.h>
#include <stm32l4xx.h>
#include "./drivers/l4/flash_stm32l4x.h"

#else

#endif

//!< Size of NVM data logger in pages
//#define NVM_MEASUREMENT_PAGES_USED		96

//!< Size of NVM data logger in bytes
#define NVM_MEASUREMENT_MAXIMUM_SIZ		(NVM_PAGE_SIZE * nvm_measurement_page_used)

//!< A macro to calculate start address of last page for NVM data logger
#define START_OF_LAST_NVM_PAGE			(nvm_measurement_base_address + NVM_MEASUREMENT_MAXIMUM_SIZ - NVM_PAGE_SIZE)

//!< Base address of NVM data logger for device with 1MB of Flash
#define LOGGER_BASE_ADDRESS_1MB_DEVICE	MEMORY_MAP_MEASUREMENT_1M_START	// Page 256 from 352

#define LOGGER_BASE_ADDRESS_512K_DEVICE	MEMORY_MAP_MEASUREMENT_512K_START	// Page 130 from 178


uint32_t nvm_measurement_base_address = 0;

//!< Start address of flash page used currently for NVM
uint32_t nvm_measurement_current_page_address = 0;

//!< How many
uint8_t nvm_measurement_page_used = 0;

/**
 * Pointer to
 */
uint8_t * nvm_data_ptr = 0;

static nvm_state_after_last_oper_t nvm_general_state = NVM_UNINITIALIZED;


/**
 *
 */
void nvm_measurement_init(void) {

	uint8_t data = 0;

#if defined(STM32L471xx)
	// flash operation result
	FLASH_Status flash_status = 0;

	// check current flash size
	if (FLASH_SIZE == 1024 KB) {
		// 1024KB
		nvm_measurement_base_address = LOGGER_BASE_ADDRESS_1MB_DEVICE;
		nvm_measurement_page_used = MEMORY_MAP_MEASUREMENT_1M_PAGES;
	}
	else if (FLASH_SIZE == 512 KB) {
		// 512KB
		nvm_measurement_base_address = LOGGER_BASE_ADDRESS_512K_DEVICE;
		nvm_measurement_page_used = MEMORY_MAP_MEASUREMENT_512K_PAGES;
	}
	else {
		// unknown device ??
		nvm_general_state = NVM_INIT_ERROR;

		return;
	}

	// find the first non-erased page
	for (uint32_t i = nvm_measurement_base_address; i < (nvm_measurement_base_address + NVM_MEASUREMENT_MAXIMUM_SIZ); i += NVM_PAGE_SIZE) {
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

		// disable programming
		FLASH->CR &= (0xFFFFFFFF ^ FLASH_CR_PG);

		FLASH_Lock();
	}
	else {
		// ig no there is no space on the flash memory
		nvm_general_state = NVM_NO_SPACE_LEFT;
	}
#endif
}

/**
 *
 * @param data
 * @return
 */
nvm_state_after_last_oper_t nvm_measurement_store(nvm_measurement_t * data) {

	nvm_state_after_last_oper_t out = NVM_OK;

#if defined(STM32L471xx)

	volatile uint32_t * src = (uint32_t *)data;

	uint32_t next_page_addr = 0;

	// flash operation result
	FLASH_Status flash_status = 0;

	// check if there is a room for storage
	if (nvm_general_state == NVM_NO_SPACE_LEFT) {
		// erase first page of NVM flash area

		nvm_data_ptr = (uint8_t*)nvm_measurement_base_address;

		flash_status = FLASH_ErasePage((uint32_t)nvm_data_ptr);

		if (flash_status == FLASH_COMPLETE) {
			nvm_general_state = NVM_OK;
		}
		else {
			nvm_general_state = NVM_PGM_ERROR;
		}

	}
	else {
		// check if currently last page is used
		if ((uint32_t)nvm_data_ptr < START_OF_LAST_NVM_PAGE) {
			// if not check if next page of flash memory is erased or not
			if (*(nvm_data_ptr + NVM_PAGE_SIZE) != 0xFF) {
				// an address somewhere within the next page of memory
				next_page_addr = (uint32_t)nvm_data_ptr + NVM_PAGE_SIZE;

				// a start of next flash page
				next_page_addr = (next_page_addr / NVM_PAGE_SIZE) * NVM_PAGE_SIZE;

				flash_status = FLASH_ErasePage(next_page_addr);

				if (flash_status != FLASH_COMPLETE) {
					nvm_general_state = NVM_PGM_ERROR;
				}
			}
		}
	}

	FLASH_Unlock();

	// check if NVM has been initialized and it is ready to work
	if (nvm_general_state != NVM_UNINITIALIZED) {
		// check if there is a room to store this measurement
		if (nvm_general_state != NVM_NO_SPACE_LEFT) {

			// enable programming
			FLASH->CR |= FLASH_CR_PG;

			// progam this measurement
			for (int i = 0 ; i < NVM_RECORD_SIZE / 4; i++) {
				*((uint32_t*)(nvm_data_ptr) + i) = *(src + i);
				WAIT_FOR_PGM_COMPLETION

				if (flash_status != FLASH_COMPLETE) {
					break;
				}
			}

			// move data pointer
			nvm_data_ptr += NVM_WRITE_BYTE_ALIGN;

			// and check if an end has
			if ((uint32_t)nvm_data_ptr < nvm_measurement_base_address + NVM_MEASUREMENT_MAXIMUM_SIZ) {
				;
			}
			else {
				out = NVM_NO_SPACE_LEFT;
				nvm_general_state = NVM_NO_SPACE_LEFT;
			}

			// disable programming
			FLASH->CR &= (0xFFFFFFFF ^ FLASH_CR_PG);

		}
	}

	FLASH_Lock();

#endif
	return out;
}

/**
 *
 */
void nvm_erase_all(void) {
#if defined(STM32L471xx)

	uint32_t base_address = 0;

	FLASH_Unlock();

	// check current flash size
	if (FLASH_SIZE == 1024 KB) {
		// 1024KB
		base_address = 0x08080000;
	}
	else if (FLASH_SIZE == 512 KB) {
		// 512KB
		base_address = 0x08040000;
	}
	else {
		return;
	}

	for (int i = 0; i < nvm_measurement_page_used; i++) {
		FLASH_ErasePage(base_address + (i * NVM_PAGE_SIZE));
	}

	FLASH_Lock();
#endif
}

/**
 *
 */
void nvm_test_prefill(void) {
#if defined(STM32L471xx)

	uint32_t * base_address = 0;

	// flash operation result
	FLASH_Status flash_status = 0;

	FLASH_Unlock();

	// enable programming
	FLASH->CR |= FLASH_CR_PG;

	// check current flash size
	if (FLASH_SIZE == 1024 KB) {
		// 1024KB
		base_address = (uint32_t *)0x08080000;
	}
	else if (FLASH_SIZE == 512 KB) {
		// 512KB
		base_address = (uint32_t *)0x08040000;
	}
	else {
		return;
	}

	// program first 10 pages of flash with 0xABCDABCD / 0x99889988
	for (int i = 0; i < 10; i++) {

		// flash programming must be 64 bit (8 bytes) aligned
		for (int j = 0; j < NVM_PAGE_SIZE / 8; j++) {
			*(base_address++) = 0xABCDABCD;
			WAIT_FOR_PGM_COMPLETION

			*(base_address++) = 0x99889988;
			WAIT_FOR_PGM_COMPLETION
		}
	}

	if (FLASH_SIZE == 1024 KB) {
		// 1024KB
		base_address = (uint32_t *)(0x08080000 + 11 * NVM_PAGE_SIZE);
	}
	else if (FLASH_SIZE == 512 KB) {
		// 512KB
		base_address = (uint32_t *)(0x08040000 + 11 * NVM_PAGE_SIZE);
	}
	else {
		return;
	}

	for (int i = 11; i < nvm_measurement_page_used; i++) {
		// flash programming must be 64 bit (8 bytes) aligned
		for (int j = 0; j < NVM_PAGE_SIZE / 8; j++) {
			*(base_address++) = 0x12345678;
			WAIT_FOR_PGM_COMPLETION

			*(base_address++) = 0x99999999;
			WAIT_FOR_PGM_COMPLETION
		}
	}
	// disable programming
	FLASH->CR &= (0xFFFFFFFF ^ FLASH_CR_PG);

	FLASH_Lock();

#endif
}
