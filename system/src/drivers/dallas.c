/*
 * dallas.c
 *
 *  Created on: 25.04.2017
 *      Author: mateusz
 */

#include "drivers/dallas.h"
#include <stm32f10x.h>
#include <stm32f10x_gpio.h>
#include <string.h>

volatile int delay_5us = 0;
volatile char timm = 0;

//GPIO_InitTypeDef GPIO_input;
//GPIO_InitTypeDef GPIO_output;

DallasStruct dallas;

void DallasInit(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint16_t GPIO_PinSource) {
//	GPIO_output.GPIO_Mode = GPIO_Mode_Out_OD;
//	GPIO_output.GPIO_Pin = GPIO_Pin;
//	GPIO_output.GPIO_Speed = GPIO_Speed_50MHz;

//	GPIO_input.GPIO_Mode = GPIO_Mode_IN_FLOATING;
//	GPIO_input.GPIO_Pin = GPIO_Pin;
//	GPIO_input.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_Init(GPIOx, &GPIO_input);

	dallas.GPIOx = GPIOx;
	dallas.GPIO_Pin = GPIO_Pin;

	dallas.GPIO_Mode = (3 << GPIO_PinSource * 4);
	dallas.GPIO_Cnf = (3 << GPIO_PinSource * 4) + 2;
	dallas.shift = GPIO_PinSource * 4 + 2;

	dallas.clear_term = 0xFFFFFFFF ^ (dallas.GPIO_Mode | dallas.GPIO_Cnf);
	dallas.input_term = 1 << dallas.shift;
	dallas.output_term = (1 << dallas.shift - 2) | (1 << dallas.shift);

}

void DallasConfigTimer(void) {
	// Disabling any time-consuming iterrupts
	NVIC_DisableIRQ( TIM3_IRQn );			// data transmission initializer
	NVIC_DisableIRQ( TIM4_IRQn );			// data transmission initializer
	NVIC_DisableIRQ( TIM7_IRQn );			// data transmission initializer
	NVIC_DisableIRQ( 25 );	// anemometer

	NVIC_SetPriority(TIM2_IRQn, 1);
	TIM2->PSC = 0;
	TIM2->ARR = 119;
	TIM2->CR1 |= TIM_CR1_DIR;
	TIM2->CR1 &= (0xFFFFFFFF ^ TIM_CR1_DIR);
	TIM2->CR1 |= TIM_CR1_CEN;
	TIM2->DIER |= 1;
	NVIC_EnableIRQ( TIM2_IRQn );	// enabling in case that it weren't been enabled earlier
	timm = 1;
}

void DallasDeConfigTimer(void) {
	TIM2->CR1 &= (0xFFFFFFFF ^ TIM_CR1_CEN);	// disabling timer

	NVIC_EnableIRQ( TIM3_IRQn );	// adc
	NVIC_EnableIRQ( TIM4_IRQn );	// data transmission initializer
	NVIC_EnableIRQ( TIM7_IRQn );	// data transmission initializer
	NVIC_EnableIRQ( 25 ); // anemometer

	// reverting back to APRS timings
	NVIC_SetPriority(TIM4_IRQn, 1);
	TIM4->PSC = 0;
	TIM4->ARR = 2499;
	TIM4->CR1 |= TIM_CR1_DIR;
	TIM4->CR1 &= (0xFFFFFFFF ^ TIM_CR1_DIR);
	TIM4->DIER |= 1;
//	NVIC_EnableIRQ( TIM4_IRQn );
	timm = 0;
}

