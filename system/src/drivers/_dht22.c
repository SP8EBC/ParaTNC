
/*
 * _dht22.c
 *
 *  Created on: 17.04.2018
 *      Author: mateusz
 */

#include <stm32f10x_gpio.h>
#include <stm32f10x_exti.h>
#include "../drivers/_dht22.h"
#include "../drivers/dallas.h"	// for delay
#include <stdint.h>
#include <stdio.h>

uint8_t bitsDuration[40];
uint8_t currentBit;

GPIO_InitTypeDef PORT_out, PORT_in;
EXTI_InitTypeDef exti, exti_disable;


void dht22_init(void) {
	memset(bitsDuration, 0x00, 40);
	currentBit = 0;

	/*
	 * Initializing of the GPIO pin used to communicatie with sensor
	 */
	GPIO_StructInit(&PORT_out);
	GPIO_StructInit(&PORT_in);
	PORT_out.GPIO_Mode = GPIO_Mode_Out_OD;
	PORT_out.GPIO_Pin = DHT22_PIN_PIN;
	PORT_out.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(DHT22_PIN_PORT,&PORT_out);

	PORT_in.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	PORT_in.GPIO_Pin = DHT22_PIN_PIN;
	PORT_in.GPIO_Speed = GPIO_Speed_50MHz;

	EXTI_StructInit(&exti);
	exti.EXTI_Line = EXTI_Line4;
	exti.EXTI_Mode = EXTI_Mode_Interrupt;
	exti.EXTI_Trigger = EXTI_Trigger_Falling;
	exti.EXTI_LineCmd = ENABLE;

	EXTI_StructInit(&exti_disable);
	exti.EXTI_Line = EXTI_Line4;
	exti.EXTI_Mode = EXTI_Mode_Interrupt;
	exti.EXTI_Trigger = EXTI_Trigger_Falling;
	exti.EXTI_LineCmd = DISABLE;
}

void dht22_comm(dht22Values *in) {

	GPIO_Init(DHT22_PIN_PORT,&PORT_out);
	GPIO_SetBits(DHT22_PIN_PORT, DHT22_PIN_PIN);
	DallasConfigTimer();

	/*
	 * Setting pin logic-low to initialize transfer.
	 */
	GPIO_ResetBits(DHT22_PIN_PORT, DHT22_PIN_PIN);
	delay_5us = DHT22_START_SIG_DURATION;
	while (delay_5us != 0);
	GPIO_SetBits(DHT22_PIN_PORT, DHT22_PIN_PIN);

	/*
	 * Waiting for the response pulse from sensor
	 */
	GPIO_Init(DHT22_PIN_PORT,&PORT_in);
	delay_5us = DHT22_WAITING_FOR_START_RESP_DURATION;
	while (delay_5us != 0);
	uint8_t sensorResp = GPIO_ReadInputDataBit(DHT22_PIN_PORT, DHT22_PIN_PIN);
	if (sensorResp == Bit_SET) {
		in->qf = DHT22_QF_UNAVALIABLE;
		return;		// if pin is still high it usually means that there is a problem with comm with the sensor
	}
	else;

	/*
	 * Starting the edge detected interrupt on this pin (EXTI)
	 */
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource4);
	EXTI_Init(&exti);
	EXTI->FTSR |= 1 << 4;
	EXTI->IMR |= 1 << 4;
	NVIC_EnableIRQ(EXTI4_IRQn);

	delay_5us = DHT22_INTERRUPT_DURATION;
	return;
	/*
	 * Now
	 */

}

void EXTI4_IRQHandler(void) {
  EXTI->PR |= EXTI_PR_PR4;
  bitsDuration[currentBit++] = delay_5us;
  delay_5us = DHT22_INTERRUPT_DURATION;
  if (currentBit >= 40) {
	  EXTI_Init(&exti_disable);
	  currentBit = 0;
  }

}
