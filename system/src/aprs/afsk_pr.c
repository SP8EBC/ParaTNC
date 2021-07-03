#include "afsk_pr.h"

#include "station_config_target_hw.h"

#ifdef STM32F10X_MD_VL
#include <stm32f10x.h>
#include "antilib_adc.h"

#endif

#ifdef STM32L471xx
#include <stm32l4xx.h>
#include <stm32l4xx_ll_adc.h>
#endif


void ADCStartConfig(void) {
#ifdef STM32F10X_MD_VL

	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
	ADC1->CR2 |= ADC_CR2_ADON;
	ADC1->CR2 |= ADC_CR2_RSTCAL;       				// Reset calibration
    while(ADC1->CR2 & ADC_CR2_RSTCAL);  			        // Wait for reset
    ADC1->CR2 |= ADC_CR2_CAL;          				// Start calibration
    while(ADC1->CR2 & ADC_CR2_CAL);
	ADC1->SQR1 = ADC_SEQUENCE_LENGTH(0);		// odczyt tylko jednego kana�u
	ADC1->SQR3 =  ADC_SEQ1(11);				// wyb�r kana�u ADC -- 11 - napi�cie zasilania
	ADC1->SMPR1 = ADC_SAMPLE_TIME0(SAMPLE_TIME_7_5);	// czas pr�bkowania
//	ADC1->CR1 = ADC_CR1_EOCIE;			/// przerwanie na zako�czenie konwersji
//	NVIC_EnableIRQ(ADC1_2_IRQn);
//	NVIC_SetPriority(ADC1_2_IRQn, 3);
	ADC1->CR2 |= ADC_CR2_CONT;
	ADC1->CR2 |= ADC_CR2_ADON;
	ADC1->DR;
#endif

#ifdef STM32L471xx

	/**
	 * On STM32L47x/L48x devices, before any conversion of an input channel coming from
		GPIO pads, it is necessary to configure the corresponding GPIOx_ASCR register in the
		GPIO, in addition to the I/O configuration in analog mode.
	 *
	 */

	// reset the clock for ADC
	RCC->AHB2ENR &= (0xFFFFFFFF ^ RCC_AHB2ENR_ADCEN);
	RCC->AHB2ENR |= RCC_AHB2ENR_ADCEN;

	// the adc should be disabled now, but just to be sure that this is a case
	ADC1->CR &= (0xFFFFFFFF ^ ADC_CR_ADEN);

	// exit from deep-power-down mode
	ADC1->CR &= (0xFFFFFFFF ^ ADC_CR_DEEPPWD);

	// start ADC voltage regulator
	ADC1->CR |= ADC_CR_ADVREGEN;

	// start the calibration
	ADC1->CR |= ADC_CR_ADCAL;

	// wait for calibration to finish
    while((ADC1->CR & ADC_CR_ADCAL) == ADC_CR_ADCAL);

    // set the first (and only channel in a conversion sequence)
    ADC1->SQR1 |= (0x4 << 6);

    // set the sampling rate to 12.5 ADC clock cycles
    ADC1->SMPR1 |= 0x2;

    // set continuous conversion
	ADC1->CFGR |= ADC_CFGR_CONT;

    // start ADC
	ADC1->CR |= ADC_CR_ADEN;

	ADC1->DR;

#endif
}

void DACStartConfig(void) {
#ifdef STM32F10X_MD_VL

	RCC->APB1ENR |= RCC_APB1ENR_DACEN;	// wlanczenie zegara
	// rejestr CR przetownirka domyslnie ma same zera
	DAC->CR &= (0xFFFFFFFF ^ DAC_CR_WAVE1);	// bez generowania przebiegu na wyjsciu
	DAC->CR |= DAC_CR_TSEL1;	// programowe wyzwalanie przetwornika (przez flaga swtrig)
	DAC->CR |= DAC_CR_TEN1;
	DAC->CR |= DAC_CR_EN1;
	DAC->DHR8R1 = 10;
	DAC->SWTRIGR |= DAC_SWTRIGR_SWTRIG1;	
#endif

#ifdef STM32L471xx
	// reset the clock for DAC
	RCC->AHB1ENR &= (0xFFFFFFFF ^ RCC_AHB1ENR_DMA1EN);
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
	DAC->CR |= DAC_CR_EN1;
	DAC->SWTRIGR |= DAC_SWTRIGR_SWTRIG1;

#endif

}
