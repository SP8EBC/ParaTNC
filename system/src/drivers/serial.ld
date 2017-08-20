#include "drivers/serial.h"

// funkcja konfiguruj¹ca kontroler portu szeregowego
void SrlConfig(void) {
	Configure_GPIO(GPIOD,3,AFPP_OUTPUT_2MHZ);
	Configure_GPIO(GPIOD,4,AFPP_OUTPUT_2MHZ);
	Configure_GPIO(GPIOD,5,AFPP_OUTPUT_2MHZ);	// konfigurowanie pinow dla USART2
	Configure_GPIO(GPIOD,6,PUD_INPUT);
	Configure_GPIO(GPIOD,7,AFPP_OUTPUT_2MHZ);
	AFIO->MAPR |= AFIO_MAPR_USART2_REMAP;		// remapowanie pinów drugiego kontrolera USART
	NVIC_EnableIRQ( USART2_IRQn );				// w³¹czenie przerwania w kontrolerze NVIC...
	NVIC_SetPriority(USART2_IRQn, 2);			// ...oraz zmiana jego priorytetu
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;		// w³¹czanie zegara dla USART
 	USART2->CR1 |= USART_CR1_UE;				// uruchamianie kontrolera
	USART2->BRR |= SrlBRRegValue;				// ustawianie wartoœci preskalera do baudrate
	SrlTXing = 0;

}

// funkcja odpowiedzialna za zainicjowanie wysy³ania danych przez port szeregowy
void SrlSendData(char* data) {
	int i;
	if (SrlTXing != 1) {
		for (i = 0; (i<32 && *(data+i) != '\0'); i++)
			SrlTXData[i]=data[i];
		SrlTXQueueLen = i;
		USART2->CR1 |= USART_CR1_TE;
		USART2->SR &= (0xFFFFFFFF ^ USART_SR_TC);
		USART2->DR = SrlTXData[0];
		SrlTXing = 1;
		SrlTRXDataCounter = 0;
//		while ((USART2->SR & USART_SR_TXE) == USART_SR_TXE);
		USART2->CR1 |= USART_CR1_TXEIE;				// przerwanie zg³aszane kiedy rejsetr DR jest pusty
		USART2->CR1 |= USART_CR1_TCIE;				// przerwanie zg³aszane po transmisji bajtu
												// je¿eli rejestr DR jest nadal pusty
	}	

}

void SrlSendByte(char byte) {
	int i;
	if (SrlTXing != 1) {
		SrlTXData[0]=byte;
		SrlTXQueueLen = 1;
		USART2->CR1 |= USART_CR1_TE;
		USART2->SR &= (0xFFFFFFFF ^ USART_SR_TC);
		USART2->DR = SrlTXData[0];
		SrlTXing = 1;
		SrlTRXDataCounter = 0;
//		while ((USART2->SR & USART_SR_TXE) == USART_SR_TXE);
		USART2->CR1 |= USART_CR1_TXEIE;				// przerwanie zg³aszane kiedy rejsetr DR jest pusty
		USART2->CR1 |= USART_CR1_TCIE;				// przerwanie zg³aszane po transmisji bajtu
												// je¿eli rejestr DR jest nadal pusty
	}	

}

void SrlReceiveData(char* data, int num) {
	SrlRXBytesNum = num;
	SrlTRXDataCounter = -1;
	USART2->CR1 |= USART_CR1_RE;					// uruchamianie odbiornika
	SrlRXing = 1;
//	SrlTRXDataCounter = 0;
 	USART2->CR1 |= USART_CR1_RXNEIE;			// przerwanie od przepe³nionego bufora odbioru
// 	USART2->CR1 |= USART_CR1_IDLEIE;			// przerwanie od bezczynnoœci szyny RS przy odbiorze
												// spowodowanej zakoñczeniem transmisji przez urz¹dzenie
}

void USART2_IRQHandler(void) {
NVIC_ClearPendingIRQ(USART2_IRQn);
	if ((USART2->SR & USART_SR_TXE) == USART_SR_TXE && SrlTXing == 1) {
		if(SrlTXQueueLen == 1 || SrlTRXDataCounter + 1 == SrlTXQueueLen) {
			while((USART2->SR & USART_SR_TC) != USART_SR_TC);
			USART2->CR1 &= (0xFFFFFFFF ^ USART_CR1_TE);		//wy³¹czanie nadajnika portu szeregowego
			SrlTXing = 0;
			USART2->CR1 &= (0xFFFFFFFF ^ USART_CR1_TXEIE);	
			USART2->CR1 &= (0xFFFFFFFF ^ USART_CR1_TCIE);	// wy³¹czanie przerwañ od portu szeregowego
			SrlTRXDataCounter = 0;
		}
		else {
			SrlTRXDataCounter++;
			USART2->DR = SrlTXData[SrlTRXDataCounter];		// wczytywanie do DR nastêpnego bajtu do transmisji
		}
	}
	if ((USART2->SR & USART_SR_RXNE) == USART_SR_RXNE && SrlRXing == 1) {
		SrlTRXDataCounter++;		
		SrlRXData[SrlTRXDataCounter] = (USART2->DR & 0x01FF);	// przenoszenie pierwszego odebranego bajtu
		if (SrlRXBytesNum == SrlTRXDataCounter + 1) {
			USART2->CR1 &= (0xFFFFFFFF ^ USART_CR1_RE);			// wy³¹czanie odbiornika po odbiorze ostatniego bajtu
			USART2->CR1 &= (0xFFFFFFFF ^ USART_CR1_RXNEIE);
//			USART2->CR1 &= (0xFFFFFFFF ^ USART_CR1_IDLEIE);		// wy³¹czanie przerwañ
			SrlRXing = 0;
			SrlTRXDataCounter++;
			SrlRXData[SrlTRXDataCounter] = '\0';
			SrlTRXDataCounter = 0;											
		}	

	}
	if ((USART2->SR & USART_SR_IDLE) == USART_SR_IDLE && SrlRXing == 1) {

	}
}
