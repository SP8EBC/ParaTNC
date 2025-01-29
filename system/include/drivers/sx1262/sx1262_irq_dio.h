/*
 * sx1262_irq_dio.h
 *
 *  Created on: Jan 25, 2025
 *      Author: mateusz
 */

#ifndef INCLUDE_DRIVERS_SX1262_SX1262_IRQ_DIO_H_
#define INCLUDE_DRIVERS_SX1262_SX1262_IRQ_DIO_H_

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

typedef enum sx1262_irq_dio_tcxo_voltage_t {
	SX1262_IRQ_DIO_TCXO_VOLTAGE_1_6 = 0,
	SX1262_IRQ_DIO_TCXO_VOLTAGE_1_7 = 1,
	SX1262_IRQ_DIO_TCXO_VOLTAGE_1_8 = 2,
	SX1262_IRQ_DIO_TCXO_VOLTAGE_2_2 = 3,
	SX1262_IRQ_DIO_TCXO_VOLTAGE_2_4 = 4,
	SX1262_IRQ_DIO_TCXO_VOLTAGE_2_7 = 5,
	SX1262_IRQ_DIO_TCXO_VOLTAGE_3_0 = 6,
	SX1262_IRQ_DIO_TCXO_VOLTAGE_3_3 = 7,
} sx1262_irq_dio_tcxo_voltage_t;

/// ==================================================================================================
///	GLOBAL VARIABLES
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

sx1262_api_return_t sx1262_irq_dio_get_mask(uint16_t * iterrupt_mask);

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
															   uint8_t timeout, uint8_t crc_error);

/**
 * This command is used to configure DIO2 so that it can be used to control an external RF switch.
 * When controlling the external RX switch, the pin DIO2 will toggle accordingly to the internal
 * state machine. DIO2 will be asserted high a few microseconds before the ramp-up of the PA and
 * will go bes et to zero after the ramp-down of the PA. DIO2 = 0 in SLEEP, STDBY_RX, STDBY_XOSC, FS
 * and RX modes, DIO2 = 1 in TX mode
 * @param enable if set to non zero DIO2 is selected to be used to control an RF switch
 * @return
 */
sx1262_api_return_t sx1262_irq_dio_set_dio2_as_rf_switch (uint8_t enable);

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
														  uint16_t delay);

#endif /* INCLUDE_DRIVERS_SX1262_SX1262_IRQ_DIO_H_ */
