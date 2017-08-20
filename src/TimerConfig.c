/*
 * TimerConfig.c
 *
 *  Created on: 19.03.2017
 *      Author: mateusz
 */

#include <stm32f10x.h>
#include <stm32f10x_tim.h>
#include "TimerConfig.h"
#include "station_config.h"


extern volatile int delay_5us;

#if (_DELAY_BASE > 16)
#error "Transmit delay shouldn't be longer that 800msec. Decrease _DELAY_BASE in config below 16"
#endif

void TIM2Delay(char delay) {
#ifdef _RANDOM_DELAY
	delay_5us = delay + ((uint16_t)ADC1->DR % 10) * 2;
#else
	delay_5us = delay;
#endif

	NVIC_SetPriority(TIM2_IRQn, 1);
	TIM2->PSC = 9999;
	TIM2->ARR = 119;		// frekwencja 20Hz - 1 tick = 50ms
	TIM2->CR1 |= TIM_CR1_DIR;
	TIM2->CR1 &= (0xFFFFFFFF ^ TIM_CR1_DIR);
	TIM2->CR1 |= TIM_CR1_CEN;
	TIM2->DIER |= 1;
	NVIC_EnableIRQ( TIM2_IRQn );
}

void TIM2DelayDeConfig(void) {
	TIM2->CR1 &= (0xFFFFFF ^ TIM_CR1_CEN);
}

void TimerConfig(void) {
	///////////////////////////////////////////
	/// konfiguracja TIM2 -- dallas delay 	///
	///////////////////////////////////////////
	NVIC_SetPriority(TIM2_IRQn, 1);
	TIM2->PSC = 0;
	TIM2->ARR = 119;
	TIM2->CR1 |= TIM_CR1_DIR;
	TIM2->CR1 &= (0xFFFFFFFF ^ TIM_CR1_DIR);
//	TIM2->CR1 |= TIM_CR1_CEN;
	TIM2->DIER |= 1;
	NVIC_EnableIRQ( TIM2_IRQn );

	// 	//////////////////////////////
	// 	////   konfiguracja TIM4 -- dac  ///
	// 	//////////////////////////////
		NVIC_SetPriority(TIM4_IRQn, 1);
		TIM4->PSC = 0;
		TIM4->ARR = 2499;
		TIM4->CR1 |= TIM_CR1_DIR;
		TIM4->CR1 &= (0xFFFFFFFF ^ TIM_CR1_DIR);
	// 	TIM4->CR1 |= TIM_CR1_CEN;			/* timer powinien byc uruchamiany tylko przy wysylaniu danych */
		TIM4->DIER |= 1;
		NVIC_EnableIRQ( TIM4_IRQn );
		///////////////////////////////////////////
		/// konfiguracja TIM7 --adc 	///
		///////////////////////////////////////////
		NVIC_SetPriority(TIM7_IRQn, 2);
		TIM7->PSC = 0;
		TIM7->ARR = 624;			/// 2499
		TIM7->CR1 |= TIM_CR1_DIR;
		TIM7->CR1 &= (0xFFFFFFFF ^ TIM_CR1_DIR);
		TIM7->CR1 |= TIM_CR1_CEN;
		TIM7->DIER |= 1;
		NVIC_EnableIRQ( TIM7_IRQn );
		///////////////////////////////////////////
		/// konfiguracja TIM3 -- wysylanie wlasnej pozycji i danych WX 	///
		///////////////////////////////////////////
		NVIC_SetPriority(TIM3_IRQn, 4);
		TIM3->PSC = 65534;			/* Czêstotliwoæ za dzielnikiem: 366Hz */
		TIM3->ARR = 21960;			/* Wyzwalanie przerwania co jednṗ minutê */
		TIM3->CR1 |= TIM_CR1_DIR;
		TIM3->CR1 &= (0xFFFFFFFF ^ TIM_CR1_DIR);
		TIM3->CR1 |= TIM_CR1_CEN;
		TIM3->DIER |= 1;
		NVIC_EnableIRQ( TIM3_IRQn );

}
