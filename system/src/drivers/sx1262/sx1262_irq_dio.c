/*
 * sx1262_irq_dio.c
 *
 *  Created on: Jan 25, 2025
 *      Author: mateusz
 */

#include "drivers/sx1262/sx1262_irq_dio.h"
#include "drivers/sx1262/sx1262_internals.h"

#include "drivers/spi.h"

#include <string.h>

/// ==================================================================================================
///	LOCAL DEFINITIONS
/// ==================================================================================================

#define SX1262_STATUS_OPCODE_CLEAR_IRQ_STATUS			(0x02)
#define SX1262_STATUS_OPCODE_GET_IRQ_STATUS				(0x12)
#define SX1262_STATUS_OPCODE_SET_DIO_IRQ_PARAMS			(0x08)
#define SX1262_STATUS_OPCODE_SET_DIO2_AS_RF_SWITCH		(0x9D)
#define SX1262_STATUS_OPCODE_SET_DIO3_AS_TCXO_SUPPLY	(0x97)


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

sx1262_api_return_t sx1262_irq_dio_clear_all(void)
{
	sx1262_api_return_t out = SX1262_API_LIB_NOINIT;

	const uint8_t is_busy = sx1262_is_busy();

	if (is_busy == 0) {
		memset(sx1262_transmit_spi_buffer, 0x00, SX1262_TRANSMIT_SPI_BUFFER_LN_FOR_CMD);
		memset(sx1262_receive_spi_buffer, 0x00, SX1262_TRANSMIT_SPI_BUFFER_LN_FOR_CMD);
		sx1262_transmit_spi_buffer[0] = SX1262_STATUS_OPCODE_CLEAR_IRQ_STATUS;
		sx1262_transmit_spi_buffer[1] = 0x01u;
		sx1262_transmit_spi_buffer[2] = 0xFFu;

		spi_rx_tx_exchange_data(3, SPI_TX_FROM_EXTERNAL, sx1262_receive_spi_buffer, sx1262_transmit_spi_buffer, 3);

		SX1262_SPI_WAIT_UNTIL_BUSY();

#ifdef SX1262_BLOCKING_IO
		out = SX1262_API_OK;
#else
		out = SX1262_API_OK;
#endif
	}
	else {
		out = SX1262_API_MODEM_BUSY;
	}

	return out;
}

sx1262_api_return_t sx1262_irq_dio_get_mask(uint16_t * iterrupt_mask)
{
	sx1262_api_return_t out = SX1262_API_LIB_NOINIT;

	const uint8_t is_busy = sx1262_is_busy();

	uint16_t temp = 0;

	if (is_busy == 0) {
		memset(sx1262_transmit_spi_buffer, 0x00, SX1262_TRANSMIT_SPI_BUFFER_LN_FOR_CMD);
		memset(sx1262_receive_spi_buffer, 0x00, SX1262_TRANSMIT_SPI_BUFFER_LN_FOR_CMD);
		sx1262_transmit_spi_buffer[0] = SX1262_STATUS_OPCODE_GET_IRQ_STATUS;

		const uint8_t spi_res = spi_rx_tx_exchange_data(3, SPI_TX_FROM_EXTERNAL, sx1262_receive_spi_buffer, sx1262_transmit_spi_buffer, 4);

		if (spi_res == SPI_OK) {

			SX1262_SPI_WAIT_UNTIL_BUSY();

#ifdef SX1262_BLOCKING_IO
			const uint8_t * ptr = spi_get_rx_data();

			temp = (ptr[2] << 8) | ptr[3];
			*iterrupt_mask = temp;

			// 0x14 0 0 0x56
			if (SX1262_CHECK_RECEIVED_DATA_OR(ptr, 2) == 1) {
				out = SX1262_API_OK;
			}
			else {
				out = SX1262_API_DAMAGED_RESP;
			}
#else
			out = SX1262_API_OK;
#endif
		}
		else {
			out = SX1262_API_SPI_BUSY;
		}
	}
	else {
		out = SX1262_API_MODEM_BUSY;
	}

	return out;
}

#ifdef SX1262_D_STORE_LAST_RX_BUFF
	static volatile uint8_t pin_dio1_previous_rx[5];
#endif

/**
 * Functions enables or disabled interrupt globally by setting IrqMask, it also sets DIO1Mask to
 * expose these interrupts on dio1 pin
 * @param tx_done Packet transmission completed
 * @param rx_done Packet received
 * @param timeout Rx or Tx timeout
 * @param crc_error Wrong CRC received
 * @return
 */
