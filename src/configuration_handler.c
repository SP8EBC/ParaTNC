/*
 * configuration_handler.c
 *
 *  Created on: Apr 28, 2021
 *      Author: mateusz
 */

#include "station_config_target_hw.h"

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

/**
 * STM32L476RE, last flash memory page
 * 0x0807F800 - 0x0807FFFF; 2 K; Page 383
 *
 *  __config_section_default_start =    0x0801E000;
    __config_section_first_start = 	    0x0801E800;		// page 61, 0x3D
    __config_section_second_start =     0x0801F000;		// page 62, 0x3E
    __config_section_third_start =      0x0801F800;		// page 63, 0x3F
 *
 */

#define CONFIG_SECTION_FIRST_START 		0x0801E800
#define CONFIG_SECTION_SECOND_START		0x0801F000
#define CONFIG_SECTION_DEFAULT_START	0x0801E000

#define CONFIG_MODE_PGM_CNTR	0x0
#define CONFIG_MODE_OFSET		0x20			//	Current size: 0xF
#define CONFIG_BASIC_OFFSET		0x40			//	Current size: 0x9C
#define CONFIG_SOURCES_OFFSET	0x120			//	Current size: 0x4
#define CONFIG_UMB_OFFSET		0x140			//	Current size: 0xE
#define CONFIG_RTU_OFFSET		0x160			//	Current size: 0x54
#define CONFIG_GSM_OFFSET		0x200			//	Current size: 0xF8
#define CONFIG__END__OFFSET		0x300

#include <string.h>

const uint32_t * const config_section_first_start = 		(const uint32_t *)CONFIG_SECTION_FIRST_START;
const uint32_t * const config_section_second_start = 		(const uint32_t *)CONFIG_SECTION_SECOND_START;
const uint32_t * const config_section_default_start = 		(const uint32_t *)CONFIG_SECTION_DEFAULT_START;

#ifdef STM32L471xx
const uint16_t * config_data_pgm_cntr_first_ptr		= (uint16_t*)(CONFIG_SECTION_FIRST_START + CONFIG_MODE_PGM_CNTR);
const uint16_t * config_data_pgm_cntr_second_ptr	= (uint16_t*)(CONFIG_SECTION_SECOND_START + CONFIG_MODE_PGM_CNTR);

const config_data_mode_t * config_data_mode_first_ptr 				= (const config_data_mode_t * )		(CONFIG_SECTION_FIRST_START + CONFIG_MODE_OFSET);
const config_data_basic_t * config_data_basic_first_ptr 			= (const config_data_basic_t *)		(CONFIG_SECTION_FIRST_START + CONFIG_BASIC_OFFSET);
const config_data_wx_sources_t * config_data_wx_sources_first_ptr 	= (const config_data_wx_sources_t *)(CONFIG_SECTION_FIRST_START + CONFIG_SOURCES_OFFSET);
const config_data_umb_t * config_data_umb_first_ptr					= (const config_data_umb_t *)		(CONFIG_SECTION_FIRST_START + CONFIG_UMB_OFFSET);
const config_data_rtu_t * config_data_rtu_first_ptr					= (const config_data_rtu_t *)		(CONFIG_SECTION_FIRST_START + CONFIG_RTU_OFFSET);
const config_data_gsm_t * config_data_gsm_first_ptr					= (const config_data_gsm_t *)		(CONFIG_SECTION_FIRST_START + CONFIG_GSM_OFFSET);

const config_data_mode_t * config_data_mode_second_ptr 						= (const config_data_mode_t * )		(CONFIG_SECTION_SECOND_START + CONFIG_MODE_OFSET);
const config_data_basic_t * config_data_basic_second_ptr 					= (const config_data_basic_t *)		(CONFIG_SECTION_SECOND_START + CONFIG_BASIC_OFFSET);
const config_data_wx_sources_t * config_data_wx_sources_second_ptr			= (const config_data_wx_sources_t *)(CONFIG_SECTION_SECOND_START + CONFIG_SOURCES_OFFSET);
const config_data_umb_t * config_data_umb_second_ptr 						= (const config_data_umb_t *)		(CONFIG_SECTION_SECOND_START + CONFIG_UMB_OFFSET);
const config_data_rtu_t * config_data_rtu_second_ptr						= (const config_data_rtu_t *)		(CONFIG_SECTION_SECOND_START + CONFIG_RTU_OFFSET);
const config_data_gsm_t * config_data_gsm_second_ptr						= (const config_data_gsm_t *)		(CONFIG_SECTION_SECOND_START + CONFIG_GSM_OFFSET);

