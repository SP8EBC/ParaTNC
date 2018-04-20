
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

uint8_t bitsDuration[41];
uint8_t currentBit;
uint8_t bytes[5];

uint8_t dht22State = 0;

GPIO_InitTypeDef PORT_out, PORT_in;
EXTI_InitTypeDef exti, exti_disable;


void dht22_init(void) {
	memset(bitsDuration, 0x00, 41);
	memset(bytes, 0x00, 5);
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
	GPIO_SetBits(DHT22_PIN_PORT, DHT22_PIN_PIN);

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

	dht22State = DHT22_STATE_IDLE;
}

void dht22_comm(dht22Values *in) {

	dht22State = DHT22_STATE_COMMS;

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
		dht22State = DHT22_STATE_TIMEOUT;
		DallasDeConfigTimer();
		if (in != 0x00)
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
  bitsDuration[currentBit++] = DHT22_INTERRUPT_DURATION - delay_5us;
  delay_5us = DHT22_INTERRUPT_DURATION;
  if (currentBit >= 41) {
	  EXTI_Init(&exti_disable);
	  NVIC_DisableIRQ(EXTI4_IRQn);
	  currentBit = 0;
	  GPIO_Init(DHT22_PIN_PORT,&PORT_out);
	  GPIO_SetBits(DHT22_PIN_PORT, DHT22_PIN_PIN);
	  dht22State = DHT22_STATE_DATA_RDY;
	  DallasDeConfigTimer();
  }

}

void dht22_decode(dht22Values *data) {
	if (data == 0x00)
		return;

	for (int i = 0; i < 41; i++) {
		if (bitsDuration[i] > DHT22_MAX_ZERO_DURATION)
			bitsDuration[i] = 1;
		else
			bitsDuration[i] = 0;
	}
	bytes[0] = (bitsDuration[1] << 7) | (bitsDuration[2] << 6) | (bitsDuration[3] << 5) | (bitsDuration[4] << 4) | (bitsDuration[5] << 3) | (bitsDuration[6] << 2) | (bitsDuration[7] << 1) | (bitsDuration[8]);
	bytes[1] = (bitsDuration[9] << 7) | (bitsDuration[10] << 6) | (bitsDuration[11] << 5) | (bitsDuration[12] << 4) | (bitsDuration[13] << 3) | (bitsDuration[14] << 2) | (bitsDuration[15] << 1) | (bitsDuration[16]);
	bytes[2] = (bitsDuration[17] << 7) | (bitsDuration[18] << 6) | (bitsDuration[19] << 5) | (bitsDuration[20] << 4) | (bitsDuration[21] << 3) | (bitsDuration[22] << 2) | (bitsDuration[23] << 1) | (bitsDuration[24]);
	bytes[3] = (bitsDuration[25] << 7) | (bitsDuration[26] << 6) | (bitsDuration[27] << 5) | (bitsDuration[28] << 4) | (bitsDuration[29] << 3) | (bitsDuration[30] << 2) | (bitsDuration[31] << 1) | (bitsDuration[32]);
	bytes[4] = (bitsDuration[33] << 7) | (bitsDuration[34] << 6) | (bitsDuration[35] << 5) | (bitsDuration[36] << 4) | (bitsDuration[37] << 3) | (bitsDuration[38] << 2) | (bitsDuration[39] << 1) | (bitsDuration[40]);

	uint8_t checksum = 0xFF & (uint32_t)(bytes[0] + bytes[1] + bytes[2] + bytes[3]);

	data->humidity = (bytes[0] << 8 | bytes[1]) / 10;
	data->scaledTemperature = ((bytes[2] & 0x7F) << 8 | bytes[3]);
	if ((bytes[2] & 0x80) > 0)
		data->scaledTemperature *= -1;
	else;

	if (checksum == bytes[4]) {
		data->qf = DHT22_QF_FULL;
		dht22State = DHT22_STATE_DATA_DECD;
	}
	else {
		data->qf = DHT22_QF_DEGRADATED;
		dht22State = DHT22_STATE_IDLE;
	}
}

void dht22_timeout_keeper(void) {
	if (dht22State == DHT22_STATE_COMMS) {
		if (delay_5us == 0) {
			dht22State = DHT22_STATE_TIMEOUT;
			dht22_init();
			EXTI_Init(&exti_disable);
			DallasDeConfigTimer();

		}
	}
}
