/*
 * dallas.c
 *
 *  Created on: 25.04.2017
 *      Author: mateusz
 */

#include "station_config_target_hw.h"

#ifdef STM32F10X_MD_VL
#include <stm32f10x.h>
#include <stm32f10x_gpio.h>
#endif

#ifdef STM32L471xx
#include <stm32l4xx.h>
#include <stm32l4xx_ll_gpio.h>
#endif

#include "drivers/dallas.h"
#include "etc/dallas_temperature_limits.h"
#include <string.h>

/* FreeRTOS includes. */
#include <FreeRTOS.h>
#include <task.h>

volatile int delay_5us = 0;
volatile char timm = 0;

dallas_struct_t dallas;

uint8_t dallas_rtos_running = 0;

static void dallas_delay_stop(void) {
	TIM2->CR1 &= (0xFFFFFFFF ^ TIM_CR1_CEN);

	NVIC_DisableIRQ( TIM2_IRQn );
}

static void dallas_delay_start(void) {
	TIM2->CR1 &= (0xFFFFFFFF ^ TIM_CR1_CEN);

	NVIC_SetPriority(TIM2_IRQn, 1);
	TIM2->PSC = 0;
	TIM2->ARR = 239;
	TIM2->CR1 |= TIM_CR1_DIR;
	TIM2->CR1 &= (0xFFFFFFFF ^ TIM_CR1_DIR);
	TIM2->CR1 |= TIM_CR1_CEN;
	TIM2->DIER |= 1;
	NVIC_EnableIRQ( TIM2_IRQn );	// enabling in case that it wasn't been enabled earlier
}

void dallas_init(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint16_t GPIO_PinSource, float_average_t* average) {

#ifdef STM32F10X_MD_VL
	GPIO_InitTypeDef GPIO_input;
	GPIO_InitTypeDef GPIO_output;

	GPIO_output.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_output.GPIO_Pin = GPIO_Pin;
	GPIO_output.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOx, &GPIO_output);
	GPIO_SetBits(GPIOx, GPIO_Pin);

	GPIO_input.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_input.GPIO_Pin = GPIO_Pin << 1;
	GPIO_input.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOx, &GPIO_input);

	dallas.GPIOx = GPIOx;
	dallas.GPIO_Pin = GPIO_Pin;
	dallas.GPIO_Pin_input = GPIO_Pin << 1;
#endif

#ifdef STM32L471xx
	LL_GPIO_InitTypeDef GPIO_input, GPIO_output;

	GPIO_output.Pin = GPIO_Pin;
	GPIO_output.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_output.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
	GPIO_output.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
	//GPIO_output.Alternate = ;
	GPIO_output.Pull = LL_GPIO_PULL_NO;

	GPIO_input.Pin = GPIO_Pin << 1;
	GPIO_input.Mode = LL_GPIO_MODE_INPUT;
	//GPIO_input.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
	GPIO_input.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
	//GPIO_output.Alternate = ;
	GPIO_input.Pull = LL_GPIO_PULL_NO;

	LL_GPIO_Init(GPIOx, &GPIO_output);
	LL_GPIO_Init(GPIOx, &GPIO_input);

	dallas.GPIOx = GPIOx;
	dallas.GPIO_Pin = GPIO_Pin;
	dallas.GPIO_Pin_input = GPIO_Pin << 1;

#endif

	for (int i = 0; i < FLOAT_AVERAGE_LN; i++) {
		average->values[i] = FLOAT_INIT_VALUE;
	}
	average->current = average->values;
	average->begin = average->values;
	average->end = average->values + FLOAT_AVERAGE_LN - 1;
}

void dallas_config_timer(void) {
	// Disabling any time-consuming iterrupts
	//NVIC_DisableIRQ( TIM3_IRQn );			// data transmission initializer
	NVIC_DisableIRQ( TIM4_IRQn );			// data transmission initialize
#ifndef NO_AUDIO_REALTIME_RX
	NVIC_DisableIRQ( TIM7_IRQn );			// data transmission initializer
#endif

	dallas_delay_start();

	//timm = 1;
}

void dallas_deconfig_timer(void) {
	TIM2->CR1 &= (0xFFFFFFFF ^ TIM_CR1_CEN);	// disabling timer

	//NVIC_EnableIRQ( TIM3_IRQn );	// adc
	NVIC_EnableIRQ( TIM4_IRQn );	// data transmission initializer
#ifndef NO_AUDIO_REALTIME_RX
	NVIC_EnableIRQ( TIM7_IRQn );	// data transmission initializer
#endif

	dallas_delay_stop();
	//timm = 0;
}

char dallas_reset(void) {
	// PULLING LINE LOW

	dallas.GPIOx->BSRR |= (dallas.GPIO_Pin << 16);	// line low
	delay_5us = 100;		// delay 500us
	while (delay_5us != 0);

	// WAITING FOR SLAVE PRESENT PULSE
	dallas.GPIOx->BSRR |= (dallas.GPIO_Pin);		// line high
	delay_5us = 20;			// delay 100us
	while (delay_5us != 0);

	// READING PIN STATE
	if ((dallas.GPIOx->IDR & dallas.GPIO_Pin_input) == dallas.GPIO_Pin_input) {
		delay_5us = 100;		// delay 500us
		while (delay_5us != 0);
		return -1;
	}
	else;
	delay_5us = 100;		// delay 500us
	while (delay_5us != 0);
	return 0;
}

