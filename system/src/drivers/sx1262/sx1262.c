/*
 * sx1262.c
 *
 *  Created on: May 19, 2025
 *      Author: mateusz
 */

#include "./drivers/sx1262/sx1262.h"
#include "./drivers/sx1262/sx1262_internals.h"

#include "LedConfig.h"

#include <stdint.h>

#include <stm32l4xx.h>
#include <stm32l4xx_ll_gpio.h>

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

void sx1262_init(void)
{
	//!< Used across this file to configure I/O pins
	LL_GPIO_InitTypeDef GPIO_InitTypeDef;

	// INTERRUPT - PC6
	GPIO_InitTypeDef.Mode = LL_GPIO_MODE_INPUT;
	GPIO_InitTypeDef.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitTypeDef.Pin = LL_GPIO_PIN_6;
	GPIO_InitTypeDef.Pull = LL_GPIO_PULL_NO;
	GPIO_InitTypeDef.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
	GPIO_InitTypeDef.Alternate = LL_GPIO_AF_0;
	LL_GPIO_Init(GPIOC, &GPIO_InitTypeDef);

	// IS BUSY - PC7
	GPIO_InitTypeDef.Pin = LL_GPIO_PIN_7;
	LL_GPIO_Init(GPIOC, &GPIO_InitTypeDef);

	// RESET output - A12
	GPIO_InitTypeDef.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitTypeDef.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitTypeDef.Pin = LL_GPIO_PIN_12;
	GPIO_InitTypeDef.Pull = LL_GPIO_PULL_NO;
	GPIO_InitTypeDef.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
	GPIO_InitTypeDef.Alternate = LL_GPIO_AF_0;
	LL_GPIO_Init(GPIOA, &GPIO_InitTypeDef);

	// keep RESET output hi-z
	LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_12);

	// EXTI6[3:0]: EXTI 6 configuration bits (EXTI6[3]) is only available on STM32L49x/L4Ax)
	// These bits are written by software to select the source input for the EXTI6 external interrupt.
	SYSCFG->EXTICR[1] |= SYSCFG_EXTICR2_EXTI6_PC;

	// EXTI7[3:0]: EXTI 7 configuration bits (EXTI7[3]) is only available on STM32L49x/L4Ax)
	// These bits are written by software to select the source input for the EXTI7 external interrupt.
	// 0111: PH[7] pin (only on STM32L49x/L4Ax devices)
	// 0010: PC[7] pin
	// #define SYSCFG_EXTICR2_EXTI7_PC             (0x00002000UL)                     /*!<PC[7] pin */
	SYSCFG->EXTICR[1] |= SYSCFG_EXTICR2_EXTI7_PC;

#ifdef SX1262_SHMIDT_NOT_GATE
	        // 1: Falling trigger enabled (for Event and Interrupt) for input line
	        EXTI->FTSR1 |= EXTI_FTSR1_FT6;          // INTERRUPT
	        // 1: Rising trigger enabled (for Event and Interrupt) for input line
	        EXTI->RTSR1 |= EXTI_RTSR1_RT7;          // IS BUSY
#else
	        // 1: Rising trigger enabled (for Event and Interrupt) for input line
	        EXTI->RTSR1 |= EXTI_RTSR1_RT6;          // INTERRUPT
	        // 1: Falling trigger enabled (for Event and Interrupt) for input line
	        EXTI->FTSR1 |= EXTI_FTSR1_FT7;          // IS BUSY
#endif

	EXTI->IMR1 |= EXTI_IMR1_IM6;
	EXTI->IMR1 |= EXTI_IMR1_IM7;

	//   EXTI9_5_IRQn                = 23,     // !< External Line[9:5] Interrupts
	NVIC_EnableIRQ( EXTI9_5_IRQn );

	sx1262_busy_flag = SX1262_BUSY_NOTACTIVE;
	sx1262_interrupt_flag = SX1262_BUSY_NOTACTIVE;
}

void sx1262_busy_released_callback(void)
{
	sx1262_busy_counter--;
	sx1262_busy_flag = SX1262_BUSY_NOTACTIVE;
	   //led_blink_led2_botoom();

}

void sx1262_interrupt_callback(void)
{
	sx1262_interrupt_flag = SX1262_BUSY_ACTIVE;
}

short sx1262_is_busy_io_line_active(void)
{
	if (LL_GPIO_IsInputPinSet(GPIOC, LL_GPIO_PIN_7) == SX1262_BUSY_ACTIVE)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

short sx1262_is_interrrupt_io_line_active(void)
{
	if (LL_GPIO_IsInputPinSet(GPIOC, LL_GPIO_PIN_6) == SX1262_BUSY_ACTIVE)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

short sx1262_is_busy_flag_active(void)
{
	if (sx1262_busy_flag == SX1262_BUSY_ACTIVE)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

short sx1262_is_interrrupt_flag_active(void)
{
	if (sx1262_interrupt_flag == SX1262_BUSY_ACTIVE)
	{
		sx1262_interrupt_flag = SX1262_BUSY_NOTACTIVE;
		return 1;
	}
	else
	{
		return 0;
	}
}

void sx1262_set_busy_flag_for_waiting(void)
{
	sx1262_busy_flag = SX1262_BUSY_ACTIVE;
}

