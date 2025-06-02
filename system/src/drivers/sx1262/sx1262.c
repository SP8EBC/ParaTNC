/*
 * sx1262.c
 *
 *  Created on: May 19, 2025
 *      Author: mateusz
 */

#include "sx1262/sx1262.h"

#include <stdint.h>

#include <stm32l4xx.h>
#include <stm32l4xx_ll_gpio.h>

void sx1262_init(void)
{
	//!< Used across this file to configure I/O pins
	LL_GPIO_InitTypeDef GPIO_InitTypeDef;

	// INTERRUPT - PC6
	GPIO_InitTypeDef.Mode = LL_GPIO_MODE_INPUT;
	GPIO_InitTypeDef.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
	GPIO_InitTypeDef.Pin = LL_GPIO_PIN_6;
	GPIO_InitTypeDef.Pull = LL_GPIO_PULL_NO;
	GPIO_InitTypeDef.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
	GPIO_InitTypeDef.Alternate = LL_GPIO_AF_7;
	LL_GPIO_Init(GPIOC, &GPIO_InitTypeDef);

	// IS BUSY - PC7
	GPIO_InitTypeDef.Pin = LL_GPIO_PIN_7;
	LL_GPIO_Init(GPIOC, &GPIO_InitTypeDef);

	// RESET output - A12
	GPIO_InitTypeDef.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitTypeDef.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
	GPIO_InitTypeDef.Pin = LL_GPIO_PIN_12;
	GPIO_InitTypeDef.Pull = LL_GPIO_PULL_NO;
	GPIO_InitTypeDef.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
	GPIO_InitTypeDef.Alternate = LL_GPIO_AF_7;
	LL_GPIO_Init(GPIOA, &GPIO_InitTypeDef);

	// keep RESET output hi-z
	LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_12);
}


