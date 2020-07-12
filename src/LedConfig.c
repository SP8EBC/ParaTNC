/*
 * LedConfig.c
 *
 *  Created on: 05.07.2017
 *      Author: mateusz
 */

#include "LedConfig.h"
#include <stm32f10x.h>

// PC8 - LED1 - upper
// PC9 - LED2 - lower

uint8_t led_blinking_led2;
uint8_t led_blinking_led1;

void led_init(void) {
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
	}

	if (led_blinking_led2 == BLINK_MSEC_PER_SVC_CALL) {
		led_flip_led2_bottom();
	}



}

