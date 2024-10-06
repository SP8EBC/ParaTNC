/*
 * debug_hardfault.h
 *
 *  Created on: Oct 5, 2024
 *      Author: mateusz
 */

#ifndef DEBUG_HARDFAULT_H_
#define DEBUG_HARDFAULT_H_

#ifdef STM32L471xx

#include <stdint.h>

#include "memory_map.h"

/// ==================================================================================================
///	X-MACROS
/// ==================================================================================================

/**
 *
 */
#define DEBUG_STACKFRAME_EXTRACT(stackpointer, source)        \
	debug_hardfault_stack_pointer = (uint32_t *)stackpointer; \
	debug_hardfault_r0 = debug_hardfault_stack_pointer[0];    \
	debug_hardfault_r1 = debug_hardfault_stack_pointer[1];    \
	debug_hardfault_r2 = debug_hardfault_stack_pointer[2];    \
	debug_hardfault_r3 = debug_hardfault_stack_pointer[3];    \
	debug_hardfault_r12 = debug_hardfault_stack_pointer[4];   \
	debug_hardfault_lr = debug_hardfault_stack_pointer[5];    \
	debug_hardfault_pc = debug_hardfault_stack_pointer[6];    \
	debug_hardfault_xpsr = debug_hardfault_stack_pointer[7];  \
	debug_hardfault_cfsr = SCB->CFSR;                         \
    debug_hardfault_mmfar = SCB->MMFAR;                       \
    debug_hardfault_bfar = SCB->BFAR;                         \
	debug_hardfault_source = source;

/**
 *
 */
#define DEBUG_STACKFRAME_STORE							\
    *((uint32_t*)(MEMORY_MAP_SRAM1_HFAULT_LOG_START) + DEBUG_HARDFAULT_OFFSET_R0) = debug_hardfault_r0;             \
    *((uint32_t*)(MEMORY_MAP_SRAM1_HFAULT_LOG_START) + DEBUG_HARDFAULT_OFFSET_R1) = debug_hardfault_r1;             \
    *((uint32_t*)(MEMORY_MAP_SRAM1_HFAULT_LOG_START) + DEBUG_HARDFAULT_OFFSET_R2) = debug_hardfault_r2;             \
    *((uint32_t*)(MEMORY_MAP_SRAM1_HFAULT_LOG_START) + DEBUG_HARDFAULT_OFFSET_R3) = debug_hardfault_r3;             \
    *((uint32_t*)(MEMORY_MAP_SRAM1_HFAULT_LOG_START) + DEBUG_HARDFAULT_OFFSET_R12) = debug_hardfault_r12;           \
    *((uint32_t*)(MEMORY_MAP_SRAM1_HFAULT_LOG_START) + DEBUG_HARDFAULT_OFFSET_LR) = debug_hardfault_lr;             \
    *((uint32_t*)(MEMORY_MAP_SRAM1_HFAULT_LOG_START) + DEBUG_HARDFAULT_OFFSET_PC) = debug_hardfault_pc;             \
    *((uint32_t*)(MEMORY_MAP_SRAM1_HFAULT_LOG_START) + DEBUG_HARDFAULT_OFFSET_XPSR) = debug_hardfault_xpsr;         \
    *((uint32_t*)(MEMORY_MAP_SRAM1_HFAULT_LOG_START) + DEBUG_HARDFAULT_OFFSET_SOURCE) = debug_hardfault_source;     \
    *((uint32_t*)(MEMORY_MAP_SRAM1_HFAULT_LOG_START) + DEBUG_HARDFAULT_OFFSET_CFSR) = debug_hardfault_cfsr;         \

/**
 *
 */
#define DEBUG_STACKFRAME_CHECKSUM                                                    \
                                                                                     \
	uint32_t sum = 0;                                                                \
	for (int i = 0; i < DEBUG_HARDFAULT_OFFSET_CHECKSUM; i++) {                      \
		sum += (*((uint32_t *)MEMORY_MAP_SRAM1_HFAULT_LOG_START + i) & 0x7FFFFFFFu); \
	}                                                                                \
	*((uint32_t *)MEMORY_MAP_SRAM1_HFAULT_LOG_START + DEBUG_HARDFAULT_OFFSET_CHECKSUM) = sum + 9u;  \

