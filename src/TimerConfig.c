/*
 * TimerConfig.c
 *
 *  Created on: 19.03.2017
 *      Author: mateusz
 */


#include "TimerConfig.h"
#include "station_config.h"
#include "station_config_target_hw.h"

#ifdef STM32F10X_MD_VL
#include <stm32f10x.h>
#include <stm32f10x_tim.h>
#endif

#ifdef STM32L471xx
#include <stm32l4xx.h>
#include <stm32l4xx_ll_tim.h>
#include <stm32l4xx_ll_lptim.h>
#endif


#if (_DELAY_BASE > 22)
#error "Transmit delay shouldn't be longer that 1100msec. Decrease _DELAY_BASE in config below 22"
#endif

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
	TIM1->PSC = 47999;		// 24MHz -> 1kHz		// 9
#else
	TIM1->PSC = 4;
#endif

	// ARR is the value to be loaded in the actual auto-reload register.
	// Refer to the Section 30.3.1: Time-base unit on page 911 for more details about ARR update
	// and behavior.
	// The counter is blocked while the auto-reload value is null.
	TIM1->ARR = 9;		// 1kHz -> 100Hz (period : 10ms)

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

#ifdef STM32F10X_MD_VL
	// 	//////////////////////////////
	// 	////   konfiguracja TIM4 -- dac  ///
	// 	//////////////////////////////
		//NVIC_SetPriority(TIM4_IRQn, 2);
		TIM4->PSC = 0;
		TIM4->ARR = 2499;
		TIM4->CR1 |= TIM_CR1_DIR;
		TIM4->CR1 &= (0xFFFFFFFF ^ TIM_CR1_DIR);
	// 	TIM4->CR1 |= TIM_CR1_CEN;			/* timer powinien byc uruchamiany tylko przy wysylaniu danych */
		TIM4->DIER |= 1;
		NVIC_EnableIRQ( TIM4_IRQn );
#else
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
#endif

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
		TIM7->DIER |= 1;
		NVIC_EnableIRQ( TIM7_IRQn );
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
