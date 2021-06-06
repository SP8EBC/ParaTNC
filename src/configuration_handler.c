/*
 * configuration_handler.c
 *
 *  Created on: Apr 28, 2021
 *      Author: mateusz
 */

#include "configuration_handler.h"
#include "config_data.h"
#include "config_data_externs.h"

#include "main.h"

#ifdef STM32F10X_MD_VL
#include <stm32f10x.h>
#include <stm32f10x_crc.h>
#include <stm32f10x_flash.h>
#endif

#ifdef STM32L471xx
#include "./drivers/l4/flash_stm32l4x.h"
#include <stm32l4xx.h>
#include <stm32l4xx_ll_crc.h>
#endif


#include <string.h>

const uint32_t * config_section_first_start = (uint32_t *)0x0801E800;
const uint32_t * config_section_second_start = (uint32_t *)0x0801F000;

#define CRC_OFFSET				0x7FC
#define CRC_16B_WORD_OFFSET		CRC_OFFSET / 2
#define CRC_32B_WORD_OFFSET		CRC_OFFSET / 4

#define CONFIG_SECTION_LN 0x7FF


volatile extern const config_data_basic_t config_data_basic_default;
volatile extern const config_data_mode_t config_data_mode_default;
volatile extern const config_data_umb_t config_data_umb_default;
volatile extern const config_data_rtu_t config_data_rtu_default;
volatile extern const config_data_wx_sources_t config_data_wx_sources_default;

uint32_t configuration_handler_check_crc(void) {

	uint32_t out = 0;

	// crc stored in the configuration section
	uint32_t crc_expected = 0;

	// calculated CRC value
	uint32_t crc_current = 0;

	// reset CRC engine
	CRC_ResetDR();

	// calculate CRC over everything from config_section_first except the last word which constit crc value itself
	CRC_CalcBlockCRC(config_section_first_start, CRC_32B_WORD_OFFSET - 1);

	// add 0x0 as a placeholder for CRC value
	crc_current = CRC_CalcCRC(0x0);

	// expected crc is stored in the last 32b word of the configuration section
	crc_expected = *(config_section_first_start + CRC_32B_WORD_OFFSET);

	// check if calculated CRC value match value stored in flash memory
	if (crc_expected == crc_current) {
		out |= 0x01;
	}

	// reset the CRC engine
	CRC_ResetDR();

	// and do the same but for second section
	CRC_CalcBlockCRC(config_section_second_start, CRC_32B_WORD_OFFSET - 1);

	// add 0x0 as a placeholder for CRC value
	crc_current = CRC_CalcCRC((uint32_t)0x0);

	//crc_expected = *__config_section_second_end;

	// check if calculated CRC value match value stored in flash memory
	if (crc_expected == crc_current) {
		out |= 0x02;
	}
	return out;
}

