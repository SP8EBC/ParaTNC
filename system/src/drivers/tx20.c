#include "drivers/tx20.h"

#ifdef _ANEMOMETER_TX20

#include "../drivers/tx20.h"

#include <stdlib.h>
#include <stm32f10x.h>
#include <math.h>
#include "diag/Trace.h"

#include "rte_wx.h"
#include "main.h"
#include "wx_handler.h"

#include "LedConfig.h"

#define BS TX20.BitSampler
#define BQ TX20.BitQueue
#define QL TX20.QueueLenght
#define DCD TX20.FrameRX
#define FC TX20.FrameBitCounter
#define RD TX20.ReceiveDone
#define MC TX20.MeasCounter
#define PM TX20.PrevMeasCounter
#define OE TX20.OddEven

#define MAX_SLEW_RATE 		9
#define HALF_MAX_SLEW_RATE	4.5f

Anemometer TX20;	// Deklaracja zmiennej strukturalnej typu Anemometer

float tx20_previous_windspeed;
uint16_t tx20_previous_direction;

float tx20_current_windspeed;
uint16_t tx20_current_direction;

#define PI 3.14159265

#ifdef _METEO
void inline TX20BlinkLed(void) {
	led_flip_led2_botoom();
}
#endif

void tx20_init(void) {
	int i;

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	TIMER->PSC = 191;
	TIMER->ARR = 75;
	/*
		Czestotliwosc na wejsciu timera: 24MHz			  1
		Dzielnik czestotliwosci: PSC + 1 = 192    ---> Za dzielnikiem: 125kHz
		Docelowa Czestotliwosc wyzwalania przetwania: 1666Hz 
		ARR = 125kHz / 1666Hz = 75.03
	*/
	TIMER->CR1 |= TIM_CR1_DIR;	//zliczanie w dol
	TIMER->CR1 &= (0xFFFFFFFF ^ TIM_CR1_DIR);  // zliczanie w gore
	TIMER->DIER |= 1;  // w��cza Update Interrupt
	NVIC_EnableIRQ( 25 );			// TIM1_UP_TIM16_IRQn
	////////////////////////////////////////
	//// inicjalizacja p�l struktury      //
	////////////////////////////////////////
	BQ = 0, QL = 0, FC = 0, DCD = 0, RD = 0, MC = 1, OE = 0, PM = 1;
	AFIO->EXTICR[(TX/4)] |= PORTNUM << (TX % 4) * 4;
	EXTI->RTSR |= 1 << TX;
	EXTI->IMR |= 1 << TX;
	if (TX <= 4)
		NVIC_EnableIRQ(6+TX);
	else if (TX > 4 && TX <= 9)
		NVIC_EnableIRQ(EXTI9_5_IRQn);
	else if (TX > 9 && TX <= 15)
		NVIC_EnableIRQ(EXTI15_10_IRQn);
}

void tx20_batch(void) {
	/* Funkcja wyzwalana w przerwaniu 1666 razy na sekund� */
	if (BS++,BS %= 2,BS == 1) {
		BQ <<= 1;		// przesuwanie zawarto�ci kolejki o jedn� pozycje
		BQ |= ((PORT->IDR & (1 << TX)) ? 1 : 0);
		QL++;
		if (((BQ & 0x1F) == START_FRAME) && DCD == 0) {
			DCD = 1;
			FC = 5;
			RD = 0;
			BQ &= 0x1F;
		}
		else;
		if (DCD == 1)
			if (FC == 0x29) {
#ifdef _METEO
				TX20BlinkLed();
#endif
				if (OE >= 3) {
					tx20_data_parse();
					OE = 0;
				}
				else
					OE++;
				DCD = 0, BQ = 0, RD = 1, FC = 0, QL = 0, BS = 0;
				TIMER->CR1 &= (0xFFFFFFFF ^ TIM_CR1_CEN);	// disabling baudrate timer after receiving whole frame
				TIMER->CNT = 0;		// resetting timer counter back to zero
			}
			else
				FC++;
		else {
			;
		}
	}
	else {
		;
	}
}

float tx20_data_average(void) {

	// copy values from previous function call
	tx20_previous_direction = tx20_current_direction;
	tx20_previous_windspeed = tx20_current_windspeed;

	// fetch current measuremeents
	tx20_current_direction = TX20.Data.WindDirX;
	tx20_current_windspeed = TX20.Data.WindSpeed;

	// calculate the difference between current and previous
	int abs_windspeed_diff = ((int32_t)tx20_current_windspeed - (int32_t)tx20_previous_windspeed);
	//int abs_direction_diff = ((int16_t)tx20_current_direction - (int16_t)tx20_previous_direction);

	// check if current measurement is too big in comparison with the previous one
	if (abs_windspeed_diff > MAX_SLEW_RATE) {
		tx20_current_windspeed = tx20_previous_windspeed + (float)HALF_MAX_SLEW_RATE;
		rte_wx_tx20_excessive_slew_rate = 1;
	}
	// check if current measuremenet is too small in comparision with the previous one
	else if (abs_windspeed_diff < -MAX_SLEW_RATE) {
		tx20_current_windspeed = tx20_previous_windspeed - (float)HALF_MAX_SLEW_RATE;
		rte_wx_tx20_excessive_slew_rate = 1;
	}
	else {
		;
	}

	return 0;
}

