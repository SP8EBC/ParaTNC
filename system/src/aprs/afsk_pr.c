#include "afsk_pr.h"
#include <stm32f10x.h>
#include "antilib_adc.h"

/*
#define N 8
static int16_t data[N];
static int16_t coeffloi[N]; 
static int16_t coeffloq[N]; 
static int16_t coeffhii[N]; 
static int16_t coeffhiq[N];
static int16_t ptr=0;
static uint8_t niesynchro_bity;	// zdekodowane przez filtr bity.. Niesynchroniczne do pr�dko�ci bodowej
static uint8_t synchro_bity;	// bity zsynchronizowane do pr�dko�ci bodowej
unsigned char nrzi_bity;	// bity zsynchronizowane i po dekodowaniu NRZI
static uint16_t destuff_bity; // bity po destuffingu. Zawierajace wlasciwe dane ramki AX25
static int8_t bit_faza;			// aktualna "faza" bitow
static uint8_t bit_counter = 0;
static uint8_t DCD = 0;	 // data carrier detect
static uint8_t PacketByteCounter = 0;	// licznik odebranych bajt�w z jednej ramki

struct Frame {
	char RAWContent[80];
};

struct Frame Frame;
*/
void ADCStartConfig(void) {
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

}

void DACStartConfig(void) {
	RCC->APB1ENR |= RCC_APB1ENR_DACEN;	// wlanczenie zegara
	// rejestr CR przetownirka domyslnie ma same zera
	DAC->CR &= (0xFFFFFFFF ^ DAC_CR_WAVE1);	// bez generowania przebiegu na wyjsciu
	DAC->CR |= DAC_CR_TSEL1;	// programowe wyzwalanie przetwornika (przez flaga swtrig)
	DAC->CR |= DAC_CR_TEN1;
	DAC->CR |= DAC_CR_EN1;
	DAC->DHR8R1 = 10;
	DAC->SWTRIGR |= DAC_SWTRIGR_SWTRIG1;	
	
}
