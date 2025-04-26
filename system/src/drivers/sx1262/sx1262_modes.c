/*
 * sx1262.c
 *
 *  Created on: Dec 14, 2024
 *      Author: mateusz
 */

#include "drivers/sx1262/sx1262_modes.h"
#include "drivers/sx1262/sx1262_status.h"
#include "drivers/sx1262/sx1262_internals.h"

#include "drivers/spi.h"

#include <string.h>

/// ==================================================================================================
///	LOCAL DEFINITIONS
/// ==================================================================================================

#define SX1262_MODES_OPCODE_SET_SLEEP				(0x84)
#define SX1262_MODES_OPCODE_SET_CONFIG				(0x80)
#define SX1262_MODES_OPCODE_SET_FS					(0x81)
#define SX1262_MODES_OPCODE_SET_TX					(0x83)
#define SX1262_MODES_OPCODE_SET_RX					(0x82)
#define SX1262_MODES_OPCODE_STOP_TIMER_ON_PREAMBLE	(0x9F)
#define SX1262_MODES_OPCODE_SET_RX_DUTY_CYCLE		(0x94)
#define SX1262_MODES_OPCODE_SET_CAD					(0xC5)
#define SX1262_MODES_OPCODE_SET_TX_CW				(0xD1)
#define SX1262_MODES_OPCODE_SET_TX_INFINITE_PREAMB	(0xD2)
#define SX1262_MODES_OPCODE_SET_REGULATOR_MODE		(0x96)
#define SX1262_MODES_OPCODE_CALIBRATE_FUNCTION		(0x89)
#define SX1262_MODES_OPCODE_CALIBRATE_IMAGE			(0x98)
#define SX1262_MODES_OPCODE_SET_PA_CONFIG			(0x95)
#define SX1262_MODES_OPCODE_SET_RX_TX_FALLBACK		(0x93)


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

/**
 * The command SetSleep(...) is used to set the device in SLEEP mode with the lowest current consumption possible.
 * This command can be sent only while in STDBY mode (STDBY_RC or STDBY_XOSC). After the rising edge of NSS,
 * all blocks are switched OFF except the backup regulator if needed and the blocks specified in the
 * parameter sleepConfig.
 * @param cold_warm_start 0: cold start, 1: warm start (device configuration in retention)
 * @param rtc_timeout_disable_enable 0: RTC timeout disable, 1: wake-up on RTC timeout
 * @return success or kind of an error. For more info refer to @link{sx1262_api_return_t} declaration
 */