char DallasReset(void) {
	// PULLING LINE LOW
	dallas.GPIOx->CRL &=  dallas.clear_term;
	dallas.GPIOx->CRL |= dallas.output_term;
	dallas.GPIOx->BSRR |= (dallas.GPIO_Pin << 16);	// line low
	delay_5us = 100;		// delay 500us
	while (delay_5us != 0);

	// WAITING FOR SLAVE PRESENT PULSE
	dallas.GPIOx->BSRR |= (dallas.GPIO_Pin);		// line high
	dallas.GPIOx->CRL &=  dallas.clear_term;
	dallas.GPIOx->CRL |= dallas.input_term;
	delay_5us = 20;			// delay 100us
	while (delay_5us != 0);

	// READING PIN STATE
	if ((dallas.GPIOx->IDR & dallas.GPIO_Pin) == GPIO_Pin_6) {
		delay_5us = 100;		// delay 500us
		while (delay_5us != 0);
		return -1;
	}
	else;
	delay_5us = 100;		// delay 500us
	while (delay_5us != 0);
	return 0;
}

void __attribute__((optimize("O0"))) DallasSendByte(char data) {
	char i;
	for (i = 0; i < 8; i++) {
		// PULLING LINE LOW
		dallas.GPIOx->CRL &=  dallas.clear_term;
		dallas.GPIOx->CRL |= dallas.output_term;
		dallas.GPIOx->BSRR |= (dallas.GPIO_Pin << 16);	// line low
		delay_5us = ((data >> i) & 0x01) ? 2 : 13;		// delay 10us if sending logic "1", or 75us if "0"
		while (delay_5us != 0);

		// PULLING LINE BACK HIGH
		dallas.GPIOx->BSRR |= (dallas.GPIO_Pin);		// line high
		delay_5us = ((data >> i) & 0x01) ? 13 : 2;		// delay 70us if sending logc "1", or 5us if "0"
		while (delay_5us != 0);

	}
}

char __attribute__((optimize("O0"))) DallasReceiveByte(void) {
	char data = 0, i;

	for (i = 0; i < 8; i++) {

		// PULLING LINE LOW
		dallas.GPIOx->CRL &=  dallas.clear_term;
		dallas.GPIOx->CRL |= dallas.output_term;
		dallas.GPIOx->BSRR |= (dallas.GPIO_Pin << 16);	// line low
		delay_5us = 2;		// delay 10us
		while (delay_5us != 0);

		// PULLING LINE BACK HIGH
		dallas.GPIOx->BSRR |= (dallas.GPIO_Pin);		// line high
//		GPIO_Init(dallas.GPIOx, &GPIO_input);				// przeroic!!
		dallas.GPIOx->CRL &=  dallas.clear_term;
		dallas.GPIOx->CRL |= dallas.input_term;
		delay_5us = 1;		// delay 10us
		while (delay_5us != 0);

		// SAMPLING PIN
		data |= (((dallas.GPIOx->IDR & dallas.GPIO_Pin) > 0 ? 1 : 0) << i);
		delay_5us = 11;		// delay 50us for complete slot
		while (delay_5us != 0);

	}
	delay_5us = 20;		// delay 100us for complete slot
	while (delay_5us != 0);

	return data;
}

float __attribute__((optimize("O0"))) DallasQuery(void) {
	unsigned char data[9];
	int crc;
	char temp1, temp2, sign, i;
	unsigned temp3;
	float temperature = 0.0f;

	// ENABLE ONEWIRE DELAY TIMER
	DallasConfigTimer();

	memset(data, 0x00, 9);
	DallasReset();
	DallasSendByte(0xCC);	// ROM skip
	DallasSendByte(0x44);	// Temperature conversion
	delay_5us = 190000;		// 800msec delay for conversion
	while (delay_5us != 0);
	DallasReset();
	DallasSendByte(0xCC);
	DallasSendByte(0xBE);	// read scratchpad
	for (i = 0; i <= 8; i++)
		data[i] = DallasReceiveByte();

	// DISABLE ONEWIRE DELAY TIMER
	DallasDeConfigTimer();

	crc = CalculateCRC8(data, 8);
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
		return -128.0f;
	}
	return temperature;

}

uint8_t CalculateCRC8(uint8_t *addr, uint8_t len) {
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
