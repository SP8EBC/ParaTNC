/*
 * sx1262_status.h
 *
 *  Created on: Jan 9, 2025
 *      Author: mateusz
 */

#ifndef INCLUDE_DRIVERS_SX1262_SX1262_STATUS_H_
#define INCLUDE_DRIVERS_SX1262_SX1262_STATUS_H_

#include "drivers/sx1262/sx1262_api_return_t.h"
#include <stdint.h>

/// ==================================================================================================
///	GLOBAL MACROS
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL DEFINITIONS
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL TYPEDEFS
/// ==================================================================================================

typedef enum sx1262_status_chip_mode_t {
	SX1262_CHIP_MODE_UNUSED		= 0x0,
	SX1262_CHIP_MODE_RESERVED	= 0x1,
	SX1262_CHIP_MODE_STDBY_RC	= 0x2,
	SX1262_CHIP_MODE_STDBY_XOSC = 0x3,
	SX1262_CHIP_MODE_FS			= 0x4,
	SX1262_CHIP_MODE_RX			= 0x5,
	SX1262_CHIP_MODE_TX			= 0x6,
	SX1262_CHIP_MODE_UNINIT		= 0xF
} sx1262_status_chip_mode_t;

/**
 *
 */
typedef enum sx1262_status_last_command_t {
	SX1262_LAST_COMMAND_UNUSED					= 0x0, /**< Looks like radio returns this as OK  */
	SX1262_LAST_COMMAND_RESERVED_OR_OK			= 0x1, /**< Sometimes it uses also this? Nobody knows why */
	SX1262_LAST_COMMAND_DATA_AVAILABLE_TO_HOST	= 0x2, /**< SX1262_LAST_COMMAND_DATA_AVAILABLE_TO_HOST */
	SX1262_LAST_COMMAND_TIMEOUT					= 0x3, /**< SX1262_LAST_COMMAND_TIMEOUT */
	SX1262_LAST_COMMAND_PROCESSING_ERROR		= 0x4, /**< SX1262_LAST_COMMAND_PROCESSING_ERROR */
	SX1262_LAST_COMMAND_FAIL_TO_EXEC			= 0x5, /**< SX1262_LAST_COMMAND_FAIL_TO_EXEC */
	SX1262_LAST_COMMAND_TX_DONE					= 0x6, /**< SX1262_LAST_COMMAND_TX_DONE */
	SX1262_LAST_COMMAND_UNINIT					= 0xF
}sx1262_status_last_command_t;


/// ==================================================================================================
///	GLOBAL VARIABLES
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================


sx1262_api_return_t sx1262_status_get(
								sx1262_status_chip_mode_t * chip_mode,
								sx1262_status_last_command_t * last_command_status);

sx1262_api_return_t sx1262_status_get_device_errors(
								sx1262_status_chip_mode_t * chip_mode,
								sx1262_status_last_command_t * last_command_status,
								uint8_t * errors);


#endif /* INCLUDE_DRIVERS_SX1262_SX1262_STATUS_H_ */
