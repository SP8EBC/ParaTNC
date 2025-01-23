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
 * @return
 */
sx1262_api_return_t sx1262_rf_buffer_base_addresses (uint8_t tx_base, uint8_t rx_base)
{

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
 * @return
 */
sx1262_api_return_t sx1262_rf_frequency (uint32_t frequency)
{

}

/**
 * The command sets the radio in LoRa®, FSK or Long Range FHSS mode. The command
 * must be the first of the radio configuration sequence. The parameter for this command is
 * sx1262_rf_packet_type_t. Changing from one mode of operation to another is done using the command
 * SetPacketType(...). The parameters from the previous mode are not kept internally. The switch
 * from one frame to another must be done in STDBY_RC mode.
 * @param type
 * @return
 */
sx1262_api_return_t sx1262_rf_packet_type (sx1262_rf_packet_type_t type)
{

}

/**
 * Returns currently selected packet type
 * @param type output pointer to an instance of @link{sx1262_rf_packet_type_t}
 * @return
 */
sx1262_api_return_t sx1262_rf_packet_type_get (sx1262_rf_packet_type_t *type)
{

}

/**
 * This command sets the TX output power by using the parameter transmit_power and the TX ramping
time by using the parameter ramp_time. This command is available for all protocols selected.
 * @param transmit_power -9 to +22 (0x16) dBm by step of 1 dB
 * @param ramp_time TX ramping time
 * @return
 */
sx1262_api_return_t sx1262_rf_tx_params (int8_t transmit_power, sx1262_rf_tx_ramp_time_t ramp_time)
{

}

/**
 * The command is used to configure the modulation parameters of the radio. It is explicitly
tailored
 * to LoRa.
 * @param sf corresponds to the Spreading Factor used for the LoRa® modulation.
 * @param bw corresponds to the bandwidth onto which the LoRa® signal is spread.
 * @param cr coding rate
 * @param opt corresponds to the Low Data Rate Optimization (LDRO). This parameter is usually set
when the LoRa® symbol time is equal or above 16.38 ms (typically for SF11 with BW125 and SF12 with
BW125 and BW250).
 * @return
 */
sx1262_api_return_t sx1262_rf_lora_modulation_params (sx1262_rf_lora_spreading_factor_t sf,
													  sx1262_rf_lora_bandwidth_t bw,
													  sx1262_rf_lora_coding_rate_t cr,
													  sx1262_rf_lora_low_datarate_optimize_t opt)
{

}

/**
 * The command is used to configure the packet parameters of the radio. It is explicitly tailored
 * to LoRa.
 * @param preamble_ln number of symbols sent as preamble
 * @param payload_ln Size of the payload (in bytes) to transmit or maximum size of the
 * payload that the receiver can accept.
 * @param header_ln see description of @link{sx1262_rf_lora_header_t}
 * @param enable_crc
 * @param invert_iq
 * @return
 */
sx1262_api_return_t sx1262_rf_lora_packet_params (uint16_t preamble_ln, uint8_t payload_ln,
												  sx1262_rf_lora_header_t header_ln,
												  uint8_t enable_crc, uint8_t invert_iq)
{

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
 * @return
 */
sx1262_api_return_t
sx1262_rf_lora_channel_act_detect (sx1262_rf_lora_channel_act_symbols_num_t symbol_num,
								   sx1262_rf_lora_channel_act_symbols_num_t det_peak,
								   sx1262_rf_lora_channel_act_symbols_num_t det_min,
								   sx1262_rf_lora_channel_act_exit_mode_t exit_mode,
								   uint32_t timeout)
{
    
}
