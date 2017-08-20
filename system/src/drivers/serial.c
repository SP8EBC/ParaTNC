#include "drivers/serial.h"
#include "drivers/gpio_conf.h"

#define PORT USART1

#include <stm32f10x.h>
#include <stm32f10x_usart.h>

#include "diag/Trace.h"


int srlBRRegValue = 0x09C4 ;		// dla symulacji ---- baudrate 9600bps
//int SrlBRRegValue = 0x0209;		// dla realnego uk��du

uint8_t srlTXData[128] = {'\0'};		// dane do wys�ania do zdalnego urz�dzenia
uint8_t srlRXData[128] = {'\0'};		// dane odebrane od zdalnego urz�dzenia
int srlTXQueueLen = 0;
int srlTRXDataCounter = 0;
int srlTXing = 0;
int srlRXing = 0;
int srlRXBytesNum = 0;			// liczba bajtow do odebrania
char srlRxDummy;				// zmienna pomocnicza do niepotrzebnych danych
uint8_t srlStartChar = 0x00;				// znak oznaczaj�cy pocz�tek istotnych danych do odbebrania
uint8_t srlStopChar = 0x00;				// znak oznaczaj�cy koniec istotnych danych do odebrania
int srlIdle = 1;
char srlEchoOn = 0;
char srlStartStopS;
char srlLenAddr = 0;
char srlLenModif = 0;

void SrlConfig(void) {
	Configure_GPIO(GPIOA,11,AFPP_OUTPUT_2MHZ);	// CTS
	Configure_GPIO(GPIOA,12,AFPP_OUTPUT_2MHZ);	// RTS
	Configure_GPIO(GPIOA,8,AFPP_OUTPUT_2MHZ);	// CLK
	Configure_GPIO(GPIOA,10,PUD_INPUT);			// RX
	Configure_GPIO(GPIOA,9,AFPP_OUTPUT_2MHZ);	// TX
	
	Configure_GPIO(GPIOB,6,AFPP_OUTPUT_2MHZ);	// TX-remap
	Configure_GPIO(GPIOB,7,PUD_INPUT);			// RX-remap
//	AFIO->MAPR |= AFIO_MAPR_USART2_REMAP;
	NVIC_EnableIRQ( USART1_IRQn );
	NVIC_SetPriority(USART1_IRQn, 9);		/// bylo 10
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;		// wġṗczanie zegara dla USART
 	PORT->CR1 |= USART_CR1_UE;
	PORT->BRR |= srlBRRegValue;				// ustawianie wartoci preskalera do baudrate
	srlTXing = 0;
	srlIdle = 1;
	PORT->SR &= (0xFFFFFFFF ^ USART_SR_TC);
//	PORT->CR1 |= USART_CR1_IDLEIE;			// zgġaszane kiedy przy odbiorze magistrala przejdzie w idle
//	PORT->CR1 |= USART_CR1_RXNEIE;			// przerwanie zgġoszone po odbiorze bajtu gdy bufor nie jest pusty

}

void SrlSendData(char* data, char mode, short leng) {
	/* Wesja z dnia 04.09.2013
	
		char* data - wskaznik na tablice z danymi do przeslania
		char mode - tryb pracy ktory okresla czy ma wysylac okreslona liczbe znakow
					czy wysylac wszystko do napotkania 0x00
		short leng - ilosc znakow do wyslania istotna tylko gdy mode = 1
		 */
	int i;
	if (srlTXing != 1) {
		if (mode == 0) { 
			for (i = 0; (i<128 && *(data+i) != '\0'); i++)
				srlTXData[i]=data[i];
			srlTXQueueLen = i;
		}
		else if (mode == 1) {
			for (i = 0; i<=leng ; i++)
				srlTXData[i]=data[i];
			srlTXQueueLen = leng;
		}
		PORT->CR1 |= USART_CR1_TE;
		PORT->SR &= (0xFFFFFFFF ^ USART_SR_TC);
		PORT->DR = srlTXData[0];
		srlTXing = 1;
		srlTRXDataCounter = 0;
//		while ((PORT->SR & USART_SR_TXE) == USART_SR_TXE);
		PORT->CR1 |= USART_CR1_TXEIE;				// przerwanie zg�aszane kiedy rejsetr DR jest pusty
		PORT->CR1 |= USART_CR1_TCIE;				// przerwanie zg�aszane po transmisji bajtu
												// je�eli rejestr DR jest nadal pusty
	}	

}

void SrlStartTX(short leng) {
	if (srlTXing != 1) {
		srlTXQueueLen = leng;
		PORT->CR1 |= USART_CR1_TE;
		PORT->SR &= (0xFFFFFFFF ^ USART_SR_TC);
		PORT->DR = srlTXData[0];
		srlTXing = 1;
		srlIdle = 0;
		srlTRXDataCounter = 0;
//		while ((PORT->SR & USART_SR_TXE) == USART_SR_TXE);
		PORT->CR1 |= USART_CR1_TXEIE;				// przerwanie zg�aszane kiedy rejsetr DR jest pusty
		PORT->CR1 |= USART_CR1_TCIE;				// przerwanie zg�aszane po transmisji bajtu
												// je�eli rejestr DR jest nadal pusty
	}
}

