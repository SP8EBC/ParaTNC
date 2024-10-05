/*
 * debug_hardfault.h
 *
 *  Created on: Oct 5, 2024
 *      Author: mateusz
 */

#ifndef DEBUG_HARDFAULT_H_
#define DEBUG_HARDFAULT_H_

#include <stdint.h>

#include "memory_map.h"

/// ==================================================================================================
///	X-MACROS
/// ==================================================================================================

#define DEBUG_STACKFRAME_STORE(stackpointer, source)                                                      \
    *((uint32_t*)(MEMORY_MAP_SRAM1_HFAULT_LOG_START) + DEBUG_HARDFAULT_OFFSET_R0) = *((volatile uint32_t*)stackpointer + (uint32_t)DEBUG_HARDFAULT_OFFSET_R0);   \
    *((uint32_t*)(MEMORY_MAP_SRAM1_HFAULT_LOG_START) + DEBUG_HARDFAULT_OFFSET_R1) = *((volatile uint32_t*)stackpointer + (uint32_t)DEBUG_HARDFAULT_OFFSET_R1);   \
    *((uint32_t*)(MEMORY_MAP_SRAM1_HFAULT_LOG_START) + DEBUG_HARDFAULT_OFFSET_R2) = *((volatile uint32_t*)stackpointer + (uint32_t)DEBUG_HARDFAULT_OFFSET_R2);   \
    *((uint32_t*)(MEMORY_MAP_SRAM1_HFAULT_LOG_START) + DEBUG_HARDFAULT_OFFSET_R3) = *((volatile uint32_t*)stackpointer + (uint32_t)DEBUG_HARDFAULT_OFFSET_R3);   \
    *((uint32_t*)(MEMORY_MAP_SRAM1_HFAULT_LOG_START) + DEBUG_HARDFAULT_OFFSET_R12) = *((volatile uint32_t*)stackpointer + (uint32_t)DEBUG_HARDFAULT_OFFSET_R12);   \
    *((uint32_t*)(MEMORY_MAP_SRAM1_HFAULT_LOG_START) + DEBUG_HARDFAULT_OFFSET_LR) = *((volatile uint32_t*)stackpointer + (uint32_t)DEBUG_HARDFAULT_OFFSET_LR);   \
    *((uint32_t*)(MEMORY_MAP_SRAM1_HFAULT_LOG_START) + DEBUG_HARDFAULT_OFFSET_PC) = *((volatile uint32_t*)stackpointer + (uint32_t)DEBUG_HARDFAULT_OFFSET_PC);   \
    *((uint32_t*)(MEMORY_MAP_SRAM1_HFAULT_LOG_START) + DEBUG_HARDFAULT_OFFSET_XPSR) = *((volatile uint32_t*)stackpointer + (uint32_t)DEBUG_HARDFAULT_OFFSET_XPSR);   \
    *((uint32_t*)(MEMORY_MAP_SRAM1_HFAULT_LOG_START) + DEBUG_HARDFAULT_OFFSET_SOURCE) = ((uint32_t)source & 0xFF);   \
    *((uint32_t*)(MEMORY_MAP_SRAM1_HFAULT_LOG_START) + DEBUG_HARDFAULT_OFFSET_CFSR) = SCB->CFSR;   \

#define DEBUG_STACKFRAME_CHECKSUM						                                            \
																									\
    uint32_t sum = 0;                                                                               \
    for (int i = 0; i < DEBUG_HARDFAULT_OFFSET_CHECKSUM; i++) {                                     \
        sum += *((uint32_t*)MEMORY_MAP_SRAM1_HFAULT_LOG_START + i);                                 \
    }                                                                                               \
    *((uint32_t*)MEMORY_MAP_SRAM1_HFAULT_LOG_START + DEBUG_HARDFAULT_OFFSET_CHECKSUM) = sum + 9u;   \

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
#define DEBUG_HARDFAULT_OFFSET_CHECKSUM     (uint32_t)10u


#define DEBUG_HARDFAULT_SOURCE_HFLT			(uint8_t)0x10u
#define DEBUG_HARDFAULT_SOURCE_USAGEFLT		(uint8_t)0x20u
#define DEBUG_HARDFAULT_SOURCE_BUSFLT		(uint8_t)0x30u
#define DEBUG_HARDFAULT_SOURCE_MMUFLT		(uint8_t)0x40u


/// ==================================================================================================
///	GLOBAL TYPES
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL VARIABLES
/// ==================================================================================================

extern volatile uint32_t debug_hardfault_stack_pointer_value;

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================



#endif /* DEBUG_HARDFAULT_H_ */