sx1262_api_return_t sx1262_modes_set_sleep(uint8_t cold_warm_start, uint8_t rtc_timeout_disable_enable) {

	/*
	 * When entering SLEEP mode, the BUSY line is asserted high and stays high for the complete duration of the SLEEP period.
	 *
	 * Once in SLEEP mode, it is possible to wake the device up from the host processor with a falling edge on the NSS line.
	 * The device can also wake up automatically based on a counter event driven by the RTC 64 kHz clock.
	 * If the RTC is used, a rising edge of NSS will still wake up the chip (the host keeps control of the chip).
	 *
	 * By default, when entering into SLEEP mode, the chip configuration is lost. However, being able to store chip configuration
	 * to lower host interaction or during RxDutyCycle mode can be implemented using the register in retention mode during
	 * SLEEP state. This is available when the SetSleep(...) command is sent with sleepConfig[2] set to 1. Once the chip leaves SLEEP
	 * mode (by NSS or RTC event), the chip will first restore the registers with the value stored into the retention register.
	 *
	 * Caution:
	 * Once the command SetSleep(...) has been sent, the device will become unresponsive for around 500 us, time needed for the
	 * configuration saving process and proper switch off of the various blocks. The user must thus make sure the device will not
	 * be receiving SPI command during these 500 us to ensure proper operations of the device.
	 */

	sx1262_api_return_t out = SX1262_API_LIB_NOINIT;

	const uint8_t is_busy = sx1262_is_busy();

	uint8_t sleep_config = 0;

	if (is_busy == 0) {
		if (rtc_timeout_disable_enable > 0) {
			sleep_config |= 1;
		}

		if (cold_warm_start > 0) {
			sleep_config |= (1 << 2);
		}

		memset(sx1262_transmit_spi_buffer, 0x00, SX1262_TRANSMIT_SPI_BUFFER_LN_FOR_CMD);
		sx1262_transmit_spi_buffer[0] = SX1262_MODES_OPCODE_SET_SLEEP;
		sx1262_transmit_spi_buffer[1] = sleep_config;

		spi_rx_tx_exchange_data(3, SPI_TX_FROM_EXTERNAL, sx1262_receive_spi_buffer, sx1262_transmit_spi_buffer, 2);

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

	return out;
}

/**
 * The command SetStandby(...) is used to set the device in a configuration mode which is at an intermediate level of
 * consumption. In this mode, the chip is placed in halt mode waiting for instructions via SPI.
 * This mode is dedicated to chip configuration using high level commands such as SetPacketType(...).
 * By default, after battery insertion or reset operation (pin NRESET goes low), the chip will enter in STDBY_RC
 * mode running with a 13 MHz RC clock.
 * @param standby_rc_xosc 0Device running on RC13M, set STDBY_RC mode, 1Device running on XTAL 32MHz, set STDBY_XOSC mode
 * @return success or kind of an error. For more info refer to @link{sx1262_api_return_t} declaration
 */
sx1262_api_return_t sx1262_modes_set_standby(uint8_t standby_rc_xosc) {

	sx1262_api_return_t out = SX1262_API_LIB_NOINIT;

	const uint8_t is_busy = sx1262_is_busy();

	if (is_busy == 0) {
		if (standby_rc_xosc <= 1) {
			memset(sx1262_transmit_spi_buffer, 0x00, SX1262_TRANSMIT_SPI_BUFFER_LN_FOR_CMD);
			sx1262_transmit_spi_buffer[0] = SX1262_MODES_OPCODE_SET_CONFIG;
			sx1262_transmit_spi_buffer[1] = standby_rc_xosc;

			spi_rx_tx_exchange_data(3, SPI_TX_FROM_EXTERNAL, sx1262_receive_spi_buffer, sx1262_transmit_spi_buffer, 2);

			SX1262_SPI_WAIT_UNTIL_BUSY();

#ifdef SX1262_BLOCKING_IO

			const uint8_t * ptr = spi_get_rx_data();

			/**
			 * Name : *ptr
	Details:170 'ª'
	Default:170 'ª'
	Decimal:-86
	Hex:0xaa
	Binary:10101010
	Octal:0252
			 */
			if (ptr[0] != 0x00  && ptr[0] != 0xFF) {
				out = SX1262_API_OK;
			}
#else
			out = SX1262_API_OK;
#endif
		}
		else {
			out = SX1262_API_OUT_OF_RNG;
		}
	}
	else {
		out = SX1262_API_MODEM_BUSY;
	}

	return out;
}

/**
 * The command SetFs() is used to set the device in the frequency synthesis mode where the PLL is locked to the carrier
 * frequency. This mode is used for test purposes of the PLL and can be considered as an intermediate mode. It is
 * automatically reached when going from STDBY_RC mode to TX mode or RX mode.
 * @return success or kind of an error. For more info refer to @link{sx1262_api_return_t} declaration
 */
sx1262_api_return_t sx1262_modes_set_fs(void) {

	/*
	 * In FS mode, the PLL will be set to the frequency programmed by the function SetRfFrequency(...) which is the same used for
	 * TX or RX operations.
	 */

	sx1262_api_return_t out = SX1262_API_LIB_NOINIT;

	const uint8_t is_busy = sx1262_is_busy();

	if (is_busy == 0) {
		memset(sx1262_transmit_spi_buffer, 0x00, SX1262_TRANSMIT_SPI_BUFFER_LN_FOR_CMD);
		sx1262_transmit_spi_buffer[0] = SX1262_MODES_OPCODE_SET_FS;

		spi_rx_tx_exchange_data(3, SPI_TX_FROM_EXTERNAL, sx1262_receive_spi_buffer, sx1262_transmit_spi_buffer, 1);

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

	return out;
}

/**
 * The command SetTx() sets the device in transmit mode.
 * @param timeout Timeout duration = timeout * 15.625 μs. the device remains in TX mode, it returns automatically to STBY_RC mode on timer
 * end-of-count or when a packet has been transmitted. The maximum timeout is then 262 s.
 */
sx1262_api_return_t sx1262_modes_set_tx(uint32_t timeout) {

	/*
	 * •Starting from STDBY_RC mode, the oscillator is switched ON followed by the PLL, then the PA is switched ON and the
	 * PA regulator starts ramping according to the ramping time defined by the command SetTxParams(...)
	 *
	 * •When the ramping is completed the packet handler starts the packet transmission
	 *
	 * •When the last bit of the packet has been sent, an IRQ TX_DONE is generated, the PA regulator is ramped down, the PA
	 * is switched OFF and the chip goes back to STDBY_RC mode
	 *
	 * •A TIMEOUT IRQ is triggered if the TX_DONE IRQ is not generated within the given timeout period
	 *
	 * •The chip goes back to STBY_RC mode after a TIMEOUT IRQ or a TX_DONE IRQ.
	 *
	 *
	 * The timeout duration can be computed with the formula:
	 * Timeout duration = timeout * 15.625 μs
	 * Timeout is a 23-bit parameter defining the number of step used during timeout as defined in the following table.
	 *
	 * The value given for the timeout should be calculated for a given packet size, given modulation and packet parameters.
	 * The timeout behaves as a security in case of conflicting commands from the host controller.
	 * The timeout in Tx mode can be used as a security to ensure that if for any reason the Tx is aborted or does not succeed
	 * (ie. the TxDone IRQ never is never triggered), the TxTimeout will prevent the system from waiting for an unknown amount of
	 * time. Using the timeout while in Tx mode remove the need to use resources from the host MCU to perform the same task.
	 *
	 */

	sx1262_api_return_t out = SX1262_API_LIB_NOINIT;

	uint8_t temp = 0;

	volatile sx1262_status_chip_mode_t chip_mode;

	volatile sx1262_status_last_command_t last_command_status;

	const uint8_t is_busy = sx1262_is_busy();

	if (is_busy == 0) {
		memset(sx1262_transmit_spi_buffer, 0x00, SX1262_TRANSMIT_SPI_BUFFER_LN_FOR_CMD);
		sx1262_transmit_spi_buffer[0] = SX1262_MODES_OPCODE_SET_TX;
		sx1262_transmit_spi_buffer[1] = (timeout & 0x000000FF);
		sx1262_transmit_spi_buffer[2] = (timeout & 0x0000FF00) >> 8;
		sx1262_transmit_spi_buffer[3] = (timeout & 0x00FF0000) >> 16;
		//sx1262_transmit_spi_buffer[4] = (timeout & 0xFF000000) >> 24;

		spi_rx_tx_exchange_data(3, SPI_TX_FROM_EXTERNAL, sx1262_receive_spi_buffer, sx1262_transmit_spi_buffer, 4);

		SX1262_SPI_WAIT_UNTIL_BUSY();

#ifdef SX1262_BLOCKING_IO
		const uint8_t * ptr = spi_get_rx_data();

		if (ptr[0] != 0x00  && ptr[0] != 0xFF) {

			temp = ptr[0] & 0x70u;
			temp >>= 4;
			chip_mode = (sx1262_status_chip_mode_t) temp;

			temp = ptr[0] & 0x0Eu;
			temp >>= 1;
			last_command_status = (sx1262_status_last_command_t)temp;

			if (last_command_status == SX1262_LAST_COMMAND_RESERVED_OR_OK)
			{
				out = SX1262_API_OK;
			}
			else
			{
				out = SX1262_API_DAMAGED_RESP;
			}

		}
#else
		out = SX1262_API_OK;
#endif
	}
	else {
		out = SX1262_API_MODEM_BUSY;
	}

	return out;
}
/**
 * The command SetRx() sets the device in receiver mode.
 * @param timeout 0xFFFFFF: Rx Continuous mode, The device remains in RX mode until the host sends a command to change the
 * operation mode. Others: Timeout active. The device remains in RX mode, it returns automatically to STBY_RC mode on timer
 * end-of-count or when a packet has been received.
 * @return success or kind of an error. For more info refer to @link{sx1262_api_return_t} declaration
 */
sx1262_api_return_t sx1262_modes_set_rx(uint32_t timeout) {

	/*
	 * This command sets the chip in RX mode, waiting for the reception of one or several packets. The receiver mode operates
	 * with a timeout to provide maximum flexibility to end users.
	 *
	 * When the timeout is active (0x000000 < timeout < 0xFFFFFF), the radio will stop the reception at the end of the timeout
	 * period unless a preamble and Sync Word (in GFSK) or Header (in LoRa®) has been detected. This is to ensure that a valid
	 * packet will not be dropped in the middle of the reception due to the pre-defined timeout. By default, the timer will be
	 * stopped only if the Sync Word or header has been detected. However, it is also possible to stop the timer upon preamble
	 * detection by using the command StopTimerOnPreamble(...).
	 *
	 *
	 */

	sx1262_api_return_t out = SX1262_API_LIB_NOINIT;

	const uint8_t is_busy = sx1262_is_busy();

	if (is_busy == 0) {
		memset(sx1262_transmit_spi_buffer, 0x00, SX1262_TRANSMIT_SPI_BUFFER_LN_FOR_CMD);
		sx1262_transmit_spi_buffer[0] = SX1262_MODES_OPCODE_SET_RX;
		sx1262_transmit_spi_buffer[1] = (timeout & 0x000000FF);
		sx1262_transmit_spi_buffer[2] = (timeout & 0x0000FF00) >> 8;
		sx1262_transmit_spi_buffer[3] = (timeout & 0x00FF0000) >> 16;
		//sx1262_transmit_spi_buffer[4] = (timeout & 0xFF000000) >> 24;

		spi_rx_tx_exchange_data(3, SPI_TX_FROM_EXTERNAL, sx1262_receive_spi_buffer, sx1262_transmit_spi_buffer, 4);

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

	return out;
}

/**
 * The command StopTimerOnPreamble(...) allows the user to select if the timer is stopped upon preamble detection of Sync
 * Word / header detection.
 * @param disable_enable 0x00: Timer is stopped upon Sync Word or Header detection, 0x01: Timer is stopped upon preamble detection
 * @return success or kind of an error. For more info refer to @link{sx1262_api_return_t} declaration
 */
sx1262_api_return_t sx1262_modes_stop_timer_on_preamble(uint8_t disable_enable) {

	/*
	 * By default, the timer is stopped only when the Sync Word (in GFSK) or Header (in LoRa®) has been detected. When the
	 * function StopTimerOnPreamble(...) is used with the value enable at 0x01, then the timer will be stopped upon preamble
	 * detection and the device will stay in RX mode until a packet is received. It is important to notice that stopping the timer
	 * upon preamble may cause the device to stay in Rx for an unexpected long period of time in case of false detection.
	 */

	sx1262_api_return_t out = SX1262_API_LIB_NOINIT;

	const uint8_t is_busy = sx1262_is_busy();

	if (is_busy == 0) {
		memset(sx1262_transmit_spi_buffer, 0x00, SX1262_TRANSMIT_SPI_BUFFER_LN_FOR_CMD);
		sx1262_transmit_spi_buffer[0] = SX1262_MODES_OPCODE_STOP_TIMER_ON_PREAMBLE;
		if (disable_enable > 0) {
			sx1262_transmit_spi_buffer[1] = 1;
		}
		else {
			sx1262_transmit_spi_buffer[1] = 0;
		}

		spi_rx_tx_exchange_data(3, SPI_TX_FROM_EXTERNAL, sx1262_receive_spi_buffer, sx1262_transmit_spi_buffer, 2);

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

	return out;
}

/**
 * This command sets the chip in sniff mode so that it regularly looks for new packets. This is the listen mode.
 * @param rx_period The RX mode duration is defined by Rx Duration = rx_period * 15.625 μs
 * @param sleep_period The SLEEP mode duration is defined by: Sleep Duration = sleep_period * 15.625 μs
 * @return success or kind of an error. For more info refer to @link{sx1262_api_return_t} declaration
 */
sx1262_api_return_t sx1262_modes_set_rx_duty_cycle(uint32_t rx_period, uint32_t sleep_period) {

	/*
	 * When this command is sent in STDBY_RC mode, the context (device configuration) is saved and the chip enters in a loop
	 * defined by the following steps:
	 *
	 * •The chip enters RX and listens for a packet for a period of time defined by rxPeriod
	 * •The chip is looking for a preamble in either LoRa® or FSK
	 * •Upon preamble detection, the timeout is stopped and restarted with the value 2 * rx_period + sleep_period
	 * •If no packet is received during the RX window (defined by rx_period), the chip goes into SLEEP mode with context saved
	 * for a period of time defined by sleepPeriod
	 * •At the end of the SLEEP window, the chip automatically restarts the process of restoring context and enters the RX
	 * mode, and so on. At any time, the host can stop the procedure.
	 *
	 * The loop is terminated if either:
	 * •A packet is detected during the RX window, at which moment the chip interrupts the host via the RX_DONE flag and
	 * returns to STBY_RC mode
	 * •The host issues a SetStandby(...) command during the RX window (during SLEEP mode, the device is unable to receive
	 * commands straight away and must first be waken up by a falling edge of NSS).
	 *
	 * The SLEEP mode duration is defined by:
	 * Sleep Duration = sleep_period * 15.625 μs
	 * The RX mode duration is defined by
	 * Rx Duration = rx_period * 15.625 μs
	 *
	 * It can be observed that the radio will spend around 1 ms to save the context and go into SLEEP mode
	 * and then re-initialize the radio, lock the PLL and go into RX. The delay is not accurate and may vary
	 * depending on the time needed for the XTAL to start, the PLL to lock, etc.
	 *
	 * Upon preamble detection, the radio is set to look for a Sync Word (in GFSK) or a header (in LoRa®) and the timer
	 * is restarted with a new value which is computed as 2 * rxPeriod + sleepPeriod. This is to ensure that the radio
	 * does not spend an indefinite amount of time waiting in Rx for a packet which may never arrive (false preamble detection).
	 *
	 */

	sx1262_api_return_t out = SX1262_API_LIB_NOINIT;

	const uint8_t is_busy = sx1262_is_busy();

	if (is_busy == 0) {
		memset(sx1262_transmit_spi_buffer, 0x00, SX1262_TRANSMIT_SPI_BUFFER_LN_FOR_CMD);
		sx1262_transmit_spi_buffer[0] = SX1262_MODES_OPCODE_SET_RX_DUTY_CYCLE;
		sx1262_transmit_spi_buffer[1] = (rx_period & 0x000000FF);
		sx1262_transmit_spi_buffer[2] = (rx_period & 0x0000FF00) >> 8;
		sx1262_transmit_spi_buffer[3] = (rx_period & 0x00FF0000) >> 16;
		sx1262_transmit_spi_buffer[4] = (sleep_period & 0x000000FF);
		sx1262_transmit_spi_buffer[5] = (sleep_period & 0x0000FF00) >> 8;
		sx1262_transmit_spi_buffer[6] = (sleep_period & 0x00FF0000) >> 16;

		spi_rx_tx_exchange_data(3, SPI_TX_FROM_EXTERNAL, sx1262_receive_spi_buffer, sx1262_transmit_spi_buffer, 7);

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

	return out;
}

/**
 * The command SetCAD() can be used only in LoRa® packet type. The Channel Activity Detection is a LoRa® specific mode of
 * operation where the device searches for the presence of a LoRa® preamble signal. After the search has completed, the
 * device returns in STDBY_RC mode. The length of the search is configured via the command SetCadParams(...).
 * @return success or kind of an error. For more info refer to @link{sx1262_api_return_t} declaration
 */
sx1262_api_return_t sx1262_modes_set_cad(void) {

	sx1262_api_return_t out = SX1262_API_LIB_NOINIT;

	const uint8_t is_busy = sx1262_is_busy();

	if (is_busy == 0) {
		memset(sx1262_transmit_spi_buffer, 0x00, SX1262_TRANSMIT_SPI_BUFFER_LN_FOR_CMD);
		sx1262_transmit_spi_buffer[0] = SX1262_MODES_OPCODE_SET_CAD;

		spi_rx_tx_exchange_data(3, SPI_TX_FROM_EXTERNAL, sx1262_receive_spi_buffer, sx1262_transmit_spi_buffer, 1);

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

	return out;
}

/**
 *
 * SetTxContinuousWave() is a test command available for all packet types to generate a continuous wave (RF tone) at selected
 * frequency and output power. The device stays in TX continuous wave until the host sends a mode configuration command.
 * @return success or kind of an error. For more info refer to @link{sx1262_api_return_t} declaration
 */
sx1262_api_return_t sx1262_modes_set_tx_cw(void) {

	sx1262_api_return_t out = SX1262_API_LIB_NOINIT;

	const uint8_t is_busy = sx1262_is_busy();

	if (is_busy == 0) {
		memset(sx1262_transmit_spi_buffer, 0x00, SX1262_TRANSMIT_SPI_BUFFER_LN_FOR_CMD);
		sx1262_transmit_spi_buffer[0] = SX1262_MODES_OPCODE_SET_TX_CW;

		spi_rx_tx_exchange_data(3, SPI_TX_FROM_EXTERNAL, sx1262_receive_spi_buffer, sx1262_transmit_spi_buffer, 1);

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

	return out;
}

/**
 * SetTxInfinitePreamble() is a test command to generate an infinite sequence of alternating zeros and ones in FSK modulation.
 * In LoRa®, the radio is only able to constantly modulate LoRa® preamble symbols. The device will remain in TX infinite
 * preamble until the host sends a mode configuration command.
 * @return success or kind of an error. For more info refer to @link{sx1262_api_return_t} declaration
 */
sx1262_api_return_t sx1262_modes_set_tx_infinite_preamble(void) {

	/*
	 * However, when using this function, it is impossible to define any data sent by the device. In LoRa® mode, the radio is only
	 * able to constantly modulate LoRa preamble symbols and, in FSK mode, the radio is only able to generate FSK preamble
	 * (0x55). Nevertheless, the end user will be able to easily monitor the spectral impact of its modulation parameters.
	 *
	 *
	 */

	sx1262_api_return_t out = SX1262_API_LIB_NOINIT;

	const uint8_t is_busy = sx1262_is_busy();

	if (is_busy == 0) {
		memset(sx1262_transmit_spi_buffer, 0x00, SX1262_TRANSMIT_SPI_BUFFER_LN_FOR_CMD);
		sx1262_transmit_spi_buffer[0] = SX1262_MODES_OPCODE_SET_TX_INFINITE_PREAMB;

		spi_rx_tx_exchange_data(3, SPI_TX_FROM_EXTERNAL, sx1262_receive_spi_buffer, sx1262_transmit_spi_buffer, 1);

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

	return out;
}

/**
 * By default only the LDO is used. This is useful in low cost applications where the cost of the extra self needed for a DC-DC
 * converter is prohibitive. Using only a linear regulator implies that the RX or TX current is almost doubled. This function
 * allows to specify if DC-DC or LDO is used for power regulation. The regulation mode is defined by parameter ldo_dcdcldo.
 * @param ldo_dcdcldo 0: Only LDO used for all modes, 1: DC_DC+LDO used for STBY_XOSC,FS, RX and TX modes
 * @return success or kind of an error. For more info refer to @link{sx1262_api_return_t} declaration
 */
sx1262_api_return_t sx1262_modes_set_regulator_mode(uint8_t ldo_dcdcldo) {

	sx1262_api_return_t out = SX1262_API_LIB_NOINIT;

	const uint8_t is_busy = sx1262_is_busy();

	if (is_busy == 0) {
		memset(sx1262_transmit_spi_buffer, 0x00, SX1262_TRANSMIT_SPI_BUFFER_LN_FOR_CMD);
		sx1262_transmit_spi_buffer[0] = SX1262_MODES_OPCODE_SET_REGULATOR_MODE;
		if (ldo_dcdcldo > 0) {
			sx1262_transmit_spi_buffer[1] = 1u;
		}
		else {
			sx1262_transmit_spi_buffer[1] = 0u;
		}

		spi_rx_tx_exchange_data(3, SPI_TX_FROM_EXTERNAL, sx1262_receive_spi_buffer, sx1262_transmit_spi_buffer, 2);

		SX1262_SPI_WAIT_UNTIL_BUSY();

#ifdef SX1262_BLOCKING_IO
		const uint8_t * ptr = spi_get_rx_data();

		//if (ptr[0] != 0x00  && ptr[0] != 0xFF) {
			out = SX1262_API_OK;
		//}
#else
		out = SX1262_API_OK;
#endif
	}
	else {
		out = SX1262_API_MODEM_BUSY;
	}

	return out;
}

/**
 * At power up the radio performs calibration of RC64k, RC13M, PLL and ADC. It is however possible to launch a calibration of
 * one or several blocks at any time starting in STDBY_RC mode. The calibrate function starts the calibration of a block defined
 * by calibParam.
 * @param rc64k RC64k calibration disabled / enabled
 * @param rc13m RC13M calibration disabled / enabled
 * @param pll PLL calibration
 * @param adc_ps ADC pulse calibration
 * @param adc_n ADC bulk N
 * @param adc_p ADC bulk P
 * @param image Image calibration
 * @return success or kind of an error. For more info refer to @link{sx1262_api_return_t} declaration
 */
sx1262_api_return_t sx1262_modes_set_calibrate_function(uint8_t rc64k, uint8_t rc13m, uint8_t pll, uint8_t adc_ps, uint8_t adc_n, uint8_t adc_p, uint8_t image) {

	sx1262_api_return_t out = SX1262_API_LIB_NOINIT;

	uint8_t data = (image << 6) | (adc_p << 5) | (adc_n << 4) | (adc_ps << 3) | (pll << 2) | (rc13m << 1) | rc64k;

	const uint8_t is_busy = sx1262_is_busy();

	if (is_busy == 0) {
		memset(sx1262_transmit_spi_buffer, 0x00, SX1262_TRANSMIT_SPI_BUFFER_LN_FOR_CMD);
		sx1262_transmit_spi_buffer[0] = SX1262_MODES_OPCODE_CALIBRATE_FUNCTION;
		sx1262_transmit_spi_buffer[1] = data;

		spi_rx_tx_exchange_data(3, SPI_TX_FROM_EXTERNAL, sx1262_receive_spi_buffer, sx1262_transmit_spi_buffer, 2);

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

	return out;
}

/**
 * The function CalibrateImage(...) allows the user to calibrate the image rejection of the device for the device operating
 * frequency band.
 * @param freq_lo
 * @param freq_hi
 * @return success or kind of an error. For more info refer to @link{sx1262_api_return_t} declaration
 */
sx1262_api_return_t sx1262_modes_set_calibrate_image(uint16_t freq_lo, uint16_t freq_hi) {

	sx1262_api_return_t out = SX1262_API_LIB_NOINIT;

	uint8_t freq1 = 0, freq2 = 0;

	if ((freq_lo == 430u) && (freq_hi == 440u)) {
		freq1 = 0x6B;
		freq2 = 0x6F;
	}
	else if ((freq_lo == 470u) && (freq_hi == 510u)) {
		freq1 = 0x75;
		freq2 = 0x81;
	}
	else if ((freq_lo == 779u) && (freq_hi == 787u)) {
		freq1 = 0xC1;
		freq2 = 0xC5;
	}
	else if ((freq_lo == 863u) && (freq_hi == 870u)) {
		freq1 = 0xD7;
		freq2 = 0xDB;
	}
	else if ((freq_lo == 902u) && (freq_hi == 928u)) {
		freq1 = 0xE1;
		freq2 = 0xE9;
	}
	else {
		out = SX1262_API_OUT_OF_RNG;
	}

	const uint8_t is_busy = sx1262_is_busy();

	if ((is_busy == 0) && (out != SX1262_API_OUT_OF_RNG)) {
		memset(sx1262_transmit_spi_buffer, 0x00, SX1262_TRANSMIT_SPI_BUFFER_LN_FOR_CMD);
		sx1262_transmit_spi_buffer[0] = SX1262_MODES_OPCODE_CALIBRATE_IMAGE;
		sx1262_transmit_spi_buffer[1] = freq1;
		sx1262_transmit_spi_buffer[2] = freq2;

		spi_rx_tx_exchange_data(3, SPI_TX_FROM_EXTERNAL, sx1262_receive_spi_buffer, sx1262_transmit_spi_buffer, 3);

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

	return out;
}

/**
 * SetPaConfig is the command to set transmit power. Internally it select values for 'paDutyCycle' and 'hpMax'
 * to match a value in dBm provided by a caller
 * @param tx_power_dbm
 * @return success or kind of an error. For more info refer to @link{sx1262_api_return_t} declaration
 */
sx1262_api_return_t sx1262_modes_set_pa_config(uint8_t tx_power_dbm) {

	sx1262_api_return_t out = SX1262_API_LIB_NOINIT;

	const uint8_t is_busy = sx1262_is_busy();

	/*
	 * Caution!
		The following restrictions must be observed to avoid voltage overstress on the PA, exceeding the maximum ratings
		may cause irreversible damage to the device:
		•For SX1262, paDutyCycle should not be higher than 0x04.
	 *
	 *
	 *
	 *
	 */

	uint8_t pa_duty_cycle = 4u;

	uint8_t hp_max = 7u;

	(void)tx_power_dbm;

	if (is_busy == 0) {
		memset(sx1262_transmit_spi_buffer, 0x00, SX1262_TRANSMIT_SPI_BUFFER_LN);
		sx1262_transmit_spi_buffer[0] = SX1262_MODES_OPCODE_SET_PA_CONFIG;
		sx1262_transmit_spi_buffer[1] = pa_duty_cycle;
		sx1262_transmit_spi_buffer[2] = hp_max;
		sx1262_transmit_spi_buffer[3] = 0x00;
		sx1262_transmit_spi_buffer[4] = 0x01;

		spi_rx_tx_exchange_data(3, SPI_TX_FROM_EXTERNAL, sx1262_receive_spi_buffer, sx1262_transmit_spi_buffer, 5);

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

	return out;
}

/**
 * The command SetRxTxFallbackMode defines into which mode the chip goes after a successful transmission or after a packet
 * reception.
 * @param fallback_mode 0x40 The radio goes into FS mode after Tx or Rx, 0x30 The radio goes into STDBY_XOSC mode after Tx or Rx,
 * 0x20 The radio goes into STDBY_RC mode after Tx or Rx
 * @return success or kind of an error. For more info refer to @link{sx1262_api_return_t} declaration
 */
sx1262_api_return_t sx1262_modes_set_rxtx_fallback_mode(uint8_t fallback_mode) {

	sx1262_api_return_t out = SX1262_API_LIB_NOINIT;

	const uint8_t is_busy = sx1262_is_busy();

	if ((fallback_mode == 0x40) || (fallback_mode == 0x30) || (fallback_mode == 0x20)) {
		if (is_busy == 0) {
			memset(sx1262_transmit_spi_buffer, 0x00, SX1262_TRANSMIT_SPI_BUFFER_LN_FOR_CMD);
			sx1262_transmit_spi_buffer[0] = SX1262_MODES_OPCODE_SET_RX_TX_FALLBACK;
			sx1262_transmit_spi_buffer[1] = fallback_mode;

			spi_rx_tx_exchange_data(3, SPI_TX_FROM_EXTERNAL, sx1262_receive_spi_buffer, sx1262_transmit_spi_buffer, 2);

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

	}

	return out;
}