const config_data_gsm_t * config_data_gsm_default_ptr = (const config_data_gsm_t *)&config_data_gsm_default;

#endif

#ifdef STM32F10X_MD_VL
const uint16_t * config_data_pgm_cntr_first_ptr		= &config_data_pgm_cntr_first;
const uint16_t * config_data_pgm_cntr_second_ptr	= &config_data_pgm_cntr_second;

const config_data_mode_t * config_data_mode_first_ptr 				= &config_data_mode_first;
const config_data_basic_t * config_data_basic_first_ptr 			= &config_data_basic_first;
const config_data_wx_sources_t * config_data_wx_sources_first_ptr 	= &config_data_wx_sources_first;
const config_data_umb_t * config_data_umb_first_ptr					= &config_data_umb_first;
const config_data_rtu_t * config_data_rtu_first_ptr					= &config_data_rtu_first;

const config_data_mode_t * config_data_mode_second_ptr 						= &config_data_mode_second;
const config_data_basic_t * config_data_basic_second_ptr 					= &config_data_basic_second;
const config_data_wx_sources_t * config_data_wx_sources_second_ptr			= &config_data_wx_sources_second;
const config_data_umb_t * config_data_umb_second_ptr 						= &config_data_umb_second;
const config_data_rtu_t * config_data_rtu_second_ptr						= &config_data_rtu_second;

#endif

#define CRC_OFFSET				0x7FC
#define CRC_16B_WORD_OFFSET		CRC_OFFSET / 2
#define CRC_32B_WORD_OFFSET		CRC_OFFSET / 4

#define CONFIG_SECTION_LN 0x7FF

#define FEND	(uint8_t)0xC0
#define FESC	(uint8_t)0xDB
#define TFEND	(uint8_t)0xDC
#define TFESC	(uint8_t)0xDD

#define KISS_GET_RUNNING_CONFIG 	(uint8_t) 0x20
#define KISS_RUNNING_CONFIG			(uint8_t) 0x70

volatile extern const config_data_basic_t config_data_basic_default;
volatile extern const config_data_mode_t config_data_mode_default;
volatile extern const config_data_umb_t config_data_umb_default;
volatile extern const config_data_rtu_t config_data_rtu_default;
volatile extern const config_data_wx_sources_t config_data_wx_sources_default;

configuration_handler_region_t configuration_handler_loaded;

static const uint8_t kiss_config_preamble[] = {FEND, KISS_RUNNING_CONFIG, CONFIG_SECTION_LN & 0xFF, (CONFIG_SECTION_LN & 0xFF00) >> 8};

uint8_t config_kiss_flash_state = 0;

