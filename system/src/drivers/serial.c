#include "drivers/serial.h"
#include "drivers/gpio_conf.h"

#define PORT USART1

#include <stm32f10x.h>
#include <stm32f10x_usart.h>
#include "station_config.h"
#include "diag/Trace.h"

#include <string.h>

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

uint8_t srl_triggered_start = 0;
uint8_t srl_triggered_stop = 0;

uint8_t srl_start_trigger = 0x00;				// znak oznaczaj�cy pocz�tek istotnych danych do odbebrania
uint8_t srl_stop_trigger = 0x00;				// znak oznaczaj�cy koniec istotnych danych do odebrania

volatile uint8_t srl_garbage_storage;

srlState srl_state = SRL_NOT_CONFIG;

uint8_t srl_enable_echo = 0;

uint8_t srl_rx_lenght_param_addres = 0;
uint8_t srl_rx_lenght_param_modifier = 0;

//char srlLenAddr = 0;
//char srlLenModif = 0;

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

uint8_t srl_send_data(uint8_t* data, uint8_t mode, uint16_t leng, uint8_t internal_external) {
	if (srl_state == SRL_RXING || srl_state == SRL_TXING)
		return SRL_BUSY;

	/* Wesja z dnia 04.09.2013
	
		char* data - wskaznik na tablice z danymi do przeslania
		char mode - tryb pracy ktory okresla czy ma wysylac okreslona liczbe znakow
					czy wysylac wszystko do napotkania 0x00
		short leng - ilosc znakow do wyslania istotna tylko gdy mode = 1
		internal_external - ustawienie 0 spowoduje skopiowanie do wewnentrznego bufora i wysylanie z niego
		jedynka spowoduje wysylanie bezposrednio z wewnetrznego
		 */
	int i;

	// resetting counter
	srl_tx_bytes_counter = 0;

	// if an user want to send data using internal buffer
	if (internal_external == 0) {

		// if data at the input is too long to fit in the buffer
		if (leng >= TX_BUFFER_LN)
			return SRL_DATA_TOO_LONG;

		// setting a pointer to transmit buffer to the internal buffer inside the driver
		srl_tx_buf_pointer = srl_tx_buffer;

		srl_tx_buf_ln = TX_BUFFER_LN;

		// cleaning the buffer from previous content
		memset(srl_tx_buf_pointer, 0x00, TX_BUFFER_LN);

		// copying the data from provided pointer to internal buffer
		if (mode == 0) {
			// copying everything till the 0x00 is spoted or till the buffer border is reached
			for (i = 0; (i < TX_BUFFER_LN && *(data+i) != '\0'); i++)
				srl_tx_buf_pointer[i]=data[i];
			srl_tx_bytes_req = i;
		}
		else if (mode == 1) {
			// we don't need to check against buffer size because this was confirmed already
			for (i = 0; i<=leng ; i++)
				srl_tx_buf_pointer[i]=data[i];
			srl_tx_bytes_req = leng;
		}
	}
	else if (internal_external == 1) {
		srl_tx_buf_pointer = data;
		srl_tx_bytes_req = leng;
		srl_tx_buf_ln = leng;
	}
	else return SRL_WRONG_BUFFER_PARAM;

	// enabling transmitter
	PORT->CR1 |= USART_CR1_TE;
	PORT->SR &= (0xFFFFFFFF ^ USART_SR_TC);
	PORT->DR = srl_tx_buf_pointer[0];
	srl_state = SRL_TXING;
	PORT->CR1 |= USART_CR1_TXEIE;				// przerwanie zg�aszane kiedy rejsetr DR jest pusty
	PORT->CR1 |= USART_CR1_TCIE;				// przerwanie zg�aszane po transmisji bajtu
												// je�eli rejestr DR jest nadal pusty
	return SRL_OK;

}

/**
 * This function assumes than
 */
uint8_t srl_start_tx(short leng) {
	if (srl_state == SRL_RXING || srl_state == SRL_TXING)
		return SRL_BUSY;

	// if data at the input is too long to fit in the buffer
	if (leng >= TX_BUFFER_LN)
		return SRL_DATA_TOO_LONG;

	srl_tx_bytes_req = leng;
	srl_tx_bytes_counter = 0;

	// setting a pointer to transmit buffer to the internal buffer inside the driver
	srl_tx_buf_pointer = srl_tx_buffer;

	PORT->CR1 |= USART_CR1_TE;
	PORT->SR &= (0xFFFFFFFF ^ USART_SR_TC);
	PORT->DR = srl_tx_buf_pointer[srl_tx_bytes_counter];

	srl_state = SRL_TXING;

	PORT->CR1 |= USART_CR1_TXEIE;				// przerwanie zg�aszane kiedy rejsetr DR jest pusty
	PORT->CR1 |= USART_CR1_TCIE;				// przerwanie zg�aszane po transmisji bajtu
												// je�eli rejestr DR jest nadal pusty

	return SRL_OK;
}

