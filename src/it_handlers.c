/*
 * it_handlers.c
 *
 *  Created on: 28.05.2017
 *      Author: mateusz
 */

#include <delay.h>
#include <stm32f10x.h>
#include "drivers/dallas.h"
#include "drivers/tx20.h"
#include "drivers/ms5611.h"
#include "drivers/_dht22.h"
#include "drivers/serial.h"
#include "drivers/i2c.h"
#include "aprs/wx.h"
#include "aprs/telemetry.h"
#include "aprs/beacon.h"
#include "main.h"
//#include "afsk.h"
#include "diag/Trace.h"

#include "station_config.h"


/*
 * INTERRUPT PRIORITIES
 *
 *	TIM2_IRQHandler 			- 1 -> Dallas delay (enable only during dallas comm)
 *	I2C1_EV_IRQHandler 			- 2 -> I2C comm interrupt (active & enable only during communication with i2c sensor)
 *	TIM4_IRQHandler 			- 3 -> APRS softmodem DAC (enable only during tx)
 *	TIM7_IRQHandler 			- 4 -> APRS softmodem ADC
 *	SysTick_Handler 			- 5
 *	TIM1_UP_TIM16_IRQHandler 	- 6 -> TX20 baudrate timer
 *	EXTI9_5_IRQHandler 			- 7 -> TX20 anemometer GPIO
 *	EXTI4_IRQHandler 			- 8 -> DHT22 sensor GPIO interrupt
 *	USART1_IRQHandler 			- 9 -> uart to comm with KISS host
 *	I2C1_ER_IRQHandler			- 10 -> I2C error interrupt
 *
 */


// TIM1 w TX20

/* Zmienne używane do oversamplingu */
char adc_sample_count = 0, adc_sample_c2 = 0;				// Zmienna odliczająca próbki
unsigned short int AdcBuffer[4];		// Bufor przechowujący kolejne wartości rejestru DR
short int AdcValue;

// this function will set all iterrupt priorities except systick
void it_handlers_set_priorities(void) {
	NVIC_SetPriority(TIM2_IRQn, 1);
	NVIC_SetPriority(I2C1_EV_IRQn, 2);
	NVIC_SetPriority(TIM4_IRQn, 3);
	NVIC_SetPriority(TIM7_IRQn, 4);
	// systick
	NVIC_SetPriority(TIM1_UP_TIM16_IRQn, 6);
	NVIC_SetPriority(EXTI9_5_IRQn, 7);
	NVIC_SetPriority(EXTI4_IRQn, 8);
	NVIC_SetPriority(USART1_IRQn, 9);
	NVIC_SetPriority(I2C1_ER_IRQn, 10);

}

// Systick interrupt used for time measurements, checking timeouts and SysTick_Handler
void SysTick_Handler(void) {

	// systick interrupt is generated every 10ms
	master_time += SYSTICK_TICKS_PERIOD;

	// decrementing a timer to trigger meteo measuremenets
	main_wx_decremenet_counter();

	srl_keep_timeout();

	i2cKeepTimeout();

	delay_decrement_counter();

}

void USART1_IRQHandler(void) {
	NVIC_ClearPendingIRQ(USART1_IRQn);
	srl_irq_handler();
}

void I2C1_EV_IRQHandler(void) {
	NVIC_ClearPendingIRQ(I2C1_EV_IRQn);

	i2cIrqHandler();

}

void I2C1_ER_IRQHandler(void) {
	i2cErrIrqHandler();
}

void EXTI4_IRQHandler(void) {
  EXTI->PR |= EXTI_PR_PR4;
  dht22_irq_handler();
}

void TIM2_IRQHandler( void ) {
	TIM2->SR &= ~(1<<0);
	if (delay_5us > 0)
		delay_5us--;

}

void TIM4_IRQHandler( void ) {
	// obsluga przerwania cyfra-analog
	TIM4->SR &= ~(1<<0);
	if (timm == 0) {
		DAC->DHR8R1 = AFSK_DAC_ISR(&main_afsk);
		DAC->SWTRIGR |= 1;
	}
	else {
			if (delay_5us > 0)
				delay_5us--;
	}

}

void TIM7_IRQHandler(void) {
// obsluga przetwarzania analog-cyfra. Wersja z oversamplingiem
	TIM7->SR &= ~(1<<0);
	#define ASC adc_sample_count
	#define ASC2 adc_sample_c2
	AdcBuffer[ASC] =  ADC1->DR;
	if(ASC == 3) {
		AdcValue = (short int)(( AdcBuffer[0] + AdcBuffer[1] + AdcBuffer[2] + AdcBuffer[3]) >> 1);
		AFSK_ADC_ISR(&main_afsk, (AdcValue - 4095) );
		if(main_ax25.dcd == true) {		// niebieska dioda
			GPIOC->BSRR |= GPIO_BSRR_BS8;
		}
		else {
			GPIOC->BSRR |= GPIO_BSRR_BR8;
		}
		ASC = 0;

		if (ASC2++ == 2) {
			// pooling AX25 musi być tu bo jak z przerwania wyskoczy nadawanie WX, BCN, TELEM przy dcd == true
			// to bedzie wisialo w nieskonczonosc bo ustawiania dcd jest w srodku ax25_poll
			ax25_poll(&main_ax25);
			ASC2 = 0;
		}
		else {

		}

	}
	else
		ASC++;

}

