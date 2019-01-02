#include "drivers/serial.h"
#include "drivers/gpio_conf.h"

#define PORT USART1

#include <stm32f10x.h>
#include <stm32f10x_usart.h>
#include "station_config.h"
#include "diag/Trace.h"


int srlBRRegValue = 0x09C4 ;		// dla symulacji ---- baudrate 9600bps
//int SrlBRRegValue = 0x0209;		// dla realnego uk��du

#ifdef SEPARATE_TX_BUFF
uint8_t srl_tx_buffer[TX_BUFFER_LN] = {'\0'};		// dane do wys�ania do zdalnego urz�dzenia
#endif

#ifdef SEPARATE_RX_BUFF
uint8_t srl_rx_buffer[RX_BUFFER_LN] = {'\0'};		// dane odebrane od zdalnego urz�dzenia
#endif

uint8_t *srl_tx_buf_pointer;
uint8_t *srl_rx_buf_pointer;

uint16_t srl_rx_buf_ln = 0;
uint16_t srl_tx_buf_ln = 0;

uint16_t srl_rx_bytes_counter = 0;
uint16_t srl_tx_bytes_counter = 0;

uint16_t srl_rx_bytes_req = 0;
uint16_t srl_tx_bytes_req = 0;

uint8_t srl_garbage_storage;

srlState srl_state = SRL_NOT_CONFIG;

uint8_t srl_enable_echo = 0;

//int srlTXQueueLen = 0;
//int srlTRXDataCounter = 0;
//int srlTXing = 0;
//int srlRXing = 0;
//int srlRXBytesNum = 0;			// liczba bajtow do odebrania
char srlRxDummy;				// zmienna pomocnicza do niepotrzebnych danych
uint8_t srlStartChar = 0x00;				// znak oznaczaj�cy pocz�tek istotnych danych do odbebrania
uint8_t srlStopChar = 0x00;				// znak oznaczaj�cy koniec istotnych danych do odebrania
int srlIdle = 1;
char srlEchoOn = 0;
char srlStartStopS;
char srlLenAddr = 0;
char srlLenModif = 0;

void srl_init(void) {
	#ifdef SEPARATE_TX_BUFF
	srl_tx_buf_pointer = srl_tx_buffer;
	srl_rx_buf_ln = RX_BUFFER_LN;
	#endif

	#ifdef SEPARATE_RX_BUFF
	srl_rx_buf_pointer = srl_rx_buffer;
	srl_tx_buf_ln = TX_BUFFER_LN;
	#endif

	Configure_GPIO(GPIOA,11,AFPP_OUTPUT_2MHZ);	// CTS
	Configure_GPIO(GPIOA,12,AFPP_OUTPUT_2MHZ);	// RTS
	Configure_GPIO(GPIOA,8,AFPP_OUTPUT_2MHZ);	// CLK
	Configure_GPIO(GPIOA,10,PUD_INPUT);			// RX
	Configure_GPIO(GPIOA,9,AFPP_OUTPUT_2MHZ);	// TX
	
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;		// wġṗczanie zegara dla USART

	USART_InitTypeDef USART_InitStructure;

	USART_InitStructure.USART_BaudRate = _SERIAL_BAUDRATE;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx;
	USART_Init(PORT, &USART_InitStructure);

	NVIC_EnableIRQ( USART1_IRQn );
	NVIC_SetPriority(USART1_IRQn, 9);		/// bylo 10

 	PORT->CR1 |= USART_CR1_UE;
//	PORT->BRR |= srlBRRegValue;				// ustawianie wartoci preskalera do baudrate
	//srlTXing = 0;
	//srlIdle = 1;
	PORT->SR &= (0xFFFFFFFF ^ USART_SR_TC);
//	PORT->CR1 |= USART_CR1_IDLEIE;			// zgġaszane kiedy przy odbiorze magistrala przejdzie w idle
//	PORT->CR1 |= USART_CR1_RXNEIE;			// przerwanie zgġoszone po odbiorze bajtu gdy bufor nie jest pusty
	srl_state = SRL_IDLE;
}

