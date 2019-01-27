
#include "dac.h"

#include <adc.h>

#include "station_config.h"

Afsk *dac_afsk;

///*********************************************************************************************************************/
//void TIM2_IRQHandler(void) {
///*********************************************************************************************************************/
//
////	TIM2->SR = ~TIM_IT_Update;
////
////	DAC->DHR8R1 = AFSK_DAC_ISR(dac_afsk);
////	DAC->SWTRIGR |= 1;
//
//}

/*********************************************************************************************************************/
void DA_Init(void) {
/*********************************************************************************************************************/

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
#ifndef _PTT_PUSHPULL
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
#else
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
#endif
	GPIO_Init(GPIOC, &GPIO_InitStructure);

#ifndef _PTT_PUSHPULL
	GPIOC->BSRR |= GPIO_BSRR_BS3; //// bez sep
#else
	GPIOC->BSRR |= GPIO_BSRR_BR3; //// bez sep
#endif

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_Init(GPIOA, &GPIO_InitStructure);


}

/*********************************************************************************************************************/
void DA_SetTimer(uint16_t prescaler, uint16_t period) {
/*********************************************************************************************************************/

}

/*********************************************************************************************************************/
void DA_Start() {
/*********************************************************************************************************************/

	AD_Stop();
//
//	//stm32_gpioPinWrite(PTT_GPIO_BASE, PTT_PIN, 1);
//	GPIO_SetBits(GPIOB, GPIO_Pin_11);

#ifndef _PTT_PUSHPULL
	GPIOC->BSRR |= GPIO_BSRR_BR3;	 /// bez sep
#else
	GPIOC->BSRR |= GPIO_BSRR_BS3;	 //// sep
#endif
	//	GPIOC->BSRR |= GPIO_BSRR_BS8;
//
//	//Timer2 ENABLE
	TIM4->CR1 |= TIM_CR1_CEN;


#if (!defined(_METEO))
	GPIO_SetBits(GPIOC, GPIO_Pin_9);
#endif
}

/*********************************************************************************************************************/
void DA_Stop() {
/*********************************************************************************************************************/

	AD_Start();

#ifndef _PTT_PUSHPULL
	GPIOC->BSRR |= GPIO_BSRR_BS3; //// bez sep
#else
	GPIOC->BSRR |= GPIO_BSRR_BR3;
#endif
//	GPIOC->BSRR |= GPIO_BSRR_BR8;

//	//Timer2 DISABLE
	TIM4->CR1 &= ~TIM_CR1_CEN;

#if (!defined(_METEO))
	GPIO_ResetBits(GPIOC, GPIO_Pin_9);
#endif
}
