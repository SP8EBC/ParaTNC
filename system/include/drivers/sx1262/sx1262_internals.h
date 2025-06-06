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

// clang-format off
/// ==================================================================================================
///	X-MACROS
/// ==================================================================================================

#define SX1262_CHECK_RECEIVED_DATA(rx_data, validated_ln)                \
    SX1262_CHECK_RECEIVED_DATA_##validated_ln(rx_data)                   \

#define SX1262_CHECK_RECEIVED_DATA_1(rx_data)                            \
    ((rx_data[0] != 0x00) && (rx_data[0] != 0xFF))                       \

#define SX1262_CHECK_RECEIVED_DATA_2(rx_data)               \
    ((rx_data[0] != 0x00) && (rx_data[0] != 0xFF)           \
    && (rx_data[1] != 0x00) && (rx_data[1] != 0xFF))        \

#define SX1262_CHECK_RECEIVED_DATA_3(rx_data)               \
    ((rx_data[0] != 0x00) && (rx_data[0] != 0xFF)           \
    && (rx_data[1] != 0x00) && (rx_data[1] != 0xFF)         \
    && (rx_data[2] != 0x00) && (rx_data[2] != 0xFF))        \


#define SX1262_CHECK_RECEIVED_DATA_4(rx_data)               \
    ((rx_data[0] != 0x00) && (rx_data[0] != 0xFF)           \
    && (rx_data[1] != 0x00) && (rx_data[1] != 0xFF)         \
    && (rx_data[2] != 0x00) && (rx_data[2] != 0xFF)         \
    && (rx_data[3] != 0x00) && (rx_data[3] != 0xFF))        \



#define SX1262_CHECK_RECEIVED_DATA_OR(rx_data, validated_ln)                \
    SX1262_CHECK_RECEIVED_DATA_OR_##validated_ln(rx_data)                   \

#define SX1262_CHECK_RECEIVED_DATA_OR_1(rx_data)                            \
    ((rx_data[0] != 0x00) && (rx_data[0] != 0xFF))                       \

#define SX1262_CHECK_RECEIVED_DATA_OR_2(rx_data)               \
    (((rx_data[0] != 0x00) && (rx_data[0] != 0xFF))           \
    || ((rx_data[1] != 0x00) && (rx_data[1] != 0xFF)))        \

#define SX1262_CHECK_RECEIVED_DATA_OR_3(rx_data)               \
    (((rx_data[0] != 0x00) && (rx_data[0] != 0xFF))            \
    || ((rx_data[1] != 0x00) && (rx_data[1] != 0xFF))          \
    || ((rx_data[2] != 0x00) && (rx_data[2] != 0xFF)))         \

#define SX1262_CHECK_RECEIVED_DATA_OR_4(rx_data)               \
    (((rx_data[0] != 0x00) && (rx_data[0] != 0xFF))            \
    || ((rx_data[1] != 0x00) && (rx_data[1] != 0xFF))          \
    || ((rx_data[2] != 0x00) && (rx_data[2] != 0xFF))          \
    || ((rx_data[3] != 0x00) && (rx_data[3] != 0xFF)))         \


// clang-format on

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

extern volatile uint8_t sx1262_busy_flag;

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