sx1262_api_return_t sx1262_irq_dio_enable_disable_on_pin_dio1 (uint8_t tx_done, uint8_t rx_done,
															   uint8_t timeout, uint8_t crc_error)
{
	sx1262_api_return_t out = SX1262_API_LIB_NOINIT;

	const uint8_t is_busy = sx1262_is_busy();

	uint16_t temp = 0;

	if (tx_done != 0)
	{
		temp = 1;
	}

	if (rx_done != 0)
	{
		temp |= (1 << 1);
	}

	if (timeout != 0)
	{
		temp |= (1 << 9);
	}

	if (crc_error != 0)
	{
		temp |= (1 << 6);
	}

	if (is_busy == 0) {
		memset(sx1262_transmit_spi_buffer, 0x00, SX1262_TRANSMIT_SPI_BUFFER_LN_FOR_CMD);
		sx1262_transmit_spi_buffer[0] = SX1262_STATUS_OPCODE_SET_DIO_IRQ_PARAMS;
		sx1262_transmit_spi_buffer[2] = (uint8_t) (temp & 0xFF);
		sx1262_transmit_spi_buffer[1] = (uint8_t) ((temp & 0xFF00) >> 8);
		sx1262_transmit_spi_buffer[4] = (uint8_t) (temp & 0xFF);
		sx1262_transmit_spi_buffer[3] = (uint8_t) ((temp & 0xFF00) >> 8);

		const uint8_t spi_res = spi_rx_tx_exchange_data(3, SPI_TX_FROM_EXTERNAL, sx1262_receive_spi_buffer, sx1262_transmit_spi_buffer, 5);

		if (spi_res == SPI_OK) {

			SX1262_SPI_WAIT_UNTIL_BUSY();

#ifdef SX1262_BLOCKING_IO
			const uint8_t * ptr = spi_get_rx_data();

			if (ptr[1] != 0x00u && ptr[1] != 0xFFu) {
				out = SX1262_API_OK;
			}
			else {
				out = SX1262_API_DAMAGED_RESP;
			}
#else
			out = SX1262_API_OK;
#endif
#ifdef SX1262_D_STORE_LAST_RX_BUFF
				memcpy(pin_dio1_previous_rx, ptr, 5);
#endif

		}
		else {
			out = SX1262_API_SPI_BUSY;
		}
	}
	else {
		out = SX1262_API_MODEM_BUSY;
	}

	return out;
}

#ifdef SX1262_D_STORE_LAST_RX_BUFF
	static volatile uint8_t set_dio2_previous_rx[2];
#endif

/**
 * This command is used to configure DIO2 so that it can be used to control an external RF switch.
 * When controlling the external RX switch, the pin DIO2 will toggle accordingly to the internal
 * state machine. DIO2 will be asserted high a few microseconds before the ramp-up of the PA and
 * will go bes et to zero after the ramp-down of the PA. DIO2 = 0 in SLEEP, STDBY_RX, STDBY_XOSC, FS
 * and RX modes, DIO2 = 1 in TX mode
 * @param enable if set to non zero DIO2 is selected to be used to control an RF switch
 * @return
 */
sx1262_api_return_t sx1262_irq_dio_set_dio2_as_rf_switch (uint8_t enable)
{
	sx1262_api_return_t out = SX1262_API_LIB_NOINIT;

	const uint8_t is_busy = sx1262_is_busy();

	if (is_busy == 0) {
		memset(sx1262_transmit_spi_buffer, 0x00, SX1262_TRANSMIT_SPI_BUFFER_LN_FOR_CMD);
		sx1262_transmit_spi_buffer[0] = SX1262_STATUS_OPCODE_SET_DIO2_AS_RF_SWITCH;
		if (enable != 0)
		{
			sx1262_transmit_spi_buffer[1] = 1;
		}

		const uint8_t spi_res = spi_rx_tx_exchange_data(3, SPI_TX_FROM_EXTERNAL, sx1262_receive_spi_buffer, sx1262_transmit_spi_buffer, 2);

		if (spi_res == SPI_OK) {

			SX1262_SPI_WAIT_UNTIL_BUSY();

#ifdef SX1262_BLOCKING_IO
			const uint8_t * ptr = spi_get_rx_data();

			if (ptr[1] != 0x00u && ptr[1] != 0xFFu) {
				out = SX1262_API_OK;
			}
			else {
				out = SX1262_API_DAMAGED_RESP;
			}
#else
			out = SX1262_API_OK;
#endif
#ifdef SX1262_D_STORE_LAST_RX_BUFF
				memcpy(set_dio2_previous_rx, ptr, 2);
#endif
		}
		else {
			out = SX1262_API_SPI_BUSY;
		}
	}
	else {
		out = SX1262_API_MODEM_BUSY;
	}

	return out;
}