void tx20_data_parse(void) {
	int temp;
	unsigned long long int raw_frame;
	raw_frame = BQ & 0x3FFFFFFFFFF;
	// kierunek wiatru
	temp = (raw_frame & 0xF00000000) >> 32;
	temp = ~temp;
	temp &= 0xF;
	temp = ((temp & 0x8) >> 3) | ((temp & 0x4) >> 1) | ((temp & 0x2) << 1) | ((temp & 0x1) << 3);
	TX20.Data.WindDirX = (short)(temp * 22.5);
	TX20.Data.CalcChecksum = temp;
	// predkosc wiatru
	temp = (raw_frame & 0xFFF00000) >> 20;
	temp = ~temp;	   	// inwetsja bit�w
	temp &= 0xFFF;
	temp = ((temp & (1 << 11)) >> 11) | ((temp & (1 << 10)) >> 9) | ((temp & (1 << 9)) >> 7) | ((temp & (1 << 8)) >> 5) | ((temp & (1 << 7)) >> 3) | ((temp & (1 << 6)) >> 1) | ((temp & (1 << 5)) << 1) | ((temp & (1 << 4)) << 3) | ((temp & (1 << 3)) << 5) | ((temp & (1 << 2)) << 7) | ((temp & (1 << 1)) << 9) | ((temp & (1 << 1)) << 9) | ((temp & 1) << 11); 
	TX20.Data.CalcChecksum += ((temp & 0xF) + ((temp & 0xF0) >> 4) + ((temp & 0xF00) >> 8));
	TX20.Data.CalcChecksum &= 0xF;
//	temp = __rev(temp);	// endian-swapping
	TX20.Data.WindSpeed = (float)temp*0.1;
	// suma kontrolna
	temp = (raw_frame & 0xF0000) >> 16; 
	temp = ~temp;
	temp &= 0xF;
	temp = ((temp & 0x8) >> 3) | ((temp & 0x4) >> 1) | ((temp & 0x2) << 1) | ((temp & 0x1) << 3);
	TX20.Data.Checksum = temp;
	if (TX20.Data.Checksum == TX20.Data.CalcChecksum)
		tx20_data_average();
	else {
		;
	}

	wx_last_good_wind_time = master_time;
}

uint16_t tx20_get_scaled_windspeed(void) {
	float out = tx20_current_windspeed * 10.0f;

	return (uint16_t) (out);
}

uint16_t tx20_get_wind_direction(void) {
	return tx20_current_direction;
}

#ifdef _ANEMOMETER_TX20
// Przerwania EXTI do synchronizacji

#if TX == 0
void EXTI0_IRQHandler(void) {
  EXTI->PR |= EXTI_PR_PR0;
  TIMER->CNT = 0;

}
#elif TX == 1
void EXTI1_IRQHandler(void) {
  EXTI->PR |= EXTI_PR_PR1;
  TIMER->CNT = 0;

}
#elif TX == 2
void EXTI2_IRQHandler(void) {
  EXTI->PR |= EXTI_PR_PR2;
  TIMER->CNT = 0;

}
#elif TX == 3
void EXTI3_IRQHandler(void) {
  EXTI->PR |= EXTI_PR_PR3;
  TIMER->CNT = 0;

}
#elif TX == 4
void EXTI4_IRQHandler(void) {
  EXTI->PR |= EXTI_PR_PR4;
  TIMER->CNT = 0;

}
#elif TX > 4 && TX <= 9
void EXTI9_5_IRQHandler(void) {
  EXTI->PR |= EXTI_PR_PR0 << TX;

  // TIMER is disabled after each complete frame, so it needs to be started once again
  // when start bit (an endge at the begining of next frame from anemometer) is received
  if ((TIMER->CR1 & TIM_CR1_CEN) == 0 )
  	TIMER->CR1 |= TIM_CR1_CEN;
//  QL = 0;


}
#elif TX > 9 && TX <= 15
void EXTI15_10_IRQHandler(void) {
  EXTI->PR |=  EXTI_PR_PR0 << TX;
  TIMER->CNT = 0;

}
#else
#error error
#endif

// Przerwania od timera


#if TIMNUMBER == 1 || TIMNUMBER == 16
void TIM1_UP_TIM16_IRQHandler( void ) {

	TIM1->SR &= ~(1<<0);
	tx20_batch();
}
#elif TIMNUMBER == 2
void TIM2_IRQHandler( void ) {

	led_flip_led2_botoom();


	TIM2->SR &= ~(1<<0);
	tx20_batch();
}

#elif TIMNUMBER == 3
void TIM3_IRQHandler( void ) {

	led_flip_led2_botoom();


	TIM3->SR &= ~(1<<0);
	tx20_batch();
}

#elif TIMNUMBER == 4
void TIM4_IRQHandler( void ) {

	led_flip_led2_botoom();


	TIM3->SR &= ~(1<<0);
	tx20_batch();
}
#else
#endif

#endif

#endif	// #define _ANEMOMETER_TX20

