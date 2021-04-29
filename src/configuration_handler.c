/*
 * configuration_handler.c
 *
 *  Created on: Apr 28, 2021
 *      Author: mateusz
 */

#include "configuration_handler.h"
#include "config_data.h"

#include <stm32f10x_crc.h>
#include <stm32f10x_flash.h>

const uint32_t * config_section_first_start = 0x0801E800;
const uint32_t * config_section_second_start = 0x0801F000;

#define CRC_OFFSET				0x7FC
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
	crc_current = CRC_CalcCRC(0x0);

	//crc_expected = *__config_section_second_end;

	// check if calculated CRC value match value stored in flash memory
	if (crc_expected == crc_current) {
		out |= 0x02;
	}
	return out;
}

uint32_t configuration_handler_restore_default(void) {

	uint32_t out = 0;

	// flash operation result
	FLASH_Status flash_status = 0;

	// unlock flash memory
	FLASH_Unlock();

	// erase first page
	//flash_status = FLASH_ErasePage(*__config_section_first_end);

	//if (flash_status)

	// lock the memory back
	FLASH_Lock();

}

uint32_t configuration_handler_load_configuration(void) {

}

uint32_t configuration_handler_program(uint8_t* data, uint16_t data_ln, uint8_t config_idx) {

}
