/*
 * sx1262_data_io.h
 *
 *  Created on: Jan 25, 2025
 *      Author: mateusz
 */

#ifndef INCLUDE_DRIVERS_SX1262_SX1262_DATA_IO_H_
#define INCLUDE_DRIVERS_SX1262_SX1262_DATA_IO_H_

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

/// ==================================================================================================
///	GLOBAL VARIABLES
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

/**
 * The command allows writing a single bytye in a data memory space at a specific
 * address.
 * @param start_address
 * @param data
 * @return
 */
sx1262_api_return_t sx1262_data_io_write_register_byte (uint16_t start_address, uint8_t data);

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
												   uint8_t *data);

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
												  uint8_t *data);

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
												 const uint8_t *data);

/**
 * This function allows reading (n-3) bytes of payload received starting at offset. Note that the
 * NOP must be sent after sending the offset.
 * @param start_address
 * @param data_ln
 * @param data
 * @return
 */
sx1262_api_return_t sx1262_data_io_read_buffer (uint8_t start_address, uint8_t data_ln,
												uint8_t *data);

#endif /* INCLUDE_DRIVERS_SX1262_SX1262_DATA_IO_H_ */
