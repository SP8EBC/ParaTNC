/*
 * sx1262_internals.h
 *
 *  Created on: Dec 15, 2024
 *      Author: mateusz
 */

#ifndef INCLUDE_DRIVERS_SX1262_SX1262_INTERNALS_H_
#define INCLUDE_DRIVERS_SX1262_SX1262_INTERNALS_H_


#include <stdint.h>

#include "drivers/sx1262/sx1262_api_return_t.h"

/// ==================================================================================================
///	GLOBAL MACROS
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL DEFINITIONS
/// ==================================================================================================

#define SX1262_BLOCKING_IO

#define SX1262_TRANSMIT_SPI_BUFFER_LN	(128)
#define SX1262_RECEIVE_SPI_BUFFER_LN	(128)

//!< Shorter lenght passed to memset in configuration commands
#define SX1262_TRANSMIT_SPI_BUFFER_LN_FOR_CMD	(16)

#ifdef SX1262_BLOCKING_IO
#define SX1262_SPI_WAIT_UNTIL_BUSY()	spi_wait_for_comms_done()
#else
#define SX1262_SPI_WAIT_UNTIL_BUSY()
#endif

//#define SX1262_DEFAULT_VALUE_FOR_OK_RESPONSE		(0x00u)

#define SX1262_TCXO_FREQ	32000ull // in kHz
#define SX1262_RXOSC_FREQ	13000u

/// ==================================================================================================
///	GLOBAL TYPEDEFS
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL VARIABLES
/// ==================================================================================================

extern uint8_t sx1262_transmit_spi_buffer[SX1262_TRANSMIT_SPI_BUFFER_LN];

extern uint8_t sx1262_receive_spi_buffer[SX1262_RECEIVE_SPI_BUFFER_LN];

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

/**
 * Checks if SX1262 radio is currently busy or not using IO pin 14- BUSY located on the chip. Radio is
 * treated as busy also when the SPI bus used to comm with it is also busy on any RX/TX transaction
 * @return
 */
uint8_t sx1262_is_busy(void);


#endif /* INCLUDE_DRIVERS_SX1262_SX1262_INTERNALS_H_ */