uint8_t srl_receive_data(int num, char start, char stop, char echo, char len_addr, char len_modifier) {
	if (srl_state == SRL_RXING || srl_state == SRL_TXING)
		return SRL_BUSY;

	//trace_printf("Serial:SrlReceiveData()\r\n");

	if (num >= RX_BUFFER_LN)
		return SRL_DATA_TOO_LONG;

	memset(srl_rx_buf_pointer, 0x00, RX_BUFFER_LN);

	// checking if user want
	if (start != 0x00) {
		srl_triggered_start = 1;
		srl_start_trigger = start;
	}
	else {
		srl_triggered_start = 0;
	}

	if (stop != 0x00) {
		srl_triggered_stop = 1;
		srl_stop_trigger = stop;
	}
	else {
		srl_triggered_stop = 0;
	}

	if (srl_triggered_start == 1 || srl_triggered_stop == 1) {
		if (num < 3)
			return SRL_WRONG_PARAMS_COMBINATION;
	}

	srl_enable_echo = echo;
	srl_rx_bytes_counter = 0;
	srl_rx_bytes_req = num;

	srl_rx_lenght_param_addres = len_addr;
	srl_rx_lenght_param_modifier = len_modifier;

	PORT->CR1 |= USART_CR1_RE;					// uruchamianie odbiornika
	PORT->CR1 |= USART_CR1_RXNEIE;			// przerwanie od przepe�nionego bufora odbioru
// 	PORT->CR1 |= USART_CR1_IDLEIE;			// przerwanie od bezczynno�ci szyny RS przy odbiorze
												// spowodowanej zako�czeniem transmisji przez urz�dzenie
 	return SRL_OK;
}



void srl_irq_handler(void) {

	// local variable for recalculating a stream length (how many bytes the driver should receives)
	uint16_t len_temp = 0;

	// set to one if there are conditions to stop receiving
	uint8_t stop_rxing = 0;

	// if overrun happened, a byte hadn't been transferred from DR before the next byte is received
	if ((PORT->SR & USART_SR_ORE) == USART_SR_ORE) {
		switch (srl_state) {
			case SRL_RXING:

				break;
			default:
				// if the UART driver is not receiving actually but hardware controler received any data
				// it is required to read value of DR register to clear the interrupt
				srl_garbage_storage = (uint8_t)PORT->DR;
				break;
		}
	}

	// if any data has been received by the UART controller
	if ((PORT->SR & USART_SR_RXNE) == USART_SR_RXNE) {
		switch (srl_state) {
			case SRL_RXING:

				// if there is any data remaining to receive
				if (srl_rx_bytes_counter < srl_rx_bytes_req) {

					// storing received byte into buffer
					srl_rx_buf_pointer[srl_rx_bytes_counter] = (uint8_t)PORT->DR;

					// checking if this byte in stream holds the protocol information about
					// how many bytes needs to be received.
					if (srl_rx_lenght_param_addres == srl_rx_bytes_counter) {
						len_temp = srl_rx_buf_pointer[srl_rx_bytes_counter];

						// adding (or substracting) a length modifier
						len_temp += srl_rx_lenght_param_modifier;

						// if the target length is bigger than buffer size switch to error state
						if (len_temp >= srl_rx_buf_ln) {
							srl_state = SRL_ERROR;
							stop_rxing = 1;
						}
						else {
							srl_rx_bytes_req = len_temp;
						}
					}

					// moving buffer pointer forward
					srl_rx_bytes_counter++;
				}

				// if the user want the driver to stop receiving after certain is received
				if (srl_triggered_stop == 1) {
					if (srl_rx_buf_pointer[srl_rx_bytes_counter - 1] == srl_stop_trigger) {

					}
				}

				// if after incrementing a pointer we reached the end of the buffer
				if (srl_rx_bytes_counter >= srl_rx_bytes_req) {

					// enabling a flag to disble receiver
					stop_rxing = 1;

					// setting a state to receive done
					srl_state = SRL_RX_DONE;
				}

				if (stop_rxing == 1) {
					// disabling UART receiver and its interrupt
					PORT->CR1 &= (0xFFFFFFFF ^ USART_CR1_RE);
					PORT->CR1 &= (0xFFFFFFFF ^ USART_CR1_RXNEIE);
				}

				break;

			// the state when a driver is waiting for start character to appear on serial link
			case SRL_WAITING_TO_RX:

				// storing the value of DR register into local variable to protect against data races
				// which may happened if this IT routine will be preempted by another (long) one
				uint8_t value = (uint8_t)PORT->DR;

				// checking if start character was received
				if (value == srl_start_trigger) {

					// storing received byte in buffer as firts one
					srl_rx_buf_pointer[srl_rx_bytes_counter] = value;

					// increasing the counter value
					srl_rx_bytes_counter++;

					// change state to receiving
					srl_state = SRL_RXING;
				}
				else {
					// if this is not start byte just store it in garbage buffer to clear interrupt condition
					srl_garbage_storage = value;
				}
				break;

			default: break;
		}

	}

	// if one byte was successfully transferred from DR to shift register for transmission over USART
	if ((PORT->SR & USART_SR_TXE) == USART_SR_TXE) {
		switch (srl_state) {
			default: break;
		}
	}


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