/// ==================================================================================================
///	GLOBAL MACROS
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL DEFINITIONS
/// ==================================================================================================

#define DEBUG_HARDFAULT_OFFSET_R0           (uint32_t)0u
#define DEBUG_HARDFAULT_OFFSET_R1           (uint32_t)1u
#define DEBUG_HARDFAULT_OFFSET_R2           (uint32_t)2u
#define DEBUG_HARDFAULT_OFFSET_R3           (uint32_t)3u
#define DEBUG_HARDFAULT_OFFSET_R12          (uint32_t)4u
#define DEBUG_HARDFAULT_OFFSET_LR           (uint32_t)5u
#define DEBUG_HARDFAULT_OFFSET_PC           (uint32_t)6u
#define DEBUG_HARDFAULT_OFFSET_XPSR         (uint32_t)7u
#define DEBUG_HARDFAULT_OFFSET_SOURCE	    (uint32_t)8u
#define DEBUG_HARDFAULT_OFFSET_CFSR         (uint32_t)9u
#define DEBUG_HARDFAULT_OFFSET_MMFAR        (uint32_t)10u
#define DEBUG_HARDFAULT_OFFSET_BFAR         (uint32_t)11u
#define DEBUG_HARDFAULT_OFFSET_CHECKSUM     (uint32_t)12u


#define DEBUG_HARDFAULT_SOURCE_HFLT			(uint8_t)0x10u
#define DEBUG_HARDFAULT_SOURCE_USAGEFLT		(uint8_t)0x20u
#define DEBUG_HARDFAULT_SOURCE_BUSFLT		(uint8_t)0x30u
#define DEBUG_HARDFAULT_SOURCE_MMUFLT		(uint8_t)0x40u


/// ==================================================================================================
///	GLOBAL TYPES
/// ==================================================================================================

typedef struct debug_hardfault_postmortem_stackframe_t {
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t lr;
    uint32_t pc;
    uint32_t xpsr;
    uint32_t source;
    uint32_t cfsr;

} debug_hardfault_postmortem_stackframe_t;

/// ==================================================================================================
///	GLOBAL VARIABLES
/// ==================================================================================================

extern volatile uint32_t debug_hardfault_stack_pointer_value;

extern volatile uint32_t * debug_hardfault_stack_pointer;

extern uint32_t debug_hardfault_r0;
extern uint32_t debug_hardfault_r1;
extern uint32_t debug_hardfault_r2;
extern uint32_t debug_hardfault_r3;
extern uint32_t debug_hardfault_r12;
extern uint32_t debug_hardfault_lr;
extern uint32_t debug_hardfault_pc;
extern uint32_t debug_hardfault_xpsr;
extern uint32_t debug_hardfault_cfsr;
extern uint32_t debug_hardfault_source;
extern uint32_t debug_hardfault_mmfar;
extern uint32_t debug_hardfault_bfar;

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

/**
 * Checks if SRAM1 noinit are contains any data from hard fault or any exception
 * @return zero if no postmortem snapshot are stored, non-zero value otherwise
 */
uint8_t debug_hardfault_check_have_postmortem(void);

/**
 * Delete collected postmortem data
 */
void debug_hardfault_delete_postmortem(void);

/**
 * 
 */
uint8_t debug_hardfault_get_postmortem(debug_hardfault_postmortem_stackframe_t * out);

/**
 * Assemble string representation of stored postmortem data
 * @param in postmortem information restored from noinit area
 * @param output_string pointer to character array where string will be assembled into
 * @param output_string_size size of output buffer
 * @return size of assembled string
 */
uint16_t debug_hardfault_assemble_info_string (debug_hardfault_postmortem_stackframe_t *in,
											   char *output_string, uint16_t output_string_size);

#endif

#endif /* DEBUG_HARDFAULT_H_ */
