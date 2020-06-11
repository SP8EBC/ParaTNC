/*
 * io.c
 *
 *  Created on: 11.06.2020
 *      Author: mateusz
 */

#include "io.h"
#include <stm32f10x.h>

#include "station_config.h"

void io_oc_init(void) {
#if (defined PARATNC_HWREV_C)

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
#endif
}

void io_oc_output_low(void) {
#if (defined PARATNC_HWREV_C)

	GPIO_SetBits(GPIOA, GPIO_Pin_11);
#endif
}

void io_oc_output_hiz(void) {
#if (defined PARATNC_HWREV_C)

	GPIO_ResetBits(GPIOA, GPIO_Pin_11);

#endif
}

