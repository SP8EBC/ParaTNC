/*
 * variant.c
 *
 *  Created on: Oct 24, 2023
 *      Author: mateusz
 */

#include "variant.h"

#include <stm32l4xx.h>

#include "memory_map.h"

int variant_validate_is_within_ram (const void *address)
{

	uint32_t addr_value = (uint32_t)address;

	if (addr_value >= SRAM_BASE && addr_value < SRAM_BASE + SRAM1_SIZE_MAX) {
		return 1;
	}
	else if (addr_value >= SRAM2_BASE && addr_value < SRAM2_BASE + SRAM2_SIZE) {
		return 1;
	}
	else {
		return 0;
	}
}

int variant_validate_is_within_sram2 (const void *address)
{
	uint32_t addr_value = (uint32_t)address;

	if (addr_value >= SRAM2_BASE && addr_value < SRAM2_BASE + SRAM2_SIZE) {
		return 1;
	}
	else {
		return 0;
	}
}

int variant_validate_is_within_flash (const void *address)
{
	uint32_t addr_value = (uint32_t)address;

	if (addr_value > FLASH_BASE && addr_value < MEMORY_MAP_FLASH_END) {
		return 1;
	}
	else {
		return 0;
	}
}

int variant_validate_is_within_flash_logger_events (const void *address)
{
	uint32_t addr_value = (uint32_t)address;

	if (addr_value > MEMORY_MAP_CONFIG_END && addr_value < MEMORY_MAP_FLASH_END) {
		return 1;
	}
	else {
		return 0;
	}
}

int variant_validate_is_within_sram_logger_events (const void *address)
{
	uint32_t addr_value = (uint32_t)address;

	if (addr_value > MEMORY_MAP_SRAM1_LOG_AREA_START &&
		addr_value < MEMORY_MAP_SRAM1_LOG_AREA_END) {
		return 1;
	}
	else {
		return 0;
	}
}

int variant_validate_is_within_read_mem_by_addr (const void *address)
{
	uint32_t addr_value = (uint32_t)address;

	if (addr_value > MEMORY_MAP_CONFIG_END && addr_value < MEMORY_MAP_EVENT_LOG_END) {
		return 1;
	}
	else if (addr_value > SRAM2_BASE && addr_value < SRAM2_BASE + SRAM2_SIZE) {
		return 1;
	}
	else {
		return 0;
	}
}
