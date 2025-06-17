/*
 * sx1262_data_io.c
 *
 *  Created on: Jan 26, 2025
 *      Author: mateusz
 */

#include "drivers/sx1262/sx1262_data_io.h"
#include "drivers/sx1262/sx1262_internals.h"

#include "drivers/spi.h"

#include <string.h>

/// ==================================================================================================
///	LOCAL DEFINITIONS
/// ==================================================================================================

#define SX1262_STATUS_OPCODE_WRITE_REGISTER			(0x0D)
#define SX1262_STATUS_OPCODE_READ_REGISTER			(0x1D)
#define SX1262_STATUS_OPCODE_WRITE_BUFFER			(0x0E)
#define SX1262_STATUS_OPCODE_READ_BUFFER			(0x08)


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

sx1262_api_return_t sx1262_data_io_write_register_byte (uint16_t start_address, uint8_t data)
{
	return sx1262_data_io_write_register(start_address, 1, &data);
}

/**
 * The command allows writing a block of bytes in a data memory space starting at a specific
 * address. The address is auto incremented after each data byte so that data is stored in
 * contiguous memory locations. The SPI data transfer is described in the following table.
 * @param start_address
 * @param data_ln
 * @param data
 * @return
 */
sx1262_api_return_t sx1262_data_io_write_register (uint16_t start_address, uint8_t data_ln,
												   uint8_t *data)
{
	sx1262_api_return_t out = SX1262_API_LIB_NOINIT;

	const uint8_t is_busy = sx1262_is_busy();

	// three must be added to reserve a room for opcode and an address
	if (data_ln + 3 < SX1262_TRANSMIT_SPI_BUFFER_LN) {
		if (is_busy == 0) {
			memset(sx1262_transmit_spi_buffer, 0x00, SX1262_TRANSMIT_SPI_BUFFER_LN_FOR_CMD);
			sx1262_transmit_spi_buffer[0] = SX1262_STATUS_OPCODE_WRITE_REGISTER;
			sx1262_transmit_spi_buffer[1] = (start_address & 0xFF00) >> 8;
			sx1262_transmit_spi_buffer[2] = (start_address & 0xFF);
			memcpy(sx1262_transmit_spi_buffer + 3, data, data_ln);

			spi_rx_tx_exchange_data(3, SPI_TX_FROM_EXTERNAL, sx1262_receive_spi_buffer, sx1262_transmit_spi_buffer, data_ln + 3);

			SX1262_SPI_WAIT_UNTIL_BUSY();

#ifdef SX1262_BLOCKING_IO
			const uint8_t * ptr = spi_get_rx_data();

			if (ptr[0] != 0x00  && ptr[0] != 0xFF) {
				out = SX1262_API_OK;
			}
#else
			out = SX1262_API_OK;
#endif
		}
		else {
			out = SX1262_API_MODEM_BUSY;
		}
	}
	else {
		out = SX1262_API_OUT_OF_RNG;
	}

	return out;
}

/**
 * The command allows reading a block of data starting at a given address. The address is
 * auto-incremented after each byte. The SPI data transfer is described in Table 13-25. Note that
 * the host has to send an NOP after sending the 2 bytes of address to start receiving data bytes on
 * the next NOP sent.
 * @param start_address
 * @param data_ln
 * @param data
 * @return
 */
sx1262_api_return_t sx1262_data_io_read_register (uint16_t start_address, uint8_t data_ln,
												  uint8_t *data)
{
	sx1262_api_return_t out = SX1262_API_LIB_NOINIT;

	const uint8_t is_busy = sx1262_is_busy();

	// three must be added to reserve a room for opcode and an address
	if (data_ln + 3 < SX1262_TRANSMIT_SPI_BUFFER_LN) {
		if (is_busy == 0) {
			memset(sx1262_transmit_spi_buffer, 0x00, SX1262_TRANSMIT_SPI_BUFFER_LN_FOR_CMD);
			memset(sx1262_receive_spi_buffer, 0x00, data_ln + 4);
			sx1262_transmit_spi_buffer[0] = SX1262_STATUS_OPCODE_READ_REGISTER;
			sx1262_transmit_spi_buffer[1] = (start_address & 0xFF00) >> 8;
			sx1262_transmit_spi_buffer[2] = (start_address & 0xFF);

			spi_rx_tx_exchange_data(3, SPI_TX_FROM_EXTERNAL, sx1262_receive_spi_buffer, sx1262_transmit_spi_buffer, data_ln + 4);

			SX1262_SPI_WAIT_UNTIL_BUSY();

#ifdef SX1262_BLOCKING_IO
			const uint8_t * ptr = spi_get_rx_data();

			if (ptr[0] != 0x00  && ptr[0] != 0xFF) {
				memcpy(data, ptr + 4, data_ln);
				out = SX1262_API_OK;
			}
#else
			out = SX1262_API_OK;
#endif
		}
		else {
			out = SX1262_API_MODEM_BUSY;
		}
	}
	else {
		out = SX1262_API_OUT_OF_RNG;
	}

	return out;
}

