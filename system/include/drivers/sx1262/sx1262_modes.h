/*
 * sx1262.h
 *
 *  Created on: Dec 14, 2024
 *      Author: mateusz
 */

#ifndef INCLUDE_DRIVERS_SX1262_H_
#define INCLUDE_DRIVERS_SX1262_H_

#include <stdint.h>

#include "drivers/sx1262/sx1262_api_return_t.h"

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
 * The command SetSleep(...) is used to set the device in SLEEP mode with the lowest current consumption possible.
 * This command can be sent only while in STDBY mode (STDBY_RC or STDBY_XOSC). After the rising edge of NSS,
 * all blocks are switched OFF except the backup regulator if needed and the blocks specified in the
 * parameter sleepConfig.
 * @param cold_warm_start 0: cold start, 1: warm start (device configuration in retention)
 * @param rtc_timeout_disable_enable 0: RTC timeout disable, 1: wake-up on RTC timeout
 * @return success or kind of an error. For more info refer to @link{sx1262_api_return_t} declaration
 */
sx1262_api_return_t sx1262_modes_set_sleep(uint8_t cold_warm_start, uint8_t rtc_timeout_disable_enable);

/**
 * The command SetStandby(...) is used to set the device in a configuration mode which is at an intermediate level of
 * consumption. In this mode, the chip is placed in halt mode waiting for instructions via SPI.
 * This mode is dedicated to chip configuration using high level commands such as SetPacketType(...).
 * By default, after battery insertion or reset operation (pin NRESET goes low), the chip will enter in STDBY_RC
 * mode running with a 13 MHz RC clock.
 * @param standby_rc_xosc 0Device running on RC13M, set STDBY_RC mode, 1Device running on XTAL 32MHz, set STDBY_XOSC mode
 * @return success or kind of an error. For more info refer to @link{sx1262_api_return_t} declaration
 */
sx1262_api_return_t sx1262_modes_set_standby(uint8_t standby_rc_xosc);

/**
 * The command SetFs() is used to set the device in the frequency synthesis mode where the PLL is locked to the carrier
 * frequency. This mode is used for test purposes of the PLL and can be considered as an intermediate mode. It is
 * automatically reached when going from STDBY_RC mode to TX mode or RX mode.
 * @return success or kind of an error. For more info refer to @link{sx1262_api_return_t} declaration
 */
sx1262_api_return_t sx1262_modes_set_fs(void);

/**
 * The command SetTx() sets the device in transmit mode.
 * @param timeout Timeout duration = timeout * 15.625 μs. the device remains in TX mode, it returns automatically to STBY_RC mode on timer
 * end-of-count or when a packet has been transmitted. The maximum timeout is then 262 s.
 */
sx1262_api_return_t sx1262_modes_set_tx(uint32_t timeout);

/**
 * The command SetRx() sets the device in receiver mode.
 * @param timeout 0xFFFFFF: Rx Continuous mode, The device remains in RX mode until the host sends a command to change the
 * operation mode. Others: Timeout active. The device remains in RX mode, it returns automatically to STBY_RC mode on timer
 * end-of-count or when a packet has been received.
 * @return success or kind of an error. For more info refer to @link{sx1262_api_return_t} declaration
 */
sx1262_api_return_t sx1262_modes_set_rx(uint32_t timeout);

/**
 * The command StopTimerOnPreamble(...) allows the user to select if the timer is stopped upon preamble detection of Sync
 * Word / header detection.
 * @param disable_enable 0x00: Timer is stopped upon Sync Word or Header detection, 0x01: Timer is stopped upon preamble detection
 * @return success or kind of an error. For more info refer to @link{sx1262_api_return_t} declaration
 */
sx1262_api_return_t sx1262_modes_stop_timer_on_preamble(uint8_t disable_enable);

/**
 * This command sets the chip in sniff mode so that it regularly looks for new packets. This is the listen mode.
 * @param rx_period The RX mode duration is defined by Rx Duration = rx_period * 15.625 μs
 * @param sleep_period The SLEEP mode duration is defined by: Sleep Duration = sleep_period * 15.625 μs
 * @return success or kind of an error. For more info refer to @link{sx1262_api_return_t} declaration
 */
