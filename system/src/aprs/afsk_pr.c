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
/*
int ADCSendViaSerial(unsigned short int adc_val) {
	int i;
	char temp[10];					// pomiary
	char* endl = ";\n\r";			// pomiary
	adc_val &= 0xFFFF;
	for (i=0 ; i<10 ; i++)
		*(temp+i) = 0x00; 
	int2string(adc_val, temp);
	strcpy(temp+5,endl);			
	FixString(temp,8);				
	SrlSendData(temp);
	return 0;				
}

void InitFilter(void) {
	int i;
	for(i=0;i<N;i++) {
		coeffloi[i]=2047*cos(2*3.1415*i/N);
		coeffloq[i]=2047*sin(2*3.1415*i/N);
		coeffhii[i]=2047*cos(2*3.1415*i/N*2200/1200);
		coeffhiq[i]=2047*sin(2*3.1415*i/N*2200/1200);
	}
}

int SampleData(short int input) {
	int16_t i,d;
	int32_t outloi=0,outloq=0,outhii=0,outhiq=0,out=0;
	data[ptr]=input; ptr = (ptr+1)%N;
	for(i=0;i<N;i++) {
		d = data[(ptr+i)%N];
		outloi += d*coeffloi[i];
		outloq += d*coeffloq[i];
		outhii += d*coeffhii[i];
		outhiq += d*coeffhiq[i];
	}
	out = (outhii>>11)*(outhii>>11)+(outhiq>>11)*(outhiq>>11)
	-(outloi>>11)*(outloi>>11)-(outloq>>11)*(outloq>>11);		// przesow o 11 bitow bo przetwornik ma 12 bitow
	return (out);
}

int SynchroNRZI(unsigned char bit) {
	unsigned char temp; 
	niesynchro_bity <<= 1;		// przesuwanie zawartosci bitow niezsynchronizowanych zeby zrobic miejsce na nowy
	niesynchro_bity |= bit;	    // dodawanie do zmiennej nastepnego zsamplowanego bitu (bodu)
	if ((niesynchro_bity & 0x03) == 0x01 || (niesynchro_bity & 0x03) == 0x02) {
		// sprawdza czy nie zmienil sie przesylany bit (zmiana tonu)
		if (bit_faza <= 32 )
			// korekcja "fazy" dekodera
			bit_faza += 1;
		else 
			bit_faza -= 1;
	}
	bit_faza += 8;		// zwiekszanie "fazy" o 8 po kazdym niezsynchronizowanym bicie.
						// Ka�dy z nich jest samplowany osiem razy
	if (bit_faza >= 64){
		// jezeli faza przekroczyla 64, czyli przetworzono 8 niezsynchronizowanych bitow.
	   	// Ka�dy pojedynczy bit (a w zasadzie bod) jest samplowany 8 razy.
		bit_faza %= 64; // powracanie z "faz�" na poczatek. W przypadku gdy po ostatnim zwiekszeniu licznika
						// wyszlo powyzej 64 to nalezy wrocic do wartosci rownej (bit_faza - 64) tak aby nie
						// gubic synchronizacji
		synchro_bity <<= 1;		// przesuwanie zsynchronizowanych do predkosci bodowej bitow o jeden
		nrzi_bity <<= 1;
	
		temp = niesynchro_bity & 0x07;	// ostatnie trzy samplowane bity
		if(temp == 0x07 || temp == 0x06 || temp == 0x05 || temp == 0x03)
			// jezeli z trzech ostatnich niesynchro bitow, dwa maja wartosc jeden
			// to mozna uznac ze odebrano jedynke
			synchro_bity |= 1;
		else;	// jezeli nie to zero. 
		// BITY SA KODOWANE PRZY UZYCIU NRZI!!!!!!! Ponizej dekodowanie
		if ((synchro_bity & 0x03) == 0x03 || (synchro_bity & 0x03) == 0x00)
			nrzi_bity |= 1;		// jezeli dwa kolejne bity sa takie same to jeden
		else;					// jezeli sa rozne to zero
//		if (nrzi_bity_c != 8)
//			nrzi_bity_c++;		// jezeli nie odebrano jeszcze calego bajtu
//		else
//			nrzi_bity_c = 0;		// w momencie odebrania calego bajtu zresetuj licznik
		if (nrzi_bity == 0x7E) {
//			 ADCSendViaSerial(7);
			 DCD = ~DCD;
		}
		else;
//		ADCSendViaSerial(synchro_bity);
		DeStuffing(nrzi_bity & 0x01); 
		return nrzi_bity;		// zwracanie odkodowanych z NRZI bitow
	}
	return -2;		
}

/// 0x7E flaga naglowka

int DeStuffing(uint8_t bit) {
	// funkcja robi destuffing CIAGU BITOW. Przyjmuje wprawdzie 8 bitow ze zmiennej nrzi_bity
	// czyli niby jeden bajt, ale do kolejnej pozycji lini opozniajacej dolancza tylko ten najmniej znaczacy (najnowszy)
	if ( DCD > 0 ) {
		destuff_bity <<= 1;
		ADCSendViaSerial(destuff_bity);
		destuff_bity |= bit;
		if ((destuff_bity & 0x3f) == 0x3e) {
			destuff_bity >>=1;
			bit_counter--;
		}
		else;
		bit_counter++;
		if (bit_counter == 8 && PacketByteCounter < 70) { 
			bit_counter = 0;
			Frame.RAWContent[PacketByteCounter] = (destuff_bity & 0xFF);
			PacketByteCounter++;
//			destuff_bity = 0;	
		}
 		if (PacketByteCounter == 70) {
			PacketByteCounter = 0;
			if (DCD == 0xFF)
				DCD = 0;
		}
	else;
	}
}

*/
