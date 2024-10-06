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
#endif


#if (_DELAY_BASE > 22)
#error "Transmit delay shouldn't be longer that 1100msec. Decrease _DELAY_BASE in config below 22"
#endif

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
 	//////////////////////////////
 	////   konfiguracja TIM4 -- dac  ///
 	//////////////////////////////
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
		///////////////////////////////////////////
		/// konfiguracja TIM7 --adc 	///
		///////////////////////////////////////////
		//NVIC_SetPriority(TIM7_IRQn, 3);
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

}

void TIM2Delay(void) {
	TIM2->CR1 |= TIM_CR1_CEN;
}

void TIM2DelayDeConfig(void) {
	TIM2->CR1 &= (0xFFFFFFFF ^ TIM_CR1_CEN);
}