uint32_t configuration_handler_restore_default_first(void) {

	uint32_t out = 0;

	// loop iterators
	int i = 0;
	int8_t config_struct_it = 0;

	// source pointer
	volatile uint16_t * source = 0x00;

	// destination pointer for flash reprogramming
	volatile uint16_t * target = 0x00;

	// amount of 16 bit words to copy across the memory
	uint16_t size = 0;

	// target region CRC value to be stored in the flash memory
	uint32_t target_crc_value = 0;

	// flash operation result
	FLASH_Status flash_status = 0;

	int comparision_result = 0;

	// unlock flash memory
	FLASH_Unlock();

	// erase first page
	flash_status = FLASH_ErasePage((uint32_t)config_section_first_start);
	flash_status = FLASH_ErasePage((uint32_t)config_section_first_start + 0x400);

	// check if erasure was completed successfully
	if (flash_status == FLASH_COMPLETE) {

		for (config_struct_it = 0; config_struct_it < 5; config_struct_it++) {

			// set pointers
			switch (config_struct_it) {
				case 0:	// mode
					source = (uint16_t *) &config_data_mode_default;
					target = (uint16_t *) &config_data_mode_first;
					size = sizeof(config_data_mode_t) / 2;
					break;
				case 1:	// basic
					source = (uint16_t *) &config_data_basic_default;
					target = (uint16_t *) &config_data_basic_first;
					size = sizeof(config_data_basic_t) / 2;
					break;
				case 2:	// sources
					source = (uint16_t *) &config_data_wx_sources_default;
					target = (uint16_t *) &config_data_wx_sources_first;
					size = sizeof(config_data_wx_sources_t) / 2;
					break;
				case 3:
					source = (uint16_t *) &config_data_umb_default;
					target = (uint16_t *) &config_data_umb_first;
					size = sizeof(config_data_umb_t) / 2;
					break;
				case 4:
					source = (uint16_t *) &config_data_rtu_default;
					target = (uint16_t *) &config_data_rtu_first;
					size = sizeof(config_data_umb_t) / 2;
					break;
			}


			// enable programming
			FLASH->CR |= FLASH_CR_PG;

			// if so reprogram first section
			for (i = 0; i < size; i++) {

				// copy data
				*(target + i) = *(source + i);

				// wait for flash operation to finish
				while (1) {
					// check current status
					flash_status = FLASH_GetBank1Status();

					if (flash_status == FLASH_BUSY) {
						;
					}
					else {
						break;
					}
				}

				if (flash_status != FLASH_COMPLETE) {
					break;	// exit from the loop in case of programming error
				}

			}

			// verify programming
			comparision_result = memcmp((const void * )target, (const void * )source, size * 2);

			if (comparision_result != 0) {
				// quit from the
				out = -1;

				return out;
			}
		}
	}

	// set programming counter. If second region is also screwed the first one will be used as a source
	// if second is OK it will be used instead (if its programming counter has value three or more).
	*(uint16_t*)&config_data_pgm_cntr_first = 0x0002u;

	// resetting CRC engine
	CRC_ResetDR();

	// calculate CRC checksum of the first block
	CRC_CalcBlockCRC(config_section_first_start, CRC_32B_WORD_OFFSET - 1);

	// adding finalizing 0x00
	target_crc_value = CRC_CalcCRC((uint32_t)0x0);

	// program the CRC value
	*(uint16_t*)((uint16_t *)config_section_first_start + CRC_16B_WORD_OFFSET) = (uint16_t)(target_crc_value & 0xFFFF);
	*(uint16_t*)((uint16_t *)config_section_first_start + CRC_16B_WORD_OFFSET + 1) = (uint16_t)((target_crc_value & 0xFFFF0000) >> 16);

	flash_status = FLASH_GetBank1Status();

	if (flash_status != FLASH_COMPLETE) {
		out = -2;	// exit from the loop in case of programming error
	}

	// disable programming
	FLASH->CR &= (0xFFFFFFFF ^ FLASH_CR_PG);

	// lock the memory back
	FLASH_Lock();

	return out;

}

