/*
 * debug_hardfault.c
 *
 *  Created on: Oct 5, 2024
 *      Author: mateusz
 */

#include "debug_hardfault.h"
#include "variant.h"

#include <stdio.h>
#include <stm32l4xx.h>

/// ==================================================================================================
///	LOCAL DATA TYPES
/// ==================================================================================================

/// ==================================================================================================
///	LOCAL VARIABLES
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL VARIABLES
/// ==================================================================================================

volatile uint32_t debug_hardfault_stack_pointer_value = 0;

volatile uint32_t *debug_hardfault_stack_pointer = 0;

uint32_t debug_hardfault_r0 = 0;
uint32_t debug_hardfault_r1 = 0;
uint32_t debug_hardfault_r2 = 0;
uint32_t debug_hardfault_r3 = 0;
uint32_t debug_hardfault_r12 = 0;
uint32_t debug_hardfault_lr = 0;
uint32_t debug_hardfault_pc = 0; // also called Return Address in "Armv8-M Exception Model User
								 // Guide", Exceptions and interrupts overview -> Stack frames
uint32_t debug_hardfault_xpsr = 0;
uint32_t debug_hardfault_cfsr = 0;
uint32_t debug_hardfault_source = 0;
uint32_t debug_hardfault_mmfar = 0;
uint32_t debug_hardfault_bfar = 0;

uint32_t debug_hardfault_bfsr = 0;

/// ==================================================================================================
///	LOCAL FUNCTIONS
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

/**
 * Checks if SRAM1 noinit are contains any data from hard fault or any exception
 * @return zero if no postmortem snapshot are stored, non-zero value otherwise
 */
uint8_t debug_hardfault_check_have_postmortem (void)
{
	uint32_t sum = 0;

	void *ptr_to_checksum =
		MEMORY_MAP_SRAM1_HFAULT_LOG_START + DEBUG_HARDFAULT_OFFSET_CHECKSUM * sizeof (uint32_t);

	const uint32_t sum_from_postmortem = *((uint32_t *)ptr_to_checksum);

	for (int i = 0; i < DEBUG_HARDFAULT_OFFSET_CHECKSUM; i++) {
		sum += (*(((uint32_t *)MEMORY_MAP_SRAM1_HFAULT_LOG_START) + i) & 0x7FFFFFFFu);
	}

	sum += 9u;

	if (sum == sum_from_postmortem) {
		return 1;
	}
	else {
		return 0;
	}
}

/**
 * Delete collected postmortem data
 */
void debug_hardfault_delete_postmortem (void)
{
	for (int i = 0; i <= DEBUG_HARDFAULT_OFFSET_CHECKSUM; i++) {
		*((uint32_t *)MEMORY_MAP_SRAM1_HFAULT_LOG_START + i) = 0;
	}
}

/**
 * Checks if SRAM1 noinit are contains any data from hard fault or any exception
 * @return zero if no postmortem snapshot are stored, non-zero value otherwise
 */
uint8_t debug_hardfault_get_postmortem (debug_hardfault_postmortem_stackframe_t *out)
{

	if (variant_validate_is_within_ram (out) != 0) {
		if (debug_hardfault_check_have_postmortem () != 0) {
			out->r0 =
				*((uint32_t *)(MEMORY_MAP_SRAM1_HFAULT_LOG_START) + DEBUG_HARDFAULT_OFFSET_R0);
			out->r1 =
				*((uint32_t *)(MEMORY_MAP_SRAM1_HFAULT_LOG_START) + DEBUG_HARDFAULT_OFFSET_R1);
			out->r2 =
				*((uint32_t *)(MEMORY_MAP_SRAM1_HFAULT_LOG_START) + DEBUG_HARDFAULT_OFFSET_R2);
			out->r3 =
				*((uint32_t *)(MEMORY_MAP_SRAM1_HFAULT_LOG_START) + DEBUG_HARDFAULT_OFFSET_R3);
			out->r12 =
				*((uint32_t *)(MEMORY_MAP_SRAM1_HFAULT_LOG_START) + DEBUG_HARDFAULT_OFFSET_R12);
			out->lr =
				*((uint32_t *)(MEMORY_MAP_SRAM1_HFAULT_LOG_START) + DEBUG_HARDFAULT_OFFSET_LR);
			out->pc =
				*((uint32_t *)(MEMORY_MAP_SRAM1_HFAULT_LOG_START) + DEBUG_HARDFAULT_OFFSET_PC);
			out->xpsr =
				*((uint32_t *)(MEMORY_MAP_SRAM1_HFAULT_LOG_START) + DEBUG_HARDFAULT_OFFSET_XPSR);
			out->source =
				*((uint32_t *)(MEMORY_MAP_SRAM1_HFAULT_LOG_START) + DEBUG_HARDFAULT_OFFSET_SOURCE);
			out->cfsr =
				*((uint32_t *)(MEMORY_MAP_SRAM1_HFAULT_LOG_START) + DEBUG_HARDFAULT_OFFSET_CFSR);

			return 1;
		}
		else {
			return 0;
		}
	}
	else {
		return 0;
	}
}

