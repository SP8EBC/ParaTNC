/*
 * sx1262.h
 *
 *  Created on: May 19, 2025
 *      Author: mateusz
 */

#ifndef INCLUDE_DRIVERS_SX1262_SX1262_H_
#define INCLUDE_DRIVERS_SX1262_SX1262_H_

/// ==================================================================================================
///	X-MACROS
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL MACROS
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL DEFINITIONS
/// ==================================================================================================

#ifdef SX1262_SHMIDT_NOT_GATE
#define SX1262_BUSY_ACTIVE 		0U
#define SX1262_BUSY_NOTACTIVE	1U
#else
#define SX1262_BUSY_ACTIVE 		1U
#define SX1262_BUSY_NOTACTIVE	0U
#endif

/// ==================================================================================================
///	GLOBAL TYPEDEFS
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL VARIABLES
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

void sx1262_init(void);

void sx1262_busy_released_callback(void);

void sx1262_interrupt_callback(void);

void sx1262_spi_transmission_done_callback(void);

short sx1262_is_busy_io_line_active(void);

short sx1262_is_interrrupt_io_line_active(void);

short sx1262_is_busy_flag_active(void);

short sx1262_is_interrrupt_flag_active(void);

void sx1262_set_busy_flag_for_waiting(void);

#endif /* INCLUDE_DRIVERS_SX1262_SX1262_H_ */