void srl_send_data(uint8_t* data, uint8_t mode, uint16_t leng, uint8_t internal_external) {
	if (srl_state != SRL_IDLE)
		return;

	/* Wesja z dnia 04.09.2013
	
		char* data - wskaznik na tablice z danymi do przeslania
		char mode - tryb pracy ktory okresla czy ma wysylac okreslona liczbe znakow
					czy wysylac wszystko do napotkania 0x00
		short leng - ilosc znakow do wyslania istotna tylko gdy mode = 1
		 */
	int i;

	if (internal_external == 0) {

		// setting a pointer to transmit buffer to the internal buffer inside the driver
		srl_tx_buf_pointer = srl_tx_buffer;

		// copying the data from provided pointer to internal buffer
		if (mode == 0) {
			// copying everything till the 0x00 is spoted or till the buffer size is reached
			for (i = 0; (i < TX_BUFFER_LN && *(data+i) != '\0'); i++)
				srl_tx_buf_pointer[i]=data[i];
			srl_tx_bytes_req = i;
		}
		else if (mode == 1) {
			for (i = 0; i<=leng ; i++)
				srl_tx_buf_pointer[i]=data[i];
			srl_tx_bytes_req = leng;
		}
	}
	else if (internal_external == 1) {
		srl_tx_buf_pointer = data;
	}
	else return;

		PORT->CR1 |= USART_CR1_TE;
		PORT->SR &= (0xFFFFFFFF ^ USART_SR_TC);
		PORT->DR = srl_tx_buf_pointer[0];
		//srlTXing = 1;
		srl_state = SRL_TXING;
		//srlTRXDataCounter = 0;
//		while ((PORT->SR & USART_SR_TXE) == USART_SR_TXE);
		PORT->CR1 |= USART_CR1_TXEIE;				// przerwanie zg�aszane kiedy rejsetr DR jest pusty
		PORT->CR1 |= USART_CR1_TCIE;				// przerwanie zg�aszane po transmisji bajtu
												// je�eli rejestr DR jest nadal pusty
//	}

}

void srl_start_tx(short leng) {
	if (srlTXing != 1) {
		srlTXQueueLen = leng;
		PORT->CR1 |= USART_CR1_TE;
		PORT->SR &= (0xFFFFFFFF ^ USART_SR_TC);
		PORT->DR = srl_tx_buf_pointer[0];
		srlTXing = 1;
		srlIdle = 0;
		srlTRXDataCounter = 0;
//		while ((PORT->SR & USART_SR_TXE) == USART_SR_TXE);
		PORT->CR1 |= USART_CR1_TXEIE;				// przerwanie zg�aszane kiedy rejsetr DR jest pusty
		PORT->CR1 |= USART_CR1_TCIE;				// przerwanie zg�aszane po transmisji bajtu
												// je�eli rejestr DR jest nadal pusty
	}
}

void srl_receive_data(int num, char start, char stop, char echo, char len_addr, char len_modifier) {
if (srlIdle == 1) {
	trace_printf("Serial:SrlReceiveData()\r\n");

	memset(srl_rx_buf_pointer, 0x00, rx_buf_ln);
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



void srl_irq_handler(void) {
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
			PORT->DR = srl_tx_buf_pointer[srlTRXDataCounter];		// wczytywanie do DR nastêpnego bajtu do transmisji
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
			srl_rx_buf_pointer[srlTRXDataCounter] = data;	// przenoszenie pierwszego odebranego bajtu
			if (srlEchoOn == 1) {
				PORT->CR1 |= USART_CR1_TE;
				PORT->SR &= (0xFFFFFFFF ^ USART_SR_TC);
				PORT->DR = srl_rx_buf_pointer[srlTRXDataCounter];
			}
			if ((srlRXBytesNum == srlTRXDataCounter + 1) || (srl_rx_buf_pointer[srlTRXDataCounter] == srlStopChar && srlStartStopS == 0)) {
				PORT->CR1 &= (0xFFFFFFFF ^ USART_CR1_RE);			// wyġṗczanie odbiornika po odbiorze ostatniego bajtu
				PORT->CR1 &= (0xFFFFFFFF ^ USART_CR1_RXNEIE);
//				PORT->CR1 &= (0xFFFFFFFF ^ USART_CR1_IDLEIE);		// wyġṗczanie przerwañ
				srlRXing = 0;
				srlTRXDataCounter++;
				srl_rx_buf_pointer[srlTRXDataCounter] = '\0';
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
