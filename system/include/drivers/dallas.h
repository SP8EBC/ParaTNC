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
#include "station_config.h"



extern volatile int delay_5us;
extern volatile char timm;

typedef struct DallasStruct {
	GPIO_TypeDef* GPIOx;           /*!< GPIOx port to be used for I/O functions */
	uint16_t GPIO_Pin;             /*!< GPIO Pin to be used for I/O functions */
	uint16_t GPIO_Pin_input;
	uint32_t GPIO_Cnf;             /*!< GPIO Pin to be used for I/O functions */
	uint32_t GPIO_Mode;             /*!< GPIO Pin to be used for I/O functions */
	uint32_t input_term;
	uint32_t output_term;
	uint32_t clear_term;
	uint8_t shift;
}DallasStruct;

typedef enum DallasQF {
	DALLAS_QF_UNKNOWN = 0,
	DALLAS_QF_FULL,
	DALLAS_QF_DEGRADATED,
	DALLAS_QF_NOT_AVALIABLE
}DallasQF;

void dallas_init(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint16_t GPIO_PinSource);
void dallas_config_timer(void);
void dallas_deconfig_timer(void);
char dallas_reset(void);
float dallas_query(DallasQF *qf);
void dallas_send_byte(char data);
char dallas_receive_byte(void);
uint8_t dallas_calculate_crc8(uint8_t *addr, uint8_t len);




/* C++ detection */
#ifdef __cplusplus
}
#endif

#endif
