/*
 * dallas.h
 *
 *  Created on: 25.04.2017
 *      Author: mateusz
 */

#ifndef DALLAS_H
#define DALLAS_H

#include <stdint.h>

//#include "station_config_target_hw.h"

/* C++ detection */
#ifdef __cplusplus
extern "C" {
#endif

#ifdef STM32F10X_MD_VL
#include <stm32f10x_gpio.h>
#endif
#ifdef STM32L471xx
#include <stm32l4xx.h>
#include <stm32l4xx_ll_gpio.h>
#endif

#include "station_config.h"

#define DALLAS_AVERAGE_LN 9
#define DALLAS_INIT_VALUE -128.0f

extern volatile int delay_5us;
extern volatile char timm;

typedef struct dallas_struct_t {
	GPIO_TypeDef* GPIOx;           /*!< GPIOx port to be used for I/O functions */
	uint16_t GPIO_Pin;             /*!< GPIO Pin to be used for I/O functions */
	uint16_t GPIO_Pin_input;
	uint32_t GPIO_Cnf;             /*!< GPIO Pin to be used for I/O functions */
	uint32_t GPIO_Mode;             /*!< GPIO Pin to be used for I/O functions */
	uint32_t input_term;
	uint32_t output_term;
	uint32_t clear_term;
	uint8_t shift;
}dallas_struct_t;

typedef struct dallas_average_t {
	float values[DALLAS_AVERAGE_LN];
	float *begin, *end, *current;
}dallas_average_t;

typedef enum dallas_qf_t {
	DALLAS_QF_UNKNOWN = 0,
	DALLAS_QF_FULL,
	DALLAS_QF_DEGRADATED,
	DALLAS_QF_NOT_AVALIABLE
}dallas_qf_t;

void dallas_init(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint16_t GPIO_PinSource, dallas_average_t* average);
void dallas_config_timer(void);
void dallas_deconfig_timer(void);
char dallas_reset(void);
float dallas_query(dallas_qf_t *qf);
void dallas_send_byte(char data);
char dallas_receive_byte(void);
uint8_t dallas_calculate_crc8(uint8_t *addr, uint8_t len);
void dallas_average(float in, dallas_average_t* average);
float dallas_get_average(const dallas_average_t* average);
float dallas_get_min(const dallas_average_t* average);
float dallas_get_max(const dallas_average_t* average);




/* C++ detection */
#ifdef __cplusplus
}
#endif

#endif
