/*
 * sx1262_rf.h
 *
 *  Created on: Jan 21, 2025
 *      Author: mateusz
 */

#ifndef INCLUDE_DRIVERS_SX1262_SX1262_RF_H_
#define INCLUDE_DRIVERS_SX1262_SX1262_RF_H_

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

/**
 *
 */
typedef enum sx1262_rf_packet_type_t {
	SX1262_RF_PACKET_TYPE_GFSK = 0x01,			 /**< SX1262_RF_PACKET_TYPE_GFSK */
	SX1262_RF_PACKET_TYPE_LORA = 0x03,			 /**< SX1262_RF_PACKET_TYPE_LORA */
	SX1262_RF_PACKET_TYPE_LONG_RANGE_FHSS = 0x04 /**< SX1262_RF_PACKET_TYPE_LONG_RANGE_FHSS */
} sx1262_rf_packet_type_t;

/**
 *
 */
typedef enum sx1262_rf_tx_ramp_time_t {
	SX1262_RF_TX_RAMPTIME_10US = 0x0,	/**< SX1262_RF_TX_RAMPTIME_10US */
	SX1262_RF_TX_RAMPTIME_20US = 0x1,	/**< SX1262_RF_TX_RAMPTIME_20US */
	SX1262_RF_TX_RAMPTIME_40US = 0x2,	/**< SX1262_RF_TX_RAMPTIME_40US */
	SX1262_RF_TX_RAMPTIME_80US = 0x3,	/**< SX1262_RF_TX_RAMPTIME_80US */
	SX1262_RF_TX_RAMPTIME_200US = 0x4,	/**< SX1262_RF_TX_RAMPTIME_200US */
	SX1262_RF_TX_RAMPTIME_800US = 0x5,	/**< SX1262_RF_TX_RAMPTIME_800US */
	SX1262_RF_TX_RAMPTIME_1700US = 0x6, /**< SX1262_RF_TX_RAMPTIME_1700US */
	SX1262_RF_TX_RAMPTIME_3400US = 0x7, /**< SX1262_RF_TX_RAMPTIME_3400US */

} sx1262_rf_tx_ramp_time_t;

/**
 * LoRa® ModParam1- SF
 */
typedef enum sx1262_rf_lora_spreading_factor_t {
	SX1262_RF_LORA_SF5 = 0x05,	/**< SX1262_RF_LORA_SF5 */
	SX1262_RF_LORA_SF6 = 0x06,	/**< SX1262_RF_LORA_SF6 */
	SX1262_RF_LORA_SF7 = 0x07,	/**< SX1262_RF_LORA_SF7 */
	SX1262_RF_LORA_SF8 = 0x08,	/**< SX1262_RF_LORA_SF8 */
	SX1262_RF_LORA_SF9 = 0x09,	/**< SX1262_RF_LORA_SF9 */
	SX1262_RF_LORA_SF10 = 0x0A, /**< SX1262_RF_LORA_SF10 */
	SX1262_RF_LORA_SF11 = 0x0B, /**< SX1262_RF_LORA_SF11 */
	SX1262_RF_LORA_SF12 = 0x0C, /**< SX1262_RF_LORA_SF12 */

} sx1262_rf_lora_spreading_factor_t;

/**
 * LoRa® ModParam2 - BW
 */
typedef enum sx1262_rf_lora_bandwidth_t {
	SX1262_RF_LORA_BW7 = 0x0,	/**< sx1262_RF_LORA_BW7 */
	SX1262_RF_LORA_BW10 = 0x8,	/**< sx1262_RF_LORA_BW10 */
	SX1262_RF_LORA_BW15 = 0x1,	/**< sx1262_RF_LORA_BW15 */
	SX1262_RF_LORA_BW20 = 0x9,	/**< sx1262_RF_LORA_BW20 */
	SX1262_RF_LORA_BW31 = 0x2,	/**< sx1262_RF_LORA_BW31 */
	SX1262_RF_LORA_BW41 = 0xA,	/**< sx1262_RF_LORA_BW41 */
	SX1262_RF_LORA_BW62 = 0x3,	/**< sx1262_RF_LORA_BW62 */
	SX1262_RF_LORA_BW125 = 0x4, /**< sx1262_RF_LORA_BW125 */
	SX1262_RF_LORA_BW250 = 0x5, /**< sx1262_RF_LORA_BW250 */
	SX1262_RF_LORA_BW500 = 0x6, /**< sx1262_RF_LORA_BW500 */

} sx1262_rf_lora_bandwidth_t;

/**
 * LoRa® ModParam3 - CR
 */
typedef enum sx1262_rf_lora_coding_rate_t {
	SX1262_RF_LORA_CR_45 = 0x01, /**< SX1262_RF_LORA_CR_45 */
	SX1262_RF_LORA_CR_46 = 0x02, /**< SX1262_RF_LORA_CR_46 */
	SX1262_RF_LORA_CR_47 = 0x03, /**< SX1262_RF_LORA_CR_47 */
	SX1262_RF_LORA_CR_48 = 0x04, /**< SX1262_RF_LORA_CR_48 */
} sx1262_rf_lora_coding_rate_t;

