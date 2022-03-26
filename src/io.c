/*
 * io.c
 *
 *  Created on: 11.06.2020
 *      Author: mateusz
 */


#include "station_config_target_hw.h"

#include "io.h"

#ifdef STM32F10X_MD_VL
#include <stm32f10x.h>
#include <drivers/f1/gpio_conf_stm32f1x.h>
#endif
#ifdef STM32L471xx
#include <stm32l4xx.h>
#include <stm32l4xx_ll_gpio.h>
#endif

#include "station_config.h"

#if defined(PARAMETEO)
LL_GPIO_InitTypeDef GPIO_InitTypeDef;
#endif

void io_oc_init(void) {
#ifdef STM32F10X_MD_VL
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
#endif
}

void io_oc_output_low(void) {
#ifdef STM32F10X_MD_VL

	GPIO_SetBits(GPIOA, GPIO_Pin_11);
#endif
}

void io_oc_output_hiz(void) {
#ifdef STM32F10X_MD_VL

	GPIO_ResetBits(GPIOA, GPIO_Pin_11);
#endif
}

void io_ext_watchdog_config(void) {
#ifdef STM32F10X_MD_VL
	  // initialize Watchdog output
	  Configure_GPIO(GPIOA,12,GPPP_OUTPUT_50MHZ);
#endif

}

void io_ext_watchdog_service(void) {
#ifdef STM32F10X_MD_VL

#endif

#ifdef STM32L471xx
	if ((GPIOA->ODR & GPIO_ODR_OD1) == 0) {
		// set high
		GPIOA->BSRR |= GPIO_BSRR_BS1;
	}
	else {
		// set low
		GPIOA->BSRR |= GPIO_BSRR_BR1;
	}
#endif
}

