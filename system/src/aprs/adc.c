
#include "adc.h"

#include <stdlib.h>
#include <stdio.h>

#define ADC1_DR_Address	((uint32_t)0x4001244C)


Afsk *adc_afsk;

volatile uint16_t ADCValue[16];

uint16_t max_value;
uint32_t samplecount;

/*********************************************************************************************************************/
void DMA1_Channel1_IRQHandler(void) {
/*********************************************************************************************************************/
	uint16_t sample;
	//DMA_ClearITPendingBit(DMA_IT_TC);
	DMA1->IFCR = DMA_IFCR_CTCIF1;

/*
	uint16_t sample = ( ADCValue[0] + ADCValue[1] + ADCValue[2] + ADCValue[3] +
						ADCValue[4] + ADCValue[5] + ADCValue[6] + ADCValue[7] +
						ADCValue[8] + ADCValue[9] + ADCValue[10] + ADCValue[11] +
						ADCValue[12] + ADCValue[13] + ADCValue[14] + ADCValue[15]
						) >> 2;
*/


	sample = (( ADCValue[0] + ADCValue[1] + ADCValue[2] + ADCValue[3]) >> 1);

	/*
	samplecount++;
	if (samplecount > 30000)
	{
		//printf("Max Value = %d \r\n", max_value);
		samplecount = 0;
		max_value = 0;

	}

	if (abs(sample - 4095) > max_value) max_value = abs(sample - 4095);
*/

	//Wyslanie probki do demodulatora
	//AFSK_ADC_ISR(adc_afsk, (int16_t)(sample - 8191));

	AFSK_ADC_ISR(adc_afsk, (int16_t)(sample - 4095));

}


/*********************************************************************************************************************/
void AD_SetTimer(uint16_t period, uint16_t prescaler) {
/*********************************************************************************************************************/

//	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	//wlacznie sygnalu zegarowego Timer3
	RCC->APB1ENR |= RCC_APB1Periph_TIM3;

	//DeInit Timer3
	RCC->APB1RSTR |= RCC_APB1Periph_TIM3;
	RCC->APB1RSTR &= ~RCC_APB1Periph_TIM3;


	/*
	TIM_TimeBaseStructure.TIM_Period = period - 1;
	TIM_TimeBaseStructure.TIM_Prescaler = prescaler - 1;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0 ;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	TIM_SelectOutputTrigger(TIM3, TIM_TRGOSource_Update);
	*/

	/*
	 * TIMER3
	 * - Clock Div = 0
	 * - Counter Mode = UP
	 * - Repetition Counter = 0
	 * - Output Trigger = TRGO
	 */

	TIM3->ARR = period - 1;
	TIM3->PSC = prescaler - 1;
	TIM3->RCR = 0;
	TIM3->CR2 = TIM_CR2_MMS_1;

}



/*********************************************************************************************************************/
static void AD_Reset() {
/*********************************************************************************************************************/

//	ADC_InitTypeDef ADC_InitStructure;
//	DMA_InitTypeDef DMA_InitStructure;


	//Konfiguracja DMA
	//Umieszcze probki w ADCValue i po zebraniu probek wyzwala przerwanie
	//Aktualnie 16 probki (oversampling 2 bit)

	/*
	DMA_DeInit(DMA1_Channel1);

	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ADC1_DR_Address; //&(ADC1->DR);
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&ADCValue;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
//	DMA_InitStructure.DMA_BufferSize = 16;
	DMA_InitStructure.DMA_BufferSize = 4;

	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

	DMA_Init(DMA1_Channel1, &DMA_InitStructure);

//	DMA_ClearITPendingBit(DMA_IT_TC);

//	DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);

//	DMA_Cmd(DMA1_Channel1, ENABLE);
	 */

	/*
	 * DMA1 Channel1
	 * - ADC1 -> MEMORY
	 * - Memory Increment
	 * - DMA Buffer Size = 4
	 * - Peripheral Data Size = HalfWord
	 * - Peripheral Data Size = HalfWord
	 * - DMA Mode = Circular
	 * - DMA Priority = High
	 * - DMA M2M Disable
	 * - IFCR = DMA_IT_TC
	 */

	DMA1_Channel1->CCR &= ~DMA_CCR1_EN;

	DMA1->IFCR = DMA_IFCR_CTCIF1;

	DMA1_Channel1->CNDTR = 4;
	DMA1_Channel1->CPAR = (uint32_t)ADC1_DR_Address; //&(ADC1->DR);
	DMA1_Channel1->CMAR =  (uint32_t)&ADCValue;
	DMA1_Channel1->CCR |= DMA_CCR1_EN | DMA_CCR1_CIRC | DMA_CCR1_MINC | DMA_CCR1_PSIZE_0 | DMA_CCR1_MSIZE_0 | DMA_CCR1_PL_1 | DMA_IT_TC;



	/*
	//struktura inicjujaca
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T3_TRGO;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	ADC_Init(ADC1, &ADC_InitStructure);


//	ADC_ExternalTrigConvCmd(ADC1, ENABLE);

//	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_1Cycles5) ;

	//Wlasz DMA dla ADC1
	//ADC_DMACmd(ADC1, ENABLE);

	// ADC1 ENABLE
	//ADC_Cmd(ADC1, ENABLE);


	*/


	//DeInit ADC1
	RCC->APB2RSTR |= RCC_APB2Periph_ADC1;
	RCC->APB2RSTR &= ~RCC_APB2Periph_ADC1;

	/*
	 * ADC1
	 * - ADC Mode = Independent
	 * - Scan Mode = DISABLE
	 * - Continuous Mode = DISABLE
	 * - External Trig = T3 TRGO
	 * - NbrOfChannels = 1
	 * - ADC Channel = 0
	 * - ADC Sample Time = 1c5
	 * - DMA = ENABLE
	 */

	ADC1->CR1 = 0x00;
	ADC1->CR2 |= ADC_CR2_EXTTRIG | ADC_CR2_EXTSEL_2;
	ADC1->SQR1 = 0x00;
	ADC1->SMPR1 = 0x00;

	ADC1->CR2 |= ADC_CR2_ADON | ADC_CR2_DMA ;

	//Kalibracja
	ADC1->CR2 |= ADC_CR2_RSTCAL;
	while (ADC1->CR2 & ADC_CR2_RSTCAL);

	ADC1->CR2 |= ADC_CR2_CAL;
	while (ADC1->CR2 & ADC_CR2_CAL);

}


/*********************************************************************************************************************/
void AD_Init(Afsk *af) {
/*********************************************************************************************************************/

	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	//Dolacz zegar do GPIOA, ADC1
	RCC->APB2ENR |= RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1;
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);

	//Dolacz zegar do DMA1
	RCC->AHBENR |= RCC_AHBPeriph_DMA1;

	//Konfiguracja portu
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 ;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	AD_Reset();

	adc_afsk = af;

	//Przerwania DMA1 Kanal1
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

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