sx1262_api_return_t sx1262_modes_set_rx_duty_cycle(uint32_t rx_period, uint32_t sleep_period);

/**
 * The command SetCAD() can be used only in LoRa® packet type. The Channel Activity Detection is a LoRa® specific mode of
 * operation where the device searches for the presence of a LoRa® preamble signal. After the search has completed, the
 * device returns in STDBY_RC mode. The length of the search is configured via the command SetCadParams(...).
 * @return success or kind of an error. For more info refer to @link{sx1262_api_return_t} declaration
 */
sx1262_api_return_t sx1262_modes_set_cad(void);

/**
 *
 * SetTxContinuousWave() is a test command available for all packet types to generate a continuous wave (RF tone) at selected
 * frequency and output power. The device stays in TX continuous wave until the host sends a mode configuration command.
 * @return success or kind of an error. For more info refer to @link{sx1262_api_return_t} declaration
 */
sx1262_api_return_t sx1262_modes_set_tx_cw(void);

/**
 * SetTxInfinitePreamble() is a test command to generate an infinite sequence of alternating zeros and ones in FSK modulation.
 * In LoRa®, the radio is only able to constantly modulate LoRa® preamble symbols. The device will remain in TX infinite
 * preamble until the host sends a mode configuration command.
 * @return success or kind of an error. For more info refer to @link{sx1262_api_return_t} declaration
 */
sx1262_api_return_t sx1262_modes_set_tx_infinite_preamble(void);

/**
 * By default only the LDO is used. This is useful in low cost applications where the cost of the extra self needed for a DC-DC
 * converter is prohibitive. Using only a linear regulator implies that the RX or TX current is almost doubled. This function
 * allows to specify if DC-DC or LDO is used for power regulation. The regulation mode is defined by parameter ldo_dcdcldo.
 * @param ldo_dcdcldo 0: Only LDO used for all modes, 1: DC_DC+LDO used for STBY_XOSC,FS, RX and TX modes
 * @return success or kind of an error. For more info refer to @link{sx1262_api_return_t} declaration
 */
sx1262_api_return_t sx1262_modes_set_regulator_mode(uint8_t ldo_dcdcldo);

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
sx1262_api_return_t sx1262_modes_set_calibrate_function(uint8_t rc64k, uint8_t rc13m, uint8_t pll, uint8_t adc_ps, uint8_t adc_n, uint8_t adc_p, uint8_t image);

/**
 * The function CalibrateImage(...) allows the user to calibrate the image rejection of the device for the device operating
 * frequency band.
 * @param freq_lo
 * @param freq_hi
 * @return success or kind of an error. For more info refer to @link{sx1262_api_return_t} declaration
 */
sx1262_api_return_t sx1262_modes_set_calibrate_image(uint8_t freq_lo, uint8_t freq_hi);

/**
 * SetPaConfig is the command to set transmit power. Internally it select values for 'paDutyCycle' and 'hpMax'
 * to match a value in dBm provided by a caller
 * @param tx_power_dbm
 * @return success or kind of an error. For more info refer to @link{sx1262_api_return_t} declaration
 */
sx1262_api_return_t sx1262_modes_set_pa_config(uint8_t tx_power_dbm);

/**
 * The command SetRxTxFallbackMode defines into which mode the chip goes after a successful transmission or after a packet
 * reception.
 * @param fallback_mode 0x40 The radio goes into FS mode after Tx or Rx, 0x30 The radio goes into STDBY_XOSC mode after Tx or Rx,
 * 0x20 The radio goes into STDBY_RC mode after Tx or Rx
 * @return success or kind of an error. For more info refer to @link{sx1262_api_return_t} declaration
 */
sx1262_api_return_t sx1262_modes_set_rxtx_fallback_mode(uint8_t fallback_mode);

#endif /* INCLUDE_DRIVERS_SX1262_H_ */
