/*
 * TimerConfig.c
 *
 *  Created on: 19.03.2017
 *      Author: mateusz
 */


#include "TimerConfig.h"
#include "station_config.h"
#include "station_config_target_hw.h"
#include <stm32l4xx.h>
#include <stm32l4xx_ll_tim.h>
#include <stm32l4xx_ll_lptim.h>
#include <stm32l4xx_ll_dma.h>

#if (_DELAY_BASE > 22)
#error "Transmit delay shouldn't be longer that 1100msec. Decrease _DELAY_BASE in config below 22"
#endif

extern unsigned short int AdcBuffer[4];

LL_DMA_InitTypeDef timer_config_DMA_InitStruct;

void TimerTimebaseConfig(void)
{
	///////////////////////////////////////////
	/// konfiguracja TIM1 -- master time 	///
	///////////////////////////////////////////
 	/// 48MHz of input clock

	// These bits allow the user to set-up the update rate of the compare registers (i.e. periodic
	// 	transfers from preload to active registers) when preload registers are enable, as well as the
	// 	update interrupt generation rate, if this interrupt is enable
	//
	//The repetition counter is decremented:
	// •At each counter overflow in upcounting mode,
	// •At each counter underflow in downcounting mode,
	TIM1->RCR = 0;

	// The counter clock frequency (CK_CNT) is equal to fCK_PSC / (PSC[15:0] + 1).
	// 	PSC contains the value to be loaded in the active prescaler register at each update event
	//
	// The prescaler can divide the counter clock frequency by any factor between 1 and 65536. It
	// is based on a 16-bit counter controlled through a 16-bit register (in the TIMx_PSC register).
	// It can be changed on the fly as this control register is buffered. The new prescaler ratio is
	// taken into account at the next update event.
#ifdef HI_SPEED
	TIM1->PSC = 479;		// 48MHz -> 100kHz
#else
	TIM1->PSC = 4;
#endif

	// ARR is the value to be loaded in the actual auto-reload register.
	// Refer to the Section 30.3.1: Time-base unit on page 911 for more details about ARR update
	// and behavior.
	// The counter is blocked while the auto-reload value is null.
	TIM1->ARR = 999;		// 100kHz -> 100Hz (period : 10ms)

	// DIR: Direction
	// 0: Counter used as upcounter
	// 1: Counter used as downcounter
	TIM1->CR1 &= (0xFFFFFFFF ^ TIM_CR1_DIR);

	// DMA/interrupt enable register, Reset value: 0x0000
	// but to be 100% sure reset value do default
	TIM1->DIER = 0x0000U;

	// Update interrupt enable
	TIM1->DIER |= TIM_DIER_UIE;

	TIM1->CR1 |= TIM_CR1_CEN;

	NVIC_EnableIRQ( TIM1_UP_TIM16_IRQn );
}


void TimerConfig(void) {

	///////////////////////////////////////////
	/// konfiguracja TIM2 -- dallas delay 	///
	///////////////////////////////////////////
#ifdef HI_SPEED
	TIM2->PSC = 1;
#else
	TIM2->PSC = 0;
#endif
	TIM2->ARR = 119;
	TIM2->CR1 |= TIM_CR1_DIR;
	TIM2->CR1 &= (0xFFFFFFFF ^ TIM_CR1_DIR);
//	TIM2->CR1 |= TIM_CR1_CEN;
	TIM2->DIER |= 1;
	NVIC_EnableIRQ( TIM2_IRQn );

 	////////////////////////////////////
 	////   konfiguracja TIM4 -- dac  ///
 	///////////////////////////////////
 	/// 48MHz of input clock
#ifdef HI_SPEED
			TIM5->PSC = 1;
#else
			TIM5->PSC = 0;
#endif
			TIM5->ARR = 2499;
			TIM5->CR1 |= TIM_CR1_DIR;
			TIM5->CR1 &= (0xFFFFFFFF ^ TIM_CR1_DIR);
		// 	TIM4->CR1 |= TIM_CR1_CEN;			/* timer powinien byc uruchamiany tylko przy wysylaniu danych */
			TIM5->DIER |= 1;
			NVIC_EnableIRQ( TIM5_IRQn );

#ifndef NO_AUDIO_REALTIME_RX
		///////////////////////////////////////////
		/// konfiguracja TIM7 --adc 			///
		///////////////////////////////////////////
		/// 48MHz of input clock
#ifdef HI_SPEED
		TIM7->PSC = 1;
#else
		TIM7->PSC = 0;
#endif
		TIM7->ARR = 624;			/// 2499
		TIM7->CR1 |= TIM_CR1_DIR;
		TIM7->CR1 &= (0xFFFFFFFF ^ TIM_CR1_DIR);
		TIM7->CR1 |= TIM_CR1_CEN;
		//TIM7->DIER |= TIM_DIER_UIE;
		//NVIC_EnableIRQ( TIM7_IRQn );
		TIM7->DIER |= TIM_DIER_UDE;

		///////////////////////////////////////////
		/// konfiguracja DMA2 --adc 			///
		///////////////////////////////////////////
		timer_config_DMA_InitStruct.Direction = LL_DMA_DIRECTION_PERIPH_TO_MEMORY;
		timer_config_DMA_InitStruct.MemoryOrM2MDstAddress = (uint32_t)AdcBuffer;
		timer_config_DMA_InitStruct.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_HALFWORD;
		timer_config_DMA_InitStruct.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
		timer_config_DMA_InitStruct.Mode = LL_DMA_MODE_NORMAL;
		timer_config_DMA_InitStruct.NbData = 4;
		timer_config_DMA_InitStruct.PeriphOrM2MSrcAddress = (uint32_t)&ADC1->DR;
		timer_config_DMA_InitStruct.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_HALFWORD;
		timer_config_DMA_InitStruct.PeriphOrM2MSrcIncMode = LL_DMA_MEMORY_NOINCREMENT;
		timer_config_DMA_InitStruct.PeriphRequest = LL_DMA_REQUEST_3; // LL_DMAMUX_REQ_TIM7_UP

		LL_DMA_Init(DMA2, LL_DMA_CHANNEL_5, &timer_config_DMA_InitStruct);

		LL_DMA_EnableIT_TC(DMA2, LL_DMA_CHANNEL_5);

		LL_DMA_EnableChannel(DMA2, LL_DMA_CHANNEL_5);

		NVIC_EnableIRQ( DMA2_Channel5_IRQn );
#endif
}

void TIM2Delay(void) {
	TIM2->CR1 |= TIM_CR1_CEN;
}

void TIM2DelayDeConfig(void) {
	TIM2->CR1 &= (0xFFFFFFFF ^ TIM_CR1_CEN);
}

void TimerAdcDisable(void) {
	#ifndef NO_AUDIO_REALTIME_RX
	TIM7->CR1 &= (0xFFFFFFFF ^ TIM_CR1_CEN);
	TIM7->SR &= (0xFFFFFFFF ^ TIM_SR_UIF_Msk);
	NVIC_ClearPendingIRQ(TIM7_IRQn);
	NVIC_DisableIRQ( TIM7_IRQn );
	#endif
}

void TimerAdcEnable(void) {
	#ifndef NO_AUDIO_REALTIME_RX
	NVIC_EnableIRQ( TIM7_IRQn );
	TIM7->CR1 |= TIM_CR1_CEN;
	#endif
}