uint32_t configuration_handler_check_crc(void) {

	uint32_t out = 0;

	// crc stored in the configuration section
	uint32_t crc_expected = 0;

	// calculated CRC value
	uint32_t crc_current = 0;

#ifdef STM32F10X_MD_VL
	// reset CRC engine
	CRC_ResetDR();

	// calculate CRC over everything from config_section_first except the last word which constit crc value itself
	CRC_CalcBlockCRC(config_section_first_start, CRC_32B_WORD_OFFSET - 1);

	// add 0x0 as a placeholder for CRC value
	crc_current = CRC_CalcCRC(0x0);
#endif

#ifdef STM32L471xx

	// reset CRC engine
	LL_CRC_ResetCRCCalculationUnit(CRC);

	for (int i = 0; i < CRC_32B_WORD_OFFSET - 1; i++) {
		// feed the data into CRC engine
		LL_CRC_FeedData32(CRC, *(config_section_first_start + i));
	}

	// placeholder for CRC value itself
	CRC->DR = 0x00;

	crc_current = CRC->DR;
#endif

	// expected crc is stored in the last 32b word of the configuration section
	crc_expected = *(config_section_first_start + CRC_32B_WORD_OFFSET);

	// check if calculated CRC value match value stored in flash memory
	if (crc_expected == crc_current) {
		out |= 0x01;
	}

#ifdef STM32F10X_MD_VL
	// reset the CRC engine
	CRC_ResetDR();

	// and do the same but for second section
	CRC_CalcBlockCRC(config_section_second_start, CRC_32B_WORD_OFFSET - 1);

	// add 0x0 as a placeholder for CRC value
	crc_current = CRC_CalcCRC((uint32_t)0x0);
#endif

#ifdef STM32L471xx
	// reset CRC engine
	LL_CRC_ResetCRCCalculationUnit(CRC);

	for (int i = 0; i < CRC_32B_WORD_OFFSET - 1; i++) {
		// feed the data into CRC engine
		LL_CRC_FeedData32(CRC, *(config_section_second_start + i));
	}

	// placeholder for CRC value itself
	CRC->DR = 0x00;

	crc_current = CRC->DR;
#endif

	//crc_expected = *__config_section_second_end;
	crc_expected = *(config_section_second_start + CRC_32B_WORD_OFFSET);

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
					target = (uint16_t *) config_data_mode_first_ptr;
					size = sizeof(config_data_mode_t) / 2;
					break;
				case 1:	// basic
					source = (uint16_t *) &config_data_basic_default;
					target = (uint16_t *) config_data_basic_first_ptr;
					size = sizeof(config_data_basic_t) / 2;
					break;
				case 2:	// sources
					source = (uint16_t *) &config_data_wx_sources_default;
					target = (uint16_t *) config_data_wx_sources_first_ptr;
					size = sizeof(config_data_wx_sources_t) / 2;
					break;
				case 3:
					source = (uint16_t *) &config_data_umb_default;
					target = (uint16_t *) config_data_umb_first_ptr;
					size = sizeof(config_data_umb_t) / 2;
					break;
				case 4:
					source = (uint16_t *) &config_data_rtu_default;
					target = (uint16_t *) config_data_rtu_first_ptr;
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
	else {
		return -2;
	}

	// set programming counter. If second region is also screwed the first one will be used as a source
	// if second is OK it will be used instead (if its programming counter has value three or more).
	*(uint16_t*)&config_data_pgm_cntr_first = 0x0002u;

#ifdef STM32F10X_MD_VL
	// resetting CRC engine
	CRC_ResetDR();

	// calculate CRC checksum of the first block
	CRC_CalcBlockCRC(config_section_first_start, CRC_32B_WORD_OFFSET - 1);

	// adding finalizing 0x00
	target_crc_value = CRC_CalcCRC((uint32_t)0x0);
#endif

#ifdef STM32L471xx
	// reset CRC engine
	LL_CRC_ResetCRCCalculationUnit(CRC);

	for (int i = 0; i < CRC_32B_WORD_OFFSET - 1; i++) {
		// feed the data into CRC engine
		LL_CRC_FeedData32(CRC, *(config_section_first_start + i));
	}

	// placeholder for CRC value itself
	CRC->DR = 0x00;

	target_crc_value = CRC->DR;
#endif

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
					target = (uint16_t *) config_data_mode_second_ptr;
					size = sizeof(config_data_mode_t) / 2;
					break;
				case 1:	// basic
					source = (uint16_t *) &config_data_basic_default;
					target = (uint16_t *) config_data_basic_second_ptr;
					size = sizeof(config_data_basic_t) / 2;
					break;
				case 2:	// sources
					source = (uint16_t *) &config_data_wx_sources_default;
					target = (uint16_t *) config_data_wx_sources_second_ptr;
					size = sizeof(config_data_wx_sources_t) / 2;
					break;
				case 3:
					source = (uint16_t *) &config_data_umb_default;
					target = (uint16_t *) config_data_umb_second_ptr;
					size = sizeof(config_data_umb_t) / 2;
					break;
				case 4:
					source = (uint16_t *) &config_data_rtu_default;
					target = (uint16_t *) config_data_rtu_second_ptr;
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
	else {
		return -2;
	}

	// set programming counter. If second region is also screwed the first one will be used as a source
	// if second is OK it will be used instead (if its programming counter has value three or more).
	*(uint16_t*)&config_data_pgm_cntr_second = 0x0002u;

#ifdef STM32F10X_MD_VL
	// resetting CRC engine
	CRC_ResetDR();

	// calculate CRC checksum of the first block
	CRC_CalcBlockCRC(config_section_first_start, CRC_32B_WORD_OFFSET - 1);

	// adding finalizing 0x00
	target_crc_value = CRC_CalcCRC((uint32_t)0x0);
#endif

#ifdef STM32L471xx
	// reset CRC engine
	LL_CRC_ResetCRCCalculationUnit(CRC);

	for (int i = 0; i < CRC_32B_WORD_OFFSET - 1; i++) {
		// feed the data into CRC engine
		LL_CRC_FeedData32(CRC, *(config_section_first_start + i));
	}

	// placeholder for CRC value itself
	CRC->DR = 0x00;

	target_crc_value = CRC->DR;
#endif

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

#ifdef STM32L471xx
	if (region == REGION_DEFAULT) {
		main_config_data_gsm = config_data_gsm_default_ptr;
	}
	else if (region == REGION_FIRST) {
		main_config_data_gsm = config_data_gsm_first_ptr;
	}
	else if (region == REGION_SECOND) {
		main_config_data_gsm = config_data_gsm_second_ptr;
	}
	else {
		;
	}
#endif

	if (region == REGION_DEFAULT) {
		main_config_data_mode = &config_data_mode_default;
		main_config_data_basic = &config_data_basic_default;
		main_config_data_wx_sources = &config_data_wx_sources_default;
		main_config_data_umb = &config_data_umb_default;
		main_config_data_rtu = &config_data_rtu_default;
	}
	else if (region == REGION_FIRST) {
		main_config_data_mode = config_data_mode_first_ptr;
		main_config_data_basic = config_data_basic_first_ptr;
		main_config_data_wx_sources = config_data_wx_sources_first_ptr;
		main_config_data_umb = config_data_umb_first_ptr;
		main_config_data_rtu = config_data_rtu_first_ptr;
	}
	else if (region == REGION_SECOND) {
		main_config_data_mode = config_data_mode_second_ptr;
		main_config_data_basic = config_data_basic_second_ptr;
		main_config_data_wx_sources = config_data_wx_sources_second_ptr;
		main_config_data_umb = config_data_umb_second_ptr;
		main_config_data_rtu = config_data_rtu_second_ptr;
	}
	else {
		;
	}

	configuration_handler_loaded = region;

}