/**
 * LoRa® ModParam4 - LowDataRateOptimize
 */
typedef enum sx1262_rf_lora_low_datarate_optimize_t {
	SX1262_RF_LORA_OPTIMIZE_OFF = 0x00, /**< SX1262_RF_LORA_OPTIMIZE_OFF */
	SX1262_RF_LORA_OPTIMIZE_ON = 0x01,	/**< SX1262_RF_LORA_OPTIMIZE_ON */
} sx1262_rf_lora_low_datarate_optimize_t;

/**
 * LoRa® PacketParam3 - HeaderType
 */
typedef enum sx1262_rf_lora_header_t {
	SX1262_RF_LORA_HEADER_VARIABLE_LN_PACKET =
		0x00,									 /**< Variable length packet (explicit header) */
	SX1262_RF_LORA_HEADER_FIXED_LN_PACKET = 0x01 /**< Fixed length packet (implicit header) */
} sx1262_rf_lora_header_t;

/**
 *
 */
typedef enum sx1262_rf_lora_channel_act_symbols_num_t {
	SX1262_RF_LORA_CHANNEL_ACT_ON_1_SYMBOLS = 0x00, /**< SX1262_RF_LORA_CHANNEL_ACT_ON_1_SYMBOLS */
	SX1262_RF_LORA_CHANNEL_ACT_ON_2_SYMBOLS = 0x01, /**< SX1262_RF_LORA_CHANNEL_ACT_ON_2_SYMBOLS */
	SX1262_RF_LORA_CHANNEL_ACT_ON_4_SYMBOLS = 0x02, /**< SX1262_RF_LORA_CHANNEL_ACT_ON_4_SYMBOLS */
	SX1262_RF_LORA_CHANNEL_ACT_ON_8_SYMBOLS = 0x03, /**< SX1262_RF_LORA_CHANNEL_ACT_ON_8_SYMBOLS */
	SX1262_RF_LORA_CHANNEL_ACT_ON_16_SYMBOLS =
		0x04, /**< SX1262_RF_LORA_CHANNEL_ACT_ON_16_SYMBOLS */

} sx1262_rf_lora_channel_act_symbols_num_t;

/**
 * The parameter defines the action to be done after a CAD operation. This is optional.
 */
typedef enum sx1262_rf_lora_channel_act_exit_mode_t {
	SX1262_RF_LORA_CHANNEL_ACT_EXIT_RC_ALWAYS, /**< The chip performs the CAD operation in LoRa®.
												  Once done and whatever the activity on the
												  channel, the chip goes back to STBY_RC mode. */
	SX1262_RF_LORA_CHANNEL_ACT_EXIT_RX, /**< The chip performs a CAD operation and if an activity is
										   detected, it stays in RX until a packet is detected or
										   the timer reaches the timeout defined by cadTimeout
										   * 15.625 us */
} sx1262_rf_lora_channel_act_exit_mode_t;

/// ==================================================================================================
///	GLOBAL VARIABLES
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
sx1262_api_return_t sx1262_rf_buffer_base_addresses (uint8_t tx_base, uint8_t rx_base);

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
sx1262_api_return_t sx1262_rf_frequency (uint32_t frequency);

/**
 * The command sets the radio in LoRa®, FSK or Long Range FHSS mode. The command
 * must be the first of the radio configuration sequence. The parameter for this command is
 * sx1262_rf_packet_type_t. Changing from one mode of operation to another is done using the command
 * SetPacketType(...). The parameters from the previous mode are not kept internally. The switch
 * from one frame to another must be done in STDBY_RC mode.
 * @param type
 * @return
 */
sx1262_api_return_t sx1262_rf_packet_type (sx1262_rf_packet_type_t type);

/**
 * Returns currently selected packet type
 * @param type output pointer to an instance of @link{sx1262_rf_packet_type_t}
 * @return
 */
sx1262_api_return_t sx1262_rf_packet_type_get (sx1262_rf_packet_type_t *type);

/**
 * This command sets the TX output power by using the parameter transmit_power and the TX ramping
time by using the parameter ramp_time. This command is available for all protocols selected.
 * @param transmit_power -9 to +22 (0x16) dBm by step of 1 dB
 * @param ramp_time TX ramping time
 * @return
 */
sx1262_api_return_t sx1262_rf_tx_params (int8_t transmit_power, sx1262_rf_tx_ramp_time_t ramp_time);

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
													  sx1262_rf_lora_low_datarate_optimize_t opt);

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
												  uint8_t enable_crc, uint8_t invert_iq);

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
								   uint32_t timeout);

#endif /* INCLUDE_DRIVERS_SX1262_SX1262_RF_H_ */
