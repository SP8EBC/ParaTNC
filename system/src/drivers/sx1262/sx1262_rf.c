/*
 * sx1262_rf.c
 *
 *  Created on: Jan 21, 2025
 *      Author: mateusz
 */

#include "drivers/sx1262/sx1262_rf.h"
#include "drivers/sx1262/sx1262_internals.h"

#include "drivers/spi.h"

#include <string.h>

/// ==================================================================================================
///	LOCAL DEFINITIONS
/// ==================================================================================================

#define SX1262_RF_OPCODE_SET_RF_FREQUENCY				(0x86)
#define SX1262_RF_OPCODE_SET_PACKET_TYPE				(0x8A)
#define SX1262_RF_OPCODE_GET_PACKET_TYPE				(0x11)
#define SX1262_RF_OPCODE_SET_TX_PARAMS					(0x8E)
#define SX1262_RF_OPCODE_SET_MODULATION_PARAMS			(0x8B)
#define SX1262_RF_OPCODE_SET_PACKET_PARAMS				(0x8C)
#define SX1262_RF_OPCODE_SET_CAD_PARAMS					(0x88)
#define SX1262_RF_OPCODE_SET_BUFFER_BASE_ADDRESS		(0x8F)
#define SX1262_RF_OPCODE_SET_LORA_SYMB_NYM_TIMEOUT		(0xA0)

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
 * This command sets the base addresses in the data buffer in all modes of operations for the packet
 * handing operation in TX and RX mode. The usage and definition of those parameters are described
 * in the different packet type sections
 * @param tx_base
 * @param rx_base
 * @return success or kind of an error. For more info refer to @link{sx1262_api_return_t}
 * declaration
 */
sx1262_api_return_t sx1262_rf_buffer_base_addresses (uint8_t tx_base, uint8_t rx_base)
{
	sx1262_api_return_t out = SX1262_API_LIB_NOINIT;

	const uint8_t is_busy = sx1262_is_busy();

	if (is_busy == 0) {
		memset(sx1262_transmit_spi_buffer, 0x00, SX1262_TRANSMIT_SPI_BUFFER_LN_FOR_CMD);
		sx1262_transmit_spi_buffer[0] = SX1262_RF_OPCODE_SET_BUFFER_BASE_ADDRESS;
		sx1262_transmit_spi_buffer[1] = (uint8_t) tx_base;
		sx1262_transmit_spi_buffer[2] = (uint8_t) rx_base;

		spi_rx_tx_exchange_data(3, SPI_TX_FROM_EXTERNAL, sx1262_receive_spi_buffer, sx1262_transmit_spi_buffer, 3);

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
	}
	else {
		out = SX1262_API_MODEM_BUSY;
	}

	return out;
}

/**
 * The command is used to set the frequency of the RF frequency mode.
 *
 * Internally this function recalculates given freq in kilohertz to a 32bit value sent to the
 * radio module.
 * The LSB of this value (RF Freq) is equal to the PLL step which is:
 *									RF Freq *F XTAL
 *		RF frequency = -------------------------------------------
 *										2^25
 *	this defines the chip frequency in FS, TX and RX modes. In RX, the required IF frequency offset
 *  is automatically configured.
 *
 *
 * @param frequency in kilohertz
 * @return success or kind of an error. For more info refer to @link{sx1262_api_return_t}
 * declaration
 */
