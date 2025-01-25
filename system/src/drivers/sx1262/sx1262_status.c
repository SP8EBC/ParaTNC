/*
 * sx1262_status.c
 *
 *  Created on: Jan 9, 2025
 *      Author: mateusz
 */

#include "drivers/sx1262/sx1262_status.h"
#include "drivers/sx1262/sx1262_internals.h"

#include "drivers/spi.h"

#include <string.h>

/// ==================================================================================================
///	LOCAL DEFINITIONS
/// ==================================================================================================

#define SX1262_STATUS_OPCODE_GET				(0xC0)
#define SX1262_STATUS_OPCODE_GET_DEVICE_ERR		(0x17)


/// ==================================================================================================
///	LOCAL DATA TYPES
/// ==================================================================================================

/// ==================================================================================================
///	LOCAL VARIABLES
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL VARIABLES
/// ==================================================================================================

/// ==================================================================================================
///	LOCAL FUNCTIONS
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

sx1262_api_return_t sx1262_status_get(
								sx1262_status_chip_mode_t * chip_mode,
								sx1262_status_last_command_t * last_command_status)
{
	sx1262_api_return_t out = SX1262_API_LIB_NOINIT;

	const uint8_t is_busy = sx1262_is_busy();

	uint8_t temp = 0;

	if (is_busy == 0) {
		memset(sx1262_transmit_spi_buffer, 0x00, SX1262_TRANSMIT_SPI_BUFFER_LN_FOR_CMD);
		sx1262_transmit_spi_buffer[0] = SX1262_STATUS_OPCODE_GET;
		sx1262_transmit_spi_buffer[1] = 0x00;

		spi_rx_tx_exchange_data(3, SPI_TX_FROM_EXTERNAL, sx1262_receive_spi_buffer, sx1262_transmit_spi_buffer, 2);

		SX1262_SPI_WAIT_UNTIL_BUSY();

		const uint8_t * ptr = spi_get_rx_data();

		if (ptr[1] != 0x00u && ptr[1] != 0xFFu) {
			temp = ptr[1] & 0x70u;
			temp >>= 4;
			*chip_mode = (sx1262_status_chip_mode_t) temp;

			temp = ptr[1] & 0x0Eu;
			temp >>= 1;
			*last_command_status = (sx1262_status_last_command_t)temp;
			out = SX1262_API_OK;
		}
		else {
			out = SX1262_API_DAMAGED_RESP;
		}
	}
	else {
		out = SX1262_API_MODEM_BUSY;
	}

	return out;
}

sx1262_api_return_t sx1262_status_get_device_errors(
		sx1262_status_chip_mode_t * chip_mode,
		sx1262_status_last_command_t * last_command_status,
		uint8_t * errors)
{
	sx1262_api_return_t out = SX1262_API_LIB_NOINIT;

	const uint8_t is_busy = sx1262_is_busy();

	uint8_t temp = 0;

	if (is_busy == 0) {
		memset(sx1262_transmit_spi_buffer, 0x00, SX1262_TRANSMIT_SPI_BUFFER_LN_FOR_CMD);
		sx1262_transmit_spi_buffer[0] = SX1262_STATUS_OPCODE_GET_DEVICE_ERR;
		sx1262_transmit_spi_buffer[1] = 0x00;
		sx1262_transmit_spi_buffer[2] = 0x00;
		sx1262_transmit_spi_buffer[3] = 0x00;

		spi_rx_tx_exchange_data(3, SPI_TX_FROM_EXTERNAL, sx1262_receive_spi_buffer, sx1262_transmit_spi_buffer, 4);

		SX1262_SPI_WAIT_UNTIL_BUSY();

		const uint16_t * ptr8 = spi_get_rx_data();

		const uint16_t * ptr16 = (const uint16_t*) spi_get_rx_data();

		volatile const uint16_t _err = *(ptr16 + 1);

		*errors = (uint8_t)(_err);

		if (ptr8[1] != 0x00u && ptr8[1] != 0xFFu) {
			temp = ptr8[1] & 0x70u;
			temp >>= 4;
			*chip_mode = (sx1262_status_chip_mode_t) temp;

			temp = ptr8[1] & 0x0Eu;
			temp >>= 1;
			*last_command_status = (sx1262_status_last_command_t)temp;
			out = SX1262_API_OK;
		}

		out = SX1262_API_OK;

	}
	else {
		out = SX1262_API_MODEM_BUSY;
	}

	return out;
}