void __attribute__((optimize("O0"))) dallas_send_byte(char data) {
	char i;
	for (i = 0; i < 8; i++) {
		// PULLING LINE LOW
		dallas.GPIOx->BSRR |= (dallas.GPIO_Pin << 16);	// line low
		delay_5us = ((data >> i) & 0x01) ? 2 : 13;		// delay 10us if sending logic "1", or 75us if "0"
		while (delay_5us != 0);

		// PULLING LINE BACK HIGH
		dallas.GPIOx->BSRR |= (dallas.GPIO_Pin);		// line high
		delay_5us = ((data >> i) & 0x01) ? 13 : 2;		// delay 70us if sending logc "1", or 5us if "0"
		while (delay_5us != 0);

	}
}

char __attribute__((optimize("O0"))) dallas_receive_byte(void) {
	char data = 0, i;

	for (i = 0; i < 8; i++) {

		// PULLING LINE LOW
		dallas.GPIOx->BSRR |= (dallas.GPIO_Pin << 16);	// line low
		delay_5us = 2;		// delay 10us
		while (delay_5us != 0);

		// PULLING LINE BACK HIGH
		dallas.GPIOx->BSRR |= (dallas.GPIO_Pin);		// line high
		delay_5us = 1;		// delay 10us
		while (delay_5us != 0);

		// SAMPLING PIN
		data |= (((dallas.GPIOx->IDR & dallas.GPIO_Pin_input) > 0 ? 1 : 0) << i);
		delay_5us = 11;		// delay 50us for complete slot
		while (delay_5us != 0);

	}
	delay_5us = 20;		// delay 100us for complete slot
	while (delay_5us != 0);

	return data;
}

float __attribute__((optimize("O0"))) dallas_query(dallas_qf_t *qf) {
	unsigned char data[9];
	int crc;
	int i;
	char temp1, temp2, sign;
	unsigned temp3;
	float temperature = 0.0f;

	if (dallas_rtos_running) {
		taskENTER_CRITICAL();
	}

	// ENABLE ONEWIRE DELAY TIMER
	dallas_config_timer();

	memset(data, 0x00, 9);
	dallas_reset();
	dallas_send_byte(0xCC);	// ROM skip
	dallas_send_byte(0x44);	// Temperature conversion
	delay_5us = 190000;		// 800msec delay for conversion
	while (delay_5us != 0);
	dallas_reset();
	dallas_send_byte(0xCC);
	dallas_send_byte(0xBE);	// read scratchpad
	for (i = 0; i <= 8; i++)
		data[i] = dallas_receive_byte();

	// DISABLE ONEWIRE DELAY TIMER
	dallas_deconfig_timer();

	if (dallas_rtos_running) {
		taskEXIT_CRITICAL();
	}

	crc = dallas_calculate_crc8(data, 8);

	if ((data[0] == 0x00 && data[1] == 0x00 && data[2] == 0x00 && data[3] == 0x00 && data[4] == 0x00 && data[5] == 0x00 && data[6] == 0x00) ||
			(data[0] == 0xFF && data[1] == 0xFF && data[2] == 0xFF && data[3] == 0xFF && data[4] == 0xFF && data[5] == 0xFF && data[6] == 0xFF))
	{
		*qf = DALLAS_QF_NOT_AVALIABLE;
		return -128.0f;
	}

	if (crc == data[8]) {

		temp3 = data[0] | (data[1] << 8);		// combined LSB and MSB
		temp1 = (0x7F0 & temp3) >> 4;			// extracting absolute without decimal part
		temp2 = 0xF & temp3;					// extracting decimal part as number of 1/8 degrees
		sign = (temp3 & 0xF000) ? -1 : 1;		// extracting sign
		if (sign == 1)
			temperature = (float)temp1 + temp2 * 0.0625f;
		else
			temperature = -1.0f * (128.0f - (float)temp1 - (float)temp2 * 0.0625f);
	}
	else {
		*qf = DALLAS_QF_NOT_AVALIABLE;
		return -128.0f;
	}

	if (temperature < DALLAS_TEMPERATURE_LIMITS_LOW || temperature > DALLAS_TEMPERATURE_LIMITS_HI)
		*qf = DALLAS_QF_DEGRADATED;
	else
		*qf = DALLAS_QF_FULL;

	return temperature;

}

uint8_t dallas_calculate_crc8(uint8_t *addr, uint8_t len) {
	uint8_t crc = 0, inbyte, i, mix;

	while (len--) {
		inbyte = *addr++;
		for (i = 8; i; i--) {
			mix = (crc ^ inbyte) & 0x01;
			crc >>= 1;
			if (mix) {
				crc ^= 0x8C;
			}
			inbyte >>= 1;
		}
	}

	/* Return calculated CRC */
	return crc;
}