/**
 * This function is used to store data payload to be transmitted. The address is auto-incremented;
 * when it exceeds the value of 255 it is wrapped back to 0 due to the circular nature of the data
 * buffer. The address starts with an offset set as a parameter of the function. Table 13-26
 * describes the SPI data transfer.
 * @param start_address
 * @param data_ln
 * @param data
 * @return
 */
sx1262_api_return_t sx1262_data_io_write_buffer (uint8_t start_address, uint8_t data_ln,
												 const uint8_t *data)
{
	sx1262_api_return_t out = SX1262_API_LIB_NOINIT;

	const uint8_t is_busy = sx1262_is_busy();

	// three must be added to reserve a room for opcode and an address
	if (data_ln + 3 < SX1262_TRANSMIT_SPI_BUFFER_LN) {
		if (is_busy == 0) {
			memset(sx1262_transmit_spi_buffer, 0x00, SX1262_TRANSMIT_SPI_BUFFER_LN_FOR_CMD);
			sx1262_transmit_spi_buffer[0] = SX1262_STATUS_OPCODE_WRITE_BUFFER;
			sx1262_transmit_spi_buffer[1] = start_address;
			memcpy(sx1262_transmit_spi_buffer + 2, data, data_ln);

			spi_rx_tx_exchange_data(3, SPI_TX_FROM_EXTERNAL, sx1262_receive_spi_buffer, sx1262_transmit_spi_buffer, data_ln + 3);

			SX1262_SPI_WAIT_UNTIL_BUSY();

#ifdef SX1262_BLOCKING_IO
			const uint8_t * ptr = spi_get_rx_data();

			if (ptr[0] != 0x00  && ptr[0] != 0xFF) {
				out = SX1262_API_OK;
			}
#else
			out = SX1262_API_OK;
#endif
		}
		else {
			out = SX1262_API_MODEM_BUSY;
		}
	}
	else {
		out = SX1262_API_OUT_OF_RNG;
	}

	return out;
}

/**
 * This function allows reading (n-3) bytes of payload received starting at offset. Note that the
 * NOP must be sent after sending the offset.
 * @param start_address
 * @param data_ln
 * @param data
 * @return
 */
sx1262_api_return_t sx1262_data_io_read_buffer (uint8_t start_address, uint8_t data_ln,
												uint8_t *data)
{
	sx1262_api_return_t out = SX1262_API_LIB_NOINIT;

	const uint8_t is_busy = sx1262_is_busy();

	// three must be added to reserve a room for opcode and an address
	if (data_ln + 3 < SX1262_TRANSMIT_SPI_BUFFER_LN) {
		if (is_busy == 0) {
			memset(sx1262_transmit_spi_buffer, 0x00, SX1262_TRANSMIT_SPI_BUFFER_LN_FOR_CMD);
			sx1262_transmit_spi_buffer[0] = SX1262_STATUS_OPCODE_READ_BUFFER;
			sx1262_transmit_spi_buffer[1] = start_address;

			spi_rx_tx_exchange_data(3, SPI_TX_FROM_EXTERNAL, sx1262_receive_spi_buffer, sx1262_transmit_spi_buffer, data_ln + 3);

			SX1262_SPI_WAIT_UNTIL_BUSY();

#ifdef SX1262_BLOCKING_IO
			const uint8_t * ptr = spi_get_rx_data();

			if (ptr[0] != 0x00  && ptr[0] != 0xFF) {
				memcpy(data, ptr + 4, data_ln);
				out = SX1262_API_OK;
			}
#else
			out = SX1262_API_OK;
#endif
		}
		else {
			out = SX1262_API_MODEM_BUSY;
		}
	}
	else {
		out = SX1262_API_OUT_OF_RNG;
	}

	return out;
}