/**
 * Assemble string representation of stored postmortem data
 * @param in postmortem information restored from noinit area
 * @param output_string pointer to character array where string will be assembled into
 * @param output_string_size size of output buffer
 * @return size of assembled string
 */
uint16_t debug_hardfault_assemble_info_string (debug_hardfault_postmortem_stackframe_t *in,
											   char *output_string, uint16_t output_string_size)
{

	if (variant_validate_is_within_ram (in) == 0) {
		return 0;
	}

	if (variant_validate_is_within_ram (output_string) == 0) {
		return 0;
	}

	uint16_t ouput_ln = 0;

	switch (in->source) {
	case DEBUG_HARDFAULT_SOURCE_HFLT: {
		ouput_ln = snprintf (output_string,
							 output_string_size,
							 "[!][HF][LR: 0x%lX][SP: 0x%lX][CFSR: 0x%lX][R0: 0x%lX][R1: 0x%lX][R2: "
							 "0x%lX][R3: 0x%lX]",
							 in->lr,
							 in->pc,
							 in->cfsr,
							 in->r0,
							 in->r1,
							 in->r2,
							 in->r3);
		break;
	}

	case DEBUG_HARDFAULT_SOURCE_USAGEFLT: {
		ouput_ln = snprintf (output_string,
							 output_string_size,
							 "[!][US][LR: 0x%lX][SP: 0x%lX][CFSR: 0x%lX][R0: 0x%lX][R1: 0x%lX][R2: "
							 "0x%lX][R3: 0x%lX]",
							 in->lr,
							 in->pc,
							 in->cfsr,
							 in->r0,
							 in->r1,
							 in->r2,
							 in->r3);

		break;
	}

	case DEBUG_HARDFAULT_SOURCE_BUSFLT: {
		ouput_ln = snprintf (output_string,
							 output_string_size,
							 "[!][BU][LR: 0x%lX][SP: 0x%lX][CFSR: 0x%lX][R0: 0x%lX][R1: 0x%lX][R2: "
							 "0x%lX][R3: 0x%lX]",
							 in->lr,
							 in->pc,
							 in->cfsr,
							 in->r0,
							 in->r1,
							 in->r2,
							 in->r3);

		break;
	}

	case DEBUG_HARDFAULT_SOURCE_MMUFLT: {
		ouput_ln = snprintf (output_string,
							 output_string_size,
							 "[!][MM][LR: 0x%lX][SP: 0x%lX][CFSR: 0x%lX][R0: 0x%lX][R1: 0x%lX][R2: "
							 "0x%lX][R3: 0x%lX]",
							 in->lr,
							 in->pc,
							 in->cfsr,
							 in->r0,
							 in->r1,
							 in->r2,
							 in->r3);

		break;
	}

	default: {
		break;
	}
	}

	return ouput_ln;
}

void debug_hardfault_freertos_assert_fail (void)
{
	debug_hardfault_stack_pointer_value = 0xFFFFFFFFu;
}