#ifdef SX1262_D_STORE_LAST_RX_BUFF
	static volatile uint8_t set_dio3_previous_rx[4];
#endif

/**
 * This command is used to configure the chip for an external TCXO reference voltage controlled by
 * DIO3. When this command is used, the device now controls the TCXO itself through DIO3. When
 * needed (in mode STDBY_XOSC, FS, TX and RX), the internal state machine will set DIO3 to a
 * predefined output voltage (control through the parameter tcxoVoltage). Internally, the clock
 * controller will wait for the 32 MHz to appear before releasing the internal state machine.
 *
 * The XOSC_START_ERR flag will be raised at POR or at wake-up from Sleep mode in a cold-start
 * condition, when a TCXO is used. It is an expected behaviour since the chip is not yet aware of
 * being clocked by a TCXO. The user should simply clear this flag with the ClearDeviceErrors
 * command.
 *
 * Most TCXO will not be immediately ready at the desired frequency and will suffer from an initial
 * setup time where the frequency is gently drifting toward the wanted frequency. This setup time is
 * different from one TCXO to another and is also dependent on the TCXO manufacturer. To ensure this
 * setup time does not have any effect on the modulation or packets, the delay value will internally
 * gate the 32 MHz coming from the TCXO to give enough time for this initial drift to stabilize. At
 * the end of the delay period, the internal block will stop gating the clock and the radio will
 * carry on to the next step.
 *
 * Note: The user should take the delay period into account when going
 * into Tx or Rx mode from STDBY_RC mode. Indeed, the time needed to switch modes will increase with
 * the duration of delay. To avoid increasing the switching mode time, the user can first set the
 * device in STDBY_XOSC which will switch on the TCXO and wait for the delay period. Then, the user
 * can set the device into Tx or Rx mode without suffering from any delay additional to the internal
 * processing.
 *
 *
 * @param volt
 * @param delay The time needed for the 32 MHz to appear and stabilize
 * @return
 */
sx1262_api_return_t sx1262_irq_dio_set_dio3_as_tcxo_ctrl (sx1262_irq_dio_tcxo_voltage_t volt,
														  uint16_t delay)
{
	sx1262_api_return_t out = SX1262_API_LIB_NOINIT;

	const uint8_t is_busy = sx1262_is_busy();

	if (is_busy == 0) {
		memset(sx1262_transmit_spi_buffer, 0x00, SX1262_TRANSMIT_SPI_BUFFER_LN_FOR_CMD);
		sx1262_transmit_spi_buffer[0] = SX1262_STATUS_OPCODE_SET_DIO3_AS_TCXO_SUPPLY;
		sx1262_transmit_spi_buffer[1] = volt;
		sx1262_transmit_spi_buffer[2] = (uint8_t) (delay & 0x00FFu);
		sx1262_transmit_spi_buffer[3] = (uint8_t) ((delay & 0xFF00u) >> 8);

		const uint8_t spi_res = spi_rx_tx_exchange_data(3, SPI_TX_FROM_EXTERNAL, sx1262_receive_spi_buffer, sx1262_transmit_spi_buffer, 4);

		if (spi_res == SPI_OK) {

			SX1262_SPI_WAIT_UNTIL_BUSY();

	#ifdef SX1262_BLOCKING_IO
			const uint8_t * ptr = spi_get_rx_data();

			if (ptr[1] != 0x00u && ptr[1] != 0xFFu) {
				out = SX1262_API_OK;
			}
			else {
				out = SX1262_API_DAMAGED_RESP;
			}
	#else
			out = SX1262_API_OK;
	#endif
#ifdef SX1262_D_STORE_LAST_RX_BUFF
				memcpy(set_dio3_previous_rx, ptr, 4);
#endif

		}
		else {
			out = SX1262_API_SPI_BUSY;
		}
	}
	else {
		out = SX1262_API_MODEM_BUSY;
	}

	return out;
}

