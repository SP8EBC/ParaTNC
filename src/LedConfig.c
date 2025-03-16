/*
 * LedConfig.c
 *
 *  Created on: 05.07.2017
 *      Author: mateusz
 */

#include "LedConfig.h"
#include "station_config_target_hw.h"

#ifdef STM32F10X_MD_VL
#include <stm32f10x_gpio.h>
#endif
#ifdef STM32L471xx
#include <stm32l4xx.h>
#include <stm32l4xx_ll_gpio.h>
#endif
// PC8 - LED1 - upper
// PC9 - LED2 - lower

uint8_t led_blinking_led2 = 0u;
uint8_t led_blinking_led1 = 0u;

void led_init(void) {
#ifdef PARATNC
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_SetBits(GPIOC, GPIO_Pin_9 | GPIO_Pin_8);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
#endif

#ifdef PARAMETEO
	/**
	 * 	GPIO_InitTypeDef.Mode = LL_GPIO_MODE_INPUT;
	GPIO_InitTypeDef.Pin = LL_GPIO_PIN_2;
	GPIO_InitTypeDef.Pull = LL_GPIO_PULL_NO;
	GPIO_InitTypeDef.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
	GPIO_InitTypeDef.Alternate = LL_GPIO_AF_2;
	 */
	LL_GPIO_InitTypeDef GPIO_InitTypeDef;

	GPIO_InitTypeDef.Pin = LL_GPIO_PIN_9;
	GPIO_InitTypeDef.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
	GPIO_InitTypeDef.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitTypeDef.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitTypeDef.Pull = LL_GPIO_PULL_NO;

	LL_GPIO_Init(GPIOC, &GPIO_InitTypeDef);

	GPIO_InitTypeDef.Pin = LL_GPIO_PIN_8;
	LL_GPIO_Init(GPIOC, &GPIO_InitTypeDef);

	LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_9);
	LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_8);

#endif

}

void led_deinit(void) {
#ifdef PARAMETEO
	LL_GPIO_InitTypeDef GPIO_InitTypeDef;

	GPIO_InitTypeDef.Pin = LL_GPIO_PIN_9;
	GPIO_InitTypeDef.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
	GPIO_InitTypeDef.Mode = LL_GPIO_MODE_ANALOG;
	GPIO_InitTypeDef.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
	GPIO_InitTypeDef.Pull = LL_GPIO_PULL_NO;

	LL_GPIO_Init(GPIOC, &GPIO_InitTypeDef);

	GPIO_InitTypeDef.Pin = LL_GPIO_PIN_8;
	LL_GPIO_Init(GPIOC, &GPIO_InitTypeDef);
#endif
}

void led_service_blink(void) {

	if (led_blinking_led1 > 0) {
		led_blinking_led1 -= BLINK_MSEC_PER_SVC_CALL;

	}

	if (led_blinking_led2 > 0) {
		led_blinking_led2 -= BLINK_MSEC_PER_SVC_CALL;
	}



	if (led_blinking_led1 == BLINK_MSEC_PER_SVC_CALL) {
		led_flip_led1_upper();
		led_blinking_led1 = 0;
	}

	if (led_blinking_led2 == BLINK_MSEC_PER_SVC_CALL) {
		led_flip_led2_bottom();
		led_blinking_led2 = 0;
	}



}