uint32_t configuration_handler_restore_default_second(void) {
	uint32_t out = 0;

	// loop iterators
	int i = 0;
	int8_t config_struct_it = 0;

	// source pointer
	volatile uint16_t * source = 0x00;

	// destination pointer for flash reprogramming
	volatile uint16_t * target = 0x00;

	// amount of 16 bit words to copy across the memory
	uint16_t size = 0;

	// target region CRC value to be stored in the flash memory
	uint32_t target_crc_value = 0;

	// flash operation result
	FLASH_Status flash_status = 0;

	int comparision_result = 0;

	// unlock flash memory
	FLASH_Unlock();

	// erase first page
	flash_status = FLASH_ErasePage((uint32_t)config_section_second_start);
	flash_status = FLASH_ErasePage((uint32_t)config_section_second_start + 0x400);

	// check if erasure was completed successfully
	if (flash_status == FLASH_COMPLETE) {

		for (config_struct_it = 0; config_struct_it < 5; config_struct_it++) {

			// set pointers
			switch (config_struct_it) {
				case 0:	// mode
					source = (uint16_t *) &config_data_mode_default;
					target = (uint16_t *) &config_data_mode_second;
					size = sizeof(config_data_mode_t) / 2;
					break;
				case 1:	// basic
					source = (uint16_t *) &config_data_basic_default;
					target = (uint16_t *) &config_data_basic_second;
					size = sizeof(config_data_basic_t) / 2;
					break;
				case 2:	// sources
					source = (uint16_t *) &config_data_wx_sources_default;
					target = (uint16_t *) &config_data_wx_sources_second;
					size = sizeof(config_data_wx_sources_t) / 2;
					break;
				case 3:
					source = (uint16_t *) &config_data_umb_default;
					target = (uint16_t *) &config_data_umb_second;
					size = sizeof(config_data_umb_t) / 2;
					break;
				case 4:
					source = (uint16_t *) &config_data_rtu_default;
					target = (uint16_t *) &config_data_rtu_second;
					size = sizeof(config_data_umb_t) / 2;
					break;
			}


			// enable programming
			FLASH->CR |= FLASH_CR_PG;

			// if so reprogram first section
			for (i = 0; i < size; i++) {

				// copy data
				*(target + i) = *(source + i);

				// wait for flash operation to finish
				while (1) {
					// check current status
					flash_status = FLASH_GetBank1Status();

					if (flash_status == FLASH_BUSY) {
						;
					}
					else {
						break;
					}
				}

				if (flash_status != FLASH_COMPLETE) {
					break;	// exit from the loop in case of programming error
				}

			}

			// verify programming
			comparision_result = memcmp((const void * )target, (const void * )source, size * 2);

			if (comparision_result != 0) {
				// quit from the
				out = -1;

				return out;
			}
		}
	}

	// set programming counter. If second region is also screwed the first one will be used as a source
	// if second is OK it will be used instead (if its programming counter has value three or more).
	*(uint16_t*)&config_data_pgm_cntr_second = 0x0002u;

	// resetting CRC engine
	CRC_ResetDR();

	// calculate CRC checksum of the first block
	CRC_CalcBlockCRC(config_section_first_start, CRC_32B_WORD_OFFSET - 1);

	// adding finalizing 0x00
	target_crc_value = CRC_CalcCRC((uint32_t)0x0);

	// program the CRC value
	*(uint16_t*)((uint16_t *)config_section_second_start + CRC_16B_WORD_OFFSET) = (uint16_t)(target_crc_value & 0xFFFF);
	*(uint16_t*)((uint16_t *)config_section_second_start + CRC_16B_WORD_OFFSET + 1) = (uint16_t)((target_crc_value & 0xFFFF0000) >> 16);

	flash_status = FLASH_GetBank1Status();

	if (flash_status != FLASH_COMPLETE) {
		out = -2;	// exit from the loop in case of programming error
	}

	// disable programming
	FLASH->CR &= (0xFFFFFFFF ^ FLASH_CR_PG);

	// lock the memory back
	FLASH_Lock();

	return out;}

void configuration_handler_load_configuration(configuration_handler_region_t region) {

	if (region == REGION_DEFAULT) {
		main_config_data_mode = &config_data_mode_default;
		main_config_data_basic = &config_data_basic_default;
		main_config_data_wx_sources = &config_data_wx_sources_default;
		main_config_data_umb = &config_data_umb_default;
		main_config_data_rtu = &config_data_rtu_default;
	}
	else if (region == REGION_FIRST) {
		main_config_data_mode = &config_data_mode_first;
		main_config_data_basic = &config_data_basic_first;
		main_config_data_wx_sources = &config_data_wx_sources_first;
		main_config_data_umb = &config_data_umb_first;
		main_config_data_rtu = &config_data_rtu_first;
	}
	else if (region == REGION_SECOND) {
		main_config_data_mode = &config_data_mode_second;
		main_config_data_basic = &config_data_basic_second;
		main_config_data_wx_sources = &config_data_wx_sources_second;
		main_config_data_umb = &config_data_umb_second;
		main_config_data_rtu = &config_data_rtu_second;
	}
	else {
		;
	}

}

uint32_t configuration_handler_program(uint8_t* data, uint16_t data_ln, uint8_t config_idx) {
	return -1;
}