uint32_t configuration_handler_program(uint8_t* data, uint16_t data_ln, uint8_t config_idx) {
	return -1;
}

uint32_t configuration_get_register(void) {

	uint32_t out = 0;

#ifdef STM32F10X_MD_VL
	out = BKP->DR3;
#endif

#ifdef STM32L471xx
	out = RTC->BKP3R;

#endif

	return out;
}

void configuration_set_register(uint32_t value) {
#ifdef STM32F10X_MD_VL
	BKP->DR3 = value;
#endif

#ifdef STM32L471xx
	RTC->BKP3R = value;

#endif
}

void configuration_set_bits_register(uint32_t value) {
#ifdef STM32F10X_MD_VL
	BKP->DR3 |= value;
#endif

#ifdef STM32L471xx
	RTC->BKP3R |= value;

#endif
}

void configuration_clear_bits_register(uint32_t value) {
#ifdef STM32F10X_MD_VL
	BKP->DR3 &= (0xFFFF ^ value);
#endif

#ifdef STM32L471xx
	RTC->BKP3R &= (0xFFFFFFFF ^ value);

#endif
}

int32_t configuration_kiss_parse_get_running_config(uint8_t* input_frame_from_host, uint16_t input_len) {

	// check if current configuration is set to something which make sense
	if (configuration_handler_loaded != REGION_DEFAULT &&
			configuration_handler_loaded != REGION_FIRST &&
			configuration_handler_loaded != REGION_SECOND)
	{
		return -1;
	}

	// send the KISS preamble
	srl_send_data(main_kiss_srl_ctx_ptr, kiss_config_preamble, SRL_MODE_DEFLN, 4, SRL_EXTERNAL);

	// wait for preamble to send completely
	srl_wait_for_tx_completion(main_kiss_srl_ctx_ptr);

	// check which configuration is in use now
	switch(configuration_handler_loaded) {
		case REGION_DEFAULT: {
			srl_send_data(main_kiss_srl_ctx_ptr, (const uint8_t*)config_section_default_start, SRL_MODE_DEFLN, CONFIG_SECTION_LN, SRL_EXTERNAL);
			break;
		}
		case REGION_FIRST: {
			srl_send_data(main_kiss_srl_ctx_ptr, (const uint8_t*)config_section_first_start, SRL_MODE_DEFLN, CONFIG_SECTION_LN, SRL_EXTERNAL);

			break;
		}
		case REGION_SECOND: {
			srl_send_data(main_kiss_srl_ctx_ptr, (const uint8_t*)config_section_second_start, SRL_MODE_DEFLN, CONFIG_SECTION_LN, SRL_EXTERNAL);

			break;
		}
	}

	// wait for data to send completely
	srl_wait_for_tx_completion(main_kiss_srl_ctx_ptr);

	return 0;
}

