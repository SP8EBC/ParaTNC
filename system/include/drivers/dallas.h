/*
 * dallas.h
 *
 *  Created on: 25.04.2017
 *      Author: mateusz
 */

#ifndef DALLAS_H
#define DALLAS_H

/* C++ detection */
#ifdef __cplusplus
extern "C" {
#endif

#include <stm32f10x_gpio.h>


extern volatile int delay_5us;
extern volatile char timm;

typedef struct DallasStruct {
	GPIO_TypeDef* GPIOx;           /*!< GPIOx port to be used for I/O functions */
	uint16_t GPIO_Pin;             /*!< GPIO Pin to be used for I/O functions */
	uint32_t GPIO_Cnf;             /*!< GPIO Pin to be used for I/O functions */
	uint32_t GPIO_Mode;             /*!< GPIO Pin to be used for I/O functions */
	uint32_t input_term;
	uint32_t output_term;
	uint32_t clear_term;
	uint8_t shift;
}DallasStruct;

void DallasInit(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint16_t GPIO_PinSource);
void DallasConfigTimer(void);
void DallasDeConfigTimer(void);
char DallasReset(void);
float DallasQuery(void);
void DallasSendByte(char data);
char DallasReceiveByte(void);
uint8_t CalculateCRC8(uint8_t *addr, uint8_t len);


/* C++ detection */
#ifdef __cplusplus
}
#endif

#endif
