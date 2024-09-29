
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

Afsk *adc_afsk;

uint16_t max_value;
uint32_t samplecount;

/*********************************************************************************************************************/
void AD_Restart(void) {
/*********************************************************************************************************************/
#ifdef STM32L471xx
	// check if conversion is done or not
	if ((ADC1->ISR & ADC_ISR_EOC) == 0) {
		// stop adc conversion
		ADC1->CR |= ADC_CR_ADSTP;

		// wait for conversion to shutdown
	    while((ADC1->ISR & ADC_CR_ADSTART) == ADC_CR_ADSTART);

	    // start ADC back again
		ADC1->CR |= ADC_CR_ADSTART;
	}
#endif
}


/*********************************************************************************************************************/
void AD_Start() {
/*********************************************************************************************************************/

// 	samplecount = 0;
// 	max_value = 0;

	//Timer3 ENABLE
	//TIM2->CR1 |= TIM_CR1_CEN;

}


/*********************************************************************************************************************/
void AD_Stop() {
/*********************************************************************************************************************/

	//Timer3 DISABLE
	//TIM2->CR1 &= ~TIM_CR1_CEN;

// 	AD_Reset();

}