void SrlReceiveData(int num, char start, char stop, char echo, char len_addr, char len_modifier) {
if (srlIdle == 1) {
	trace_printf("Serial:SrlReceiveData()\r\n");

	memset(srlRXData, 0x00, 128);
	srlRXBytesNum = num;
	srlStartChar = start;
	srlStopChar = stop;
	srlTRXDataCounter = -1;
	PORT->CR1 |= USART_CR1_RE;					// uruchamianie odbiornika
	if (srlStartChar != 0x00 || srlStopChar != 0x00)
		srlRXing = 0;
	else
		srlRXing = 1;
	srlIdle = 0;
	srlEchoOn = echo;
	srlTRXDataCounter = 0;
	srlLenAddr = len_addr;
	srlLenModif = len_modifier;
 	PORT->CR1 |= USART_CR1_RXNEIE;			// przerwanie od przepe�nionego bufora odbioru
// 	PORT->CR1 |= USART_CR1_IDLEIE;			// przerwanie od bezczynno�ci szyny RS przy odbiorze
												// spowodowanej zako�czeniem transmisji przez urz�dzenie
}
}

void USART1_IRQHandler(void) {
	if ((PORT->SR & USART_SR_TXE) == USART_SR_TXE && srlTXing == 1) {
		if(srlTXQueueLen == 1 || srlTRXDataCounter + 1 == srlTXQueueLen) {
			while((PORT->SR & USART_SR_TC) != USART_SR_TC);
			PORT->CR1 &= (0xFFFFFFFF ^ USART_CR1_TE);		//wyġṗczanie nadajnika portu szeregowego
			srlTXing = 0;
			srlIdle = 1;
			PORT->CR1 &= (0xFFFFFFFF ^ USART_CR1_TXEIE);
			PORT->CR1 &= (0xFFFFFFFF ^ USART_CR1_TCIE);	// wyġṗczanie przerwañ od portu szeregowego
			srlTRXDataCounter = 0;
			PORT->SR &= (0xFFFFFFFF ^ USART_SR_TC);
		}
		else {
			srlTRXDataCounter++;
			PORT->DR = srlTXData[srlTRXDataCounter];		// wczytywanie do DR nastêpnego bajtu do transmisji
		}
	}
	if ((PORT->SR & USART_SR_RXNE) == USART_SR_RXNE && srlIdle == 0) {
		uint8_t data = ((uint8_t)PORT->DR & 0xFF);
		if ((data == srlStartChar) && srlRXing == 0)
			// Jeṡeli zlecono odbiór danych sprawdza czy odebrany bajt jest pierwszym bajtem znaczṗcym
			srlRXing = 1, srlStartStopS = 1;			// Jeṡeli zostaġ odebrany to ustaw flagê odbioru ṡeby umoṡliwiæ kopiowanie do bufora
		else if (srlRXing == 0) {
			srlRxDummy = data;		// jeṡeli bajt nie byġ znaczṗcy to go usuñ
			if (srlEchoOn == 1) {
				PORT->CR1 |= USART_CR1_TE;
				PORT->SR &= (0xFFFFFFFF ^ USART_SR_TC);
				PORT->DR = srlRxDummy;
			}	
			else {

			}
		}
		else {

		}
		if (srlRXing == 1) {
			srlRXData[srlTRXDataCounter] = data;	// przenoszenie pierwszego odebranego bajtu
			if (srlEchoOn == 1) {
				PORT->CR1 |= USART_CR1_TE;
				PORT->SR &= (0xFFFFFFFF ^ USART_SR_TC);
				PORT->DR = srlRXData[srlTRXDataCounter];
			}
			if ((srlRXBytesNum == srlTRXDataCounter + 1) || (srlRXData[srlTRXDataCounter] == srlStopChar && srlStartStopS == 0)) {
				PORT->CR1 &= (0xFFFFFFFF ^ USART_CR1_RE);			// wyġṗczanie odbiornika po odbiorze ostatniego bajtu
				PORT->CR1 &= (0xFFFFFFFF ^ USART_CR1_RXNEIE);
//				PORT->CR1 &= (0xFFFFFFFF ^ USART_CR1_IDLEIE);		// wyġṗczanie przerwañ
				srlRXing = 0;
				srlTRXDataCounter++;
				srlRXData[srlTRXDataCounter] = '\0';
				srlTRXDataCounter = 0;
				srlIdle = 1;
			}
			srlTRXDataCounter++;
		}
		else {

		}
		srlStartStopS = 0;
	}
	if ((PORT->SR & USART_SR_IDLE) == USART_SR_IDLE && srlRXing == 1) {

	}
}
