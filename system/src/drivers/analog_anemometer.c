/*
 * analog_anemometer.c
 *
 *  Created on: 25.12.2019
 *      Author: mateusz
 */

#include "drivers/analog_anemometer.h"

#include <stdint.h>
#include <stm32f10x_tim.h>
#include <stm32f10x_dma.h>
#include "drivers/gpio_conf.h"

uint16_t analog_anemometer_windspeed_pulses_time[ANALOG_ANEMOMETER_SPEED_PULSES_N];

void analog_anemometer_init(uint16_t pulses_per_ms, uint16_t mvolts_for_1deg,
		uint16_t mvolts_for_359deg, uint8_t reversed) {

	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	DMA_InitTypeDef DMA_InitStruct;

	// enabling the clock for TIM17
	RCC->APB2ENR |= RCC_APB2ENR_TIM17EN;
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;

	// Configuring a pin where pulses from anemometer are connected
	Configure_GPIO(GPIOB,9,FLOATING_INPUT);

	// resetting the timer to defaults
	TIM_DeInit(TIM17);

	// initializing structure with default values
	TIM_TimeBaseStructInit(&TIM_TimeBaseInitStruct);

	TIM_TimeBaseInitStruct.TIM_Prescaler = 23999;					// PSC 23999
	TIM_TimeBaseInitStruct.TIM_Period = 60000;					// ARR
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;

	// Configuring basics of thr timer
	TIM_TimeBaseInit(TIM17, &TIM_TimeBaseInitStruct);

	// Enabling capture input
	TIM_TIxExternalClockConfig(TIM17, TIM_TIxExternalCLK1Source_TI1, TIM_ICPolarity_Rising, 0);

	// Starting timer
	TIM_Cmd(TIM17, ENABLE);

	// Enabling a DMA request signal from first capture-compare channel
	TIM_DMACmd(TIM17, TIM_DMA_CC1, ENABLE);

	// Enabling an interrupt
	TIM_ITConfig(TIM17, TIM_IT_Update, ENABLE);
	NVIC_EnableIRQ( TIM1_TRG_COM_TIM17_IRQn );

	// Initializing the struct with DMA configuration
	DMA_StructInit(&DMA_InitStruct);

	// De initializing DMA1
	DMA_DeInit(DMA1_Channel7);

	DMA_InitStruct.DMA_BufferSize = ANALOG_ANEMOMETER_SPEED_PULSES_N;
	DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;
	DMA_InitStruct.DMA_MemoryBaseAddr = analog_anemometer_windspeed_pulses_time;
	DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStruct.DMA_PeripheralBaseAddr = &TIM17->CCR1;
	DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;

	DMA_Init(DMA1_Channel7, &DMA_InitStruct);
	DMA1_Channel7->CCR |= DMA_CCR7_EN;
	DMA1_Channel7->CCR |= DMA_CCR7_TCIE;

	NVIC_EnableIRQ( DMA1_Channel7_IRQn );


	return;
}

void analog_anemometer_timer_irq(void) {

}

void analog_anemometer_dma_irq(void) {
	DMA_ClearITPendingBit(DMA1_IT_GL7);
}