int32_t configuration_kiss_flash_config(uint8_t* input_frame_from_host, uint16_t input_len) {

	int32_t out = 0;

	FLASH_Status flash_status = FLASH_COMPLETE;	// FLASH_COMPLETE

	if (input_frame_from_host == 0x00 || input_len == 0) {
		out = -1;
	}
	else {
		// check which config is currently in use
		switch(configuration_handler_loaded) {
			case REGION_DEFAULT: {

				// erase both regions

				if (FLASH_ErasePage((uint32_t)config_section_second_start) != FLASH_COMPLETE) {
					flash_status = FLASH_ERROR_PG;
				}

				if (FLASH_ErasePage((uint32_t)config_section_second_start + 0x400) != FLASH_COMPLETE) {
					flash_status = FLASH_ERROR_PG;
				}

				if (FLASH_ErasePage((uint32_t)config_section_first_start) != FLASH_COMPLETE) {
					flash_status = FLASH_ERROR_PG;
				}

				if (FLASH_ErasePage((uint32_t)config_section_first_start + 0x400) != FLASH_COMPLETE) {
					flash_status = FLASH_ERROR_PG;
				}

				// check if operation successed
				if (flash_status != FLASH_COMPLETE) {
					out = -2;
				}


				break;
			}

			case REGION_FIRST: {
				// erase second region
				if (FLASH_ErasePage((uint32_t)config_section_second_start) != FLASH_COMPLETE) {
					flash_status = FLASH_ERROR_PG;
				}

				if (FLASH_ErasePage((uint32_t)config_section_second_start + 0x400) != FLASH_COMPLETE) {
					flash_status = FLASH_ERROR_PG;
				}

				// check if operation successed
				if (flash_status != FLASH_COMPLETE) {
					out = -2;
				}

				break;
			}

			case REGION_SECOND: {

				if (FLASH_ErasePage((uint32_t)config_section_first_start) != FLASH_COMPLETE) {
					flash_status = FLASH_ERROR_PG;
				}

				if (FLASH_ErasePage((uint32_t)config_section_first_start + 0x400) != FLASH_COMPLETE) {
					flash_status = FLASH_ERROR_PG;
				}

				// check if operation successed
				if (flash_status != FLASH_COMPLETE) {
					out = -2;
				}

				break;
			}
		}
	}

	return out;
}
