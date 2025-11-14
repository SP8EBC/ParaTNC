/*
 * it_handlers.h
 *
 *  Created on: 27.01.2019
 *      Author: mateusz
 */

#ifndef IT_HANDLERS_H_
#define IT_HANDLERS_H_

#include <stdint.h>

/// ==================================================================================================
///	X-MACROS
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL MACROS
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL DEFINITIONS
/// ==================================================================================================

#define IT_HANDLERS_PROXY_KISS_UART_EV		   (1 << 0)
#define IT_HANDLERS_PROXY_KISS_TX_UART_EV	   (1 << 1)
#define IT_HANDLERS_PROXY_GSM_RX_UART_EV	   (1 << 2)
#define IT_HANDLERS_PROXY_GSM_TX_UART_EV	   (1 << 3)
#define IT_HANDLERS_PROXY_NEW_RADIO_MESSAGE_EV (1 << 4)
#define IT_HANDLERS_PROXY_SX1262_INTERRUPT	   (1 << 5)
#define IT_HANDLERS_PROXY_SX1262_ISBUSY		   (1 << 6)
#define IT_HANDLERS_PROXY_WX_RX_UART_EV		   (1 << 7)
#define IT_HANDLERS_PROXY_WX_RX_ERROR_UART_EV  (1 << 8)
#define IT_HANDLERS_PROXY_WX_TX_UART_EV		   (1 << 9)

/// ==================================================================================================
///	GLOBAL TYPES
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL VARIABLES
/// ==================================================================================================

extern uint8_t it_handlers_inhibit_radiomodem_dcd_led;

extern volatile uint32_t it_handlers_freertos_proxy;

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

void it_handlers_set_priorities (void);

#endif /* IT_HANDLERS_H_ */