sx1262_api_return_t sx1262_rf_frequency (uint32_t frequency)
{
	sx1262_api_return_t out = SX1262_API_LIB_NOINIT;

	const uint8_t is_busy = sx1262_is_busy();

	const uint64_t pll_steps_counter = 0x2000000ull;
	//								   0x2000000

	if (frequency < 930000u && frequency > 850000u) {
		if (is_busy == 0) {
			const uint64_t freq = (frequency * pll_steps_counter) / SX1262_TCXO_FREQ;

			memset(sx1262_transmit_spi_buffer, 0x00, SX1262_TRANSMIT_SPI_BUFFER_LN_FOR_CMD);
			sx1262_transmit_spi_buffer[0] = SX1262_RF_OPCODE_SET_RF_FREQUENCY;
			sx1262_transmit_spi_buffer[4] = (uint8_t) (freq & 0x000000FFu);
			sx1262_transmit_spi_buffer[3] = (uint8_t) ((freq & 0x0000FF00u) >> 8);
			sx1262_transmit_spi_buffer[2] = (uint8_t) ((freq & 0x00FF0000u) >> 16);
			sx1262_transmit_spi_buffer[1] = (uint8_t) ((freq & 0xFF000000u) >> 24);

			spi_rx_tx_exchange_data(3, SPI_TX_FROM_EXTERNAL, sx1262_receive_spi_buffer, sx1262_transmit_spi_buffer, 5);

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
 * The command sets the radio in LoRa®, FSK or Long Range FHSS mode. The command
 * must be the first of the radio configuration sequence. The parameter for this command is
 * sx1262_rf_packet_type_t. Changing from one mode of operation to another is done using the command
 * SetPacketType(...). The parameters from the previous mode are not kept internally. The switch
 * from one frame to another must be done in STDBY_RC mode.
 * @param type
 * @return success or kind of an error. For more info refer to @link{sx1262_api_return_t}
 * declaration
 */
sx1262_api_return_t sx1262_rf_packet_type (sx1262_rf_packet_type_t type)
{
	sx1262_api_return_t out = SX1262_API_LIB_NOINIT;

	const uint8_t is_busy = sx1262_is_busy();

	if (is_busy == 0) {
		memset(sx1262_transmit_spi_buffer, 0x00, SX1262_TRANSMIT_SPI_BUFFER_LN_FOR_CMD);
		sx1262_transmit_spi_buffer[0] = SX1262_RF_OPCODE_SET_PACKET_TYPE;
		sx1262_transmit_spi_buffer[1] = (uint8_t) type;

		spi_rx_tx_exchange_data(3, SPI_TX_FROM_EXTERNAL, sx1262_receive_spi_buffer, sx1262_transmit_spi_buffer, 2);

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
	}
	else {
		out = SX1262_API_MODEM_BUSY;
	}

	return out;
}

/**
 * Returns currently selected packet type
 * @param type output pointer to an instance of @link{sx1262_rf_packet_type_t}
 * @return success or kind of an error. For more info refer to @link{sx1262_api_return_t}
 * declaration
 */
sx1262_api_return_t sx1262_rf_packet_type_get (sx1262_rf_packet_type_t *type)
{
	sx1262_api_return_t out = SX1262_API_LIB_NOINIT;

	const uint8_t is_busy = sx1262_is_busy();

	if (is_busy == 0) {
		memset(sx1262_transmit_spi_buffer, 0x00, SX1262_TRANSMIT_SPI_BUFFER_LN_FOR_CMD);
		sx1262_transmit_spi_buffer[0] = SX1262_RF_OPCODE_GET_PACKET_TYPE;
		sx1262_transmit_spi_buffer[1] = 0x00;
		sx1262_transmit_spi_buffer[1] = 0x00;

		spi_rx_tx_exchange_data(3, SPI_TX_FROM_EXTERNAL, sx1262_receive_spi_buffer, sx1262_transmit_spi_buffer, 3);

		SX1262_SPI_WAIT_UNTIL_BUSY();

#ifdef SX1262_BLOCKING_IO
		const uint8_t * ptr = spi_get_rx_data();

		if (ptr[2] == SX1262_RF_PACKET_TYPE_GFSK || ptr[2] == SX1262_RF_PACKET_TYPE_LORA || ptr[2] == SX1262_RF_PACKET_TYPE_LONG_RANGE_FHSS) {
			*type = (sx1262_rf_packet_type_t)ptr[2];
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
		out = SX1262_API_MODEM_BUSY;
	}

	return out;
}

/**
 * This command sets the TX output power by using the parameter transmit_power and the TX ramping
time by using the parameter ramp_time. This command is available for all protocols selected.
 * @param transmit_power -9 to +22 (0x16) dBm by step of 1 dB
 * @param ramp_time TX ramping time
 * @return success or kind of an error. For more info refer to @link{sx1262_api_return_t}
 * declaration
 */
sx1262_api_return_t sx1262_rf_tx_params (int8_t transmit_power, sx1262_rf_tx_ramp_time_t ramp_time)
{
	sx1262_api_return_t out = SX1262_API_LIB_NOINIT;

	const uint8_t is_busy = sx1262_is_busy();

	if (transmit_power >= -8 && transmit_power <= 22) {
		if (is_busy == 0) {
			memset(sx1262_transmit_spi_buffer, 0x00, SX1262_TRANSMIT_SPI_BUFFER_LN_FOR_CMD);
			sx1262_transmit_spi_buffer[0] = SX1262_RF_OPCODE_SET_TX_PARAMS;
			sx1262_transmit_spi_buffer[1] = (uint8_t) transmit_power;
			sx1262_transmit_spi_buffer[2] = (uint8_t) ramp_time;

			spi_rx_tx_exchange_data(3, SPI_TX_FROM_EXTERNAL, sx1262_receive_spi_buffer, sx1262_transmit_spi_buffer, 3);

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
 * The command is used to configure the modulation parameters of the radio. It is explicitly
tailored
 * to LoRa.
 * @param sf corresponds to the Spreading Factor used for the LoRa® modulation. ModParam1
 * @param bw corresponds to the bandwidth onto which the LoRa® signal is spread. ModParam2
 * @param cr coding rate ModParam3
 * @param opt corresponds to the Low Data Rate Optimization (LDRO). This parameter is usually set
when the LoRa® symbol time is equal or above 16.38 ms (typically for SF11 with BW125 and SF12 with
BW125 and BW250). ModParam4
 * @return success or kind of an error. For more info refer to @link{sx1262_api_return_t}
 * declaration
 */
sx1262_api_return_t sx1262_rf_lora_modulation_params (sx1262_rf_lora_spreading_factor_t sf,
													  sx1262_rf_lora_bandwidth_t bw,
													  sx1262_rf_lora_coding_rate_t cr,
													  sx1262_rf_lora_low_datarate_optimize_t opt)
{
	sx1262_api_return_t out = SX1262_API_LIB_NOINIT;

	const uint8_t is_busy = sx1262_is_busy();

	if (is_busy == 0) {
		memset(sx1262_transmit_spi_buffer, 0x00, SX1262_TRANSMIT_SPI_BUFFER_LN_FOR_CMD);
		sx1262_transmit_spi_buffer[0] = SX1262_RF_OPCODE_SET_MODULATION_PARAMS;
		sx1262_transmit_spi_buffer[1] = (uint8_t) sf;
		sx1262_transmit_spi_buffer[2] = (uint8_t) bw;
		sx1262_transmit_spi_buffer[3] = (uint8_t) cr;
		sx1262_transmit_spi_buffer[4] = (uint8_t) opt;

		spi_rx_tx_exchange_data(3, SPI_TX_FROM_EXTERNAL, sx1262_receive_spi_buffer, sx1262_transmit_spi_buffer, 9);

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
	}
	else {
		out = SX1262_API_MODEM_BUSY;
	}

	return out;
}

/**
 * The command is used to configure the packet parameters of the radio. It is explicitly tailored
 * to LoRa.
 * @param preamble_ln number of symbols sent as preamble. PacketParam1 & PacketParam2
 * @param payload_ln Size of the payload (in bytes) to transmit or maximum size of the
 * payload that the receiver can accept. PacketParam4
 * @param header_ln see description of @link{sx1262_rf_lora_header_t}. PacketParam3
 * @param enable_crc PacketParam5
 * @param invert_iq PacketParam6
 * @return success or kind of an error. For more info refer to @link{sx1262_api_return_t}
 * declaration
 */
sx1262_api_return_t sx1262_rf_lora_packet_params (uint16_t preamble_ln, uint8_t payload_ln,
												  sx1262_rf_lora_header_t header_ln,
												  uint8_t enable_crc, uint8_t invert_iq)
{
	sx1262_api_return_t out = SX1262_API_LIB_NOINIT;

	const uint8_t is_busy = sx1262_is_busy();

	if (is_busy == 0) {
		memset(sx1262_transmit_spi_buffer, 0x00, SX1262_TRANSMIT_SPI_BUFFER_LN_FOR_CMD);
		sx1262_transmit_spi_buffer[0] = SX1262_RF_OPCODE_SET_PACKET_PARAMS;
		sx1262_transmit_spi_buffer[1] = (uint8_t) ((preamble_ln & 0xFF00) >> 8);
		sx1262_transmit_spi_buffer[2] = (uint8_t) (preamble_ln & 0x00FF);
		sx1262_transmit_spi_buffer[3] = (uint8_t) header_ln;
		sx1262_transmit_spi_buffer[4] = (uint8_t) payload_ln;
		if (enable_crc == 0)
		{
			sx1262_transmit_spi_buffer[5] = 0;
		}
		else
		{
			sx1262_transmit_spi_buffer[5] = 1;
		}

		if (invert_iq == 0)
		{
			sx1262_transmit_spi_buffer[6] = 0;
		}
		else
		{
			sx1262_transmit_spi_buffer[6] = 1;
		}

		spi_rx_tx_exchange_data(3, SPI_TX_FROM_EXTERNAL, sx1262_receive_spi_buffer, sx1262_transmit_spi_buffer, 10);

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
	}
	else {
		out = SX1262_API_MODEM_BUSY;
	}

	return out;
}

/**
 * Defines the number of symbols on which CAD operates.
 *
 * Parameters cadDetPeak and cadDetMin define the sensitivity of the LoRa modem when trying to
 * correlate to actual LoRa preamble symbols. These two settings depend on the LoRa spreading factor
 * and Bandwidth, but also depend on the number of symbol used to validate or not the detection.
 * Choosing the right value is not easy and the values selected must be carefully tested to ensure a
 * good detection at sensitivity level, and also to limit the number of false detections.
 * Application note AN1200.48 provides guidance for the selection of these parameters.
 * @param symbol_num
 * @param det_peak
 * @param det_min
 * @param exit_mode The parameter defines the action to be done after a CAD operation. This is
 * optional.
 * @param timeout The parameter is only used when the CAD is performed with cadExitMode = CAD_RX,
 *  and indicates the time the device will stay in Rx following a successful CAD.
 * @return success or kind of an error. For more info refer to @link{sx1262_api_return_t}
 * declaration
 */
sx1262_api_return_t
sx1262_rf_lora_channel_act_detect (sx1262_rf_lora_channel_act_symbols_num_t symbol_num,
								   sx1262_rf_lora_channel_act_symbols_num_t det_peak,
								   sx1262_rf_lora_channel_act_symbols_num_t det_min,
								   sx1262_rf_lora_channel_act_exit_mode_t exit_mode,
								   uint32_t timeout)
{
	sx1262_api_return_t out = SX1262_API_LIB_NOINIT;

	const uint8_t is_busy = sx1262_is_busy();

	if (is_busy == 0) {
		memset(sx1262_transmit_spi_buffer, 0x00, SX1262_TRANSMIT_SPI_BUFFER_LN_FOR_CMD);
		sx1262_transmit_spi_buffer[0] = SX1262_RF_OPCODE_SET_CAD_PARAMS;
		sx1262_transmit_spi_buffer[1] = (uint8_t) symbol_num;
		sx1262_transmit_spi_buffer[2] = (uint8_t) det_peak;
		sx1262_transmit_spi_buffer[3] = (uint8_t) det_min;
		sx1262_transmit_spi_buffer[4] = (uint8_t) exit_mode;
		sx1262_transmit_spi_buffer[5] = (uint8_t) (timeout & 0x000000FFu);
		sx1262_transmit_spi_buffer[6] = (uint8_t) ((timeout & 0x0000FF00u) >> 8);
		sx1262_transmit_spi_buffer[7] = (uint8_t) ((timeout & 0x00FF0000u) >> 16);

		spi_rx_tx_exchange_data(3, SPI_TX_FROM_EXTERNAL, sx1262_receive_spi_buffer, sx1262_transmit_spi_buffer, 8);

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
	}
	else {
		out = SX1262_API_MODEM_BUSY;
	}

	return out;
}

/**
 * This command sets the number of symbols used by the modem to validate a successful reception.
 * In LoRa® mode, when going into Rx, the modem will lock as soon as a LoRa® symbol has been
 * detected which may lead to false detection. This phenomena is quite rare but nevertheless
 * possible. To avoid this, the command SetLoRaSymbNumTimeout can be used to define the number of
 * symbols which will be used to validate the correct reception of a packet. When the SymbNum
 * parameter is set the 0, the modem will validate the reception as soon as a LoRa® Symbol has been
 * detected.
 * When SymbNum is different from 0, the modem will wait for a total of SymbNum LoRa® symbol to
 * validate, or not, the correct detection of a LoRa® packet. If the various states of the
 * demodulator are not lock at this moment, the radio will generate the RxTimeout IRQ.
 *
 *
 * @param timeout
 * @return success or kind of an error. For more info refer to @link{sx1262_api_return_t}
 * declaration
 */
sx1262_api_return_t sx1262_rf_lora_symbol_num_timeout (uint8_t timeout)
{
	sx1262_api_return_t out = SX1262_API_LIB_NOINIT;

	const uint8_t is_busy = sx1262_is_busy();

	if (is_busy == 0) {
		memset(sx1262_transmit_spi_buffer, 0x00, SX1262_TRANSMIT_SPI_BUFFER_LN_FOR_CMD);
		sx1262_transmit_spi_buffer[0] = SX1262_RF_OPCODE_SET_TX_PARAMS;
		sx1262_transmit_spi_buffer[1] = (uint8_t) timeout;

		spi_rx_tx_exchange_data(3, SPI_TX_FROM_EXTERNAL, sx1262_receive_spi_buffer, sx1262_transmit_spi_buffer, 2);

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
	}
	else {
		out = SX1262_API_MODEM_BUSY;
	}

	return out;
}
