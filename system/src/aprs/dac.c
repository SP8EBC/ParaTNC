
#include "dac.h"
#include <adc.h>

#include "station_config.h"
#include "station_config_target_hw.h"

#ifdef STM32F10X_MD_VL
#include <stm32f10x.h>
#include "antilib_gpio.h"

#endif

#ifdef STM32L471xx
#include <stm32l4xx.h>
#include <stm32l4xx_ll_gpio.h>
#endif

Afsk *dac_afsk;

/*********************************************************************************************************************/
void DA_Init(void) {
/*********************************************************************************************************************/
#ifdef STM32F10X_MD_VL

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
#endif

#ifdef STM32L471xx

	LL_GPIO_InitTypeDef GPIO_InitTypeDef;

	GPIO_InitTypeDef.Pin = LL_GPIO_PIN_4;
	GPIO_InitTypeDef.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitTypeDef.Alternate = LL_GPIO_AF_0;
	GPIO_InitTypeDef.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitTypeDef.Pull = LL_GPIO_PULL_NO;
	GPIO_InitTypeDef.Speed = LL_GPIO_SPEED_FREQ_LOW;
	LL_GPIO_Init(GPIOA, &GPIO_InitTypeDef);		// PTT

	GPIO_InitTypeDef.Pin = LL_GPIO_PIN_5;
	GPIO_InitTypeDef.Mode = LL_GPIO_MODE_ANALOG;
	GPIO_InitTypeDef.Alternate = LL_GPIO_AF_0;
	GPIO_InitTypeDef.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitTypeDef.Pull = LL_GPIO_PULL_NO;
	GPIO_InitTypeDef.Speed = LL_GPIO_SPEED_FREQ_LOW;	// DAC_OUT
	LL_GPIO_Init(GPIOA, &GPIO_InitTypeDef);		// DAC_OUT

	GPIO_InitTypeDef.Pin = LL_GPIO_PIN_1;
	GPIO_InitTypeDef.Mode = LL_GPIO_MODE_ANALOG;
	GPIO_InitTypeDef.Alternate = LL_GPIO_AF_0;
	GPIO_InitTypeDef.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitTypeDef.Pull = LL_GPIO_PULL_NO;
	GPIO_InitTypeDef.Speed = LL_GPIO_SPEED_FREQ_LOW;
	LL_GPIO_Init(GPIOC, &GPIO_InitTypeDef);		// ADC_IN

	LL_GPIO_EnablePinAnalogControl(GPIOC, LL_GPIO_PIN_1);



#endif

}

/*********************************************************************************************************************/
void DA_SetTimer(uint16_t prescaler, uint16_t period) {
/*********************************************************************************************************************/

}

/*********************************************************************************************************************/
void DA_Start() {
/*********************************************************************************************************************/

	AD_Stop();


#ifdef STM32F10X_MD_VL

	GPIOC->BSRR |= GPIO_BSRR_BS3;	 //// sep
#endif

#ifdef STM32L471xx
	GPIOA->BSRR |= GPIO_BSRR_BS4;	 //// sep

#endif

#ifdef STM32F10X_MD_VL

	TIM4->CR1 |= TIM_CR1_CEN;
#else
	TIM5->CR1 |= TIM_CR1_CEN;

#endif

#if (!defined(_METEO))
	GPIO_SetBits(GPIOC, GPIO_Pin_9);
#endif
}

/*********************************************************************************************************************/
void DA_Stop() {
/*********************************************************************************************************************/

	AD_Start();

#ifdef STM32F10X_MD_VL

	GPIOC->BSRR |= GPIO_BSRR_BR3;
#endif

#ifdef STM32L471xx
	GPIOA->BSRR |= GPIO_BSRR_BR4;

#endif

//	//Timer2 DISABLE
#ifdef STM32F10X_MD_VL
	TIM4->CR1 &= ~TIM_CR1_CEN;
#else
	TIM5->CR1 &= ~TIM_CR1_CEN;

#endif

#if (!defined(_METEO))
	GPIO_ResetBits(GPIOC, GPIO_Pin_9);
#endif
}
