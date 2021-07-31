
#include "adc.h"

#ifdef STM32F10X_MD_VL
#include <stm32f10x.h>
#endif

#ifdef STM32L471xx
#include <stm32l4xx.h>
#include <stm32l4xx_ll_tim.h>
#endif

#include <stdlib.h>
#include <stdio.h>

#define ADC1_DR_Address	((uint32_t)0x4001244C)


Afsk *adc_afsk;

volatile uint16_t ADCValue[16];

uint16_t max_value;
uint32_t samplecount;




/*********************************************************************************************************************/
void AD_Init(Afsk *af) {
/*********************************************************************************************************************/

//	GPIO_InitTypeDef GPIO_InitStructure;
//	NVIC_InitTypeDef NVIC_InitStructure;
//
//	//Dolacz zegar do GPIOA, ADC1
//	RCC->APB2ENR |= RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1;
//	RCC_ADCCLKConfig(RCC_PCLK2_Div6);
//
//	//Dolacz zegar do DMA1
//	RCC->AHBENR |= RCC_AHBPeriph_DMA1;
//
//	//Konfiguracja portu
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 ;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
//	GPIO_Init(GPIOA, &GPIO_InitStructure);
//
//	AD_Reset();
//
//	adc_afsk = af;
//
//	//Przerwania DMA1 Kanal1
//	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//	NVIC_Init(&NVIC_InitStructure);

}


/*********************************************************************************************************************/
void AD_Start() {
/*********************************************************************************************************************/

// 	samplecount = 0;
// 	max_value = 0;

	//Timer3 ENABLE
	TIM2->CR1 |= TIM_CR1_CEN;

}


/*********************************************************************************************************************/
void AD_Stop() {
/*********************************************************************************************************************/

	//Timer3 DISABLE
	TIM2->CR1 &= ~TIM_CR1_CEN;

// 	AD_Reset();

}
