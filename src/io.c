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
#include <drivers/gpio_conf_stm32f1x.h>
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

#ifdef STM32L471xx
		GPIO_InitTypeDef.Mode = LL_GPIO_MODE_OUTPUT;
		GPIO_InitTypeDef.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
		GPIO_InitTypeDef.Pin = LL_GPIO_PIN_12;
		GPIO_InitTypeDef.Pull = LL_GPIO_PULL_NO;
		GPIO_InitTypeDef.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
		GPIO_InitTypeDef.Alternate = LL_GPIO_AF_7;
		LL_GPIO_Init(GPIOA, &GPIO_InitTypeDef);
#endif
}

void io_ext_watchdog_service(void) {
#ifdef STM32F10X_MD_VL

#endif

#ifdef STM32L471xx

#endif
}

//#ifdef STM32F10X_MD_VL
//void io_5v_isol_sw_cntrl_vbat_s_enable(void) {
//	//GPIO_SetBits(GPIOB, GPIO_Pin_8);
//	GPIOB->BSRR |= GPIO_BSRR_BS8;
//}
//void io_5v_isol_sw_cntrl_vbat_s_disable(void) {
//	//GPIO_ResetBits(GPIOB, GPIO_Pin_8);
//	GPIOB->BSRR |= GPIO_BSRR_BR8;
//}
//
//void io_12v_sw_cntrl_vbat_g_enable(void) {
//	//GPIO_SetBits(GPIOA, GPIO_Pin_6);
//	GPIOA->BSRR |= GPIO_BSRR_BS6;
//
//}
//void io_12v_sw_cntrl_vbat_g_disable(void) {
//	//GPIO_ResetBits(GPIOA, GPIO_Pin_6);
//	GPIOA->BSRR |= GPIO_BSRR_BR6;
//
//}
//
//#endif
//
//#ifdef STM32L471xx
//void io_5v_isol_sw_cntrl_vbat_s_enable(void) {
//}
//void io_5v_isol_sw_cntrl_vbat_s_disable(void) {
//}
//
//void io_12v_sw_cntrl_vbat_g_enable(void) {
//}
//void io_12v_sw_cntrl_vbat_g_disable(void) {
//}
//#endif
