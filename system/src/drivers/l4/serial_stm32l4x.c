#include "drivers/serial.h"

#include "station_config.h"
#include "station_config_target_hw.h"

#ifdef PARAMETEO
	// those defines and an undef are only required for shitty Eclipse indexer to see anything from STM32L471xx target
	#define STM32L471xx
	#define USE_FULL_LL_DRIVER
	#undef STM32F10X_MD_VL
#endif

//#include <drivers/f1/gpio_conf_stm32f1x.h>
#include "diag/Trace.h"

#include "main.h" 	// global_time is here

#include <string.h>

#include <stm32l4xx.h>
#include <stm32l4xx_ll_gpio.h>
#include <stm32l4xx_ll_usart.h>

#ifdef SEPARATE_TX_BUFF
uint8_t srl_usart1_tx_buffer[TX_BUFFER_1_LN] = {'\0'};
#endif

#ifdef SEPARATE_RX_BUFF
uint8_t srl_usart1_rx_buffer[RX_BUFFER_1_LN] = {'\0'};
#endif

#ifdef SEPARATE_TX_BUFF
uint8_t srl_usart2_tx_buffer[TX_BUFFER_2_LN] = {'\0'};
#endif

#ifdef SEPARATE_RX_BUFF
uint8_t srl_usart2_rx_buffer[RX_BUFFER_2_LN] = {'\0'};
#endif


void srl_init(
			srl_context_t *ctx,
			USART_TypeDef *port,
			uint8_t *rx_buffer,
			uint16_t rx_buffer_size,
			uint8_t *tx_buffer,
			uint16_t tx_buffer_size,
			uint32_t baudrate,
			uint8_t stop_bits
			) {

	if (ctx->srl_rx_state == SRL_RX_IDLE)
		return;

	#ifdef SEPARATE_TX_BUFF
	ctx->srl_tx_buf_pointer = tx_buffer;
	ctx->srl_rx_buf_ln = tx_buffer_size;
	#endif

	#ifdef SEPARATE_RX_BUFF
	ctx->srl_rx_buf_pointer = rx_buffer;
	ctx->srl_tx_buf_ln = rx_buffer_size;
	#endif

	memset(ctx->srl_rx_buf_pointer, 0x00, ctx->srl_rx_buf_ln);
	memset(ctx->srl_tx_buf_pointer, 0x00, ctx->srl_tx_buf_ln);

	ctx->port = port;
	ctx->port_baurate = baudrate;
	ctx->port_stopbits = stop_bits;

	//ctx->te_port = 0;
	//ctx->te_pin = 0;

	ctx->srl_tx_start_time = 0xFFFFFFFFu;

	LL_USART_InitTypeDef USART_InitStructure;

	USART_InitStructure.BaudRate = baudrate;
	USART_InitStructure.DataWidth = LL_USART_DATAWIDTH_8B;
	USART_InitStructure.StopBits = LL_USART_STOPBITS_1;
	USART_InitStructure.Parity = LL_USART_PARITY_NONE;
	USART_InitStructure.TransferDirection = LL_USART_DIRECTION_RX;
	USART_InitStructure.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
	USART_InitStructure.OverSampling = LL_USART_OVERSAMPLING_16;


//	USART_InitStructure.USART_BaudRate = baudrate;
//	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
//	USART_InitStructure.USART_Parity = USART_Parity_No;
//	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
//	USART_InitStructure.USART_Mode = USART_Mode_Rx;
//	if (stop_bits == 1)
//		USART_InitStructure.USART_StopBits = USART_StopBits_1;
//	else if (stop_bits == 2)
//		USART_InitStructure.USART_StopBits = USART_StopBits_2;
//	else if (stop_bits == 3)
//		USART_InitStructure.USART_StopBits = USART_StopBits_0_5;
//	else if (stop_bits == 4)
//		USART_InitStructure.USART_StopBits = USART_StopBits_1_5;
//	else
//		USART_InitStructure.USART_StopBits = USART_StopBits_1;
//
//	USART_Init(port, &USART_InitStructure);

	LL_USART_Init(port, &USART_InitStructure);

	if (port == USART1) {
		NVIC_EnableIRQ( USART1_IRQn );
	}
	else if (port == USART2) {
		NVIC_EnableIRQ( USART2_IRQn );
	}

	port->CR1 |= USART_CR1_UE;
	port->ISR &= (0xFFFFFFFF ^ USART_ISR_TC);

	ctx->srl_rx_state = SRL_RX_IDLE;
	ctx->srl_tx_state = SRL_TX_IDLE;

	ctx->srl_rx_error_reason = SRL_ERR_NONE;

	ctx->srl_rx_timeout_calc_started = 0;
	ctx->total_idle_counter = 0;

	ctx->srl_rx_start_time = 0;
	ctx->srl_rx_waiting_start_time = 0;

	ctx->srl_rx_timeout_enable = 0;
	ctx->srl_rx_timeout_waiting_enable = 0;
}

void srl_close(srl_context_t *ctx) {
	LL_USART_DeInit(ctx->port);

	if (ctx->te_port != NULL && ctx->te_pin != 0) {
		LL_GPIO_ResetOutputPin(ctx->te_port, ctx->te_pin);
	}

	ctx->srl_rx_state = SRL_RX_NOT_CONFIG;
	ctx->srl_tx_state = SRL_TX_NOT_CONFIG;
}

// this function shall be called in 10ms periods by some timer to check the timeout
// during receive. This method works differently depends on what receive mode was initiaded
//
// if start & stop characters are in use the timeout will be calculted from the time when
// start character was received and operation mode has been switched from SRL_WAITING_TO_RX
// to SRL_RXING
//
// if no start & stop character is used by software timeout is calculated from the time when
// first character was received after calling srl_receive_data
void srl_keep_timeout(srl_context_t *ctx) {
	if (ctx->srl_rx_state != SRL_RX_NOT_CONFIG && ctx->srl_rx_timeout_enable == 1) {

		// checking if flag to check a timeout is raised
		if (ctx->srl_rx_timeout_calc_started == 1) {

			// check if timeout expired
			if (master_time - ctx->srl_rx_start_time > ctx->srl_rx_timeout_trigger_value_in_msec) {
				// disable the receiving part of UART, disable interrupt and switch to an error state
				ctx->port->CR1 &= (0xFFFFFFFF ^ USART_CR1_RE);
				ctx->port->CR1 &= (0xFFFFFFFF ^ USART_CR1_RXNEIE);

				ctx->srl_rx_state = SRL_RX_ERROR;

				ctx->srl_rx_error_reason = SRL_ERR_TIMEOUT_RECEIVING;
			}
		}

	}
	else {
		;
	}

	if ((ctx->srl_rx_state == SRL_WAITING_TO_RX || ctx->srl_rx_state == SRL_RXING ) && ctx->srl_rx_timeout_waiting_enable == 1) {
		if (master_time - ctx->srl_rx_waiting_start_time > ctx->srl_rx_timeout_trigger_value_in_msec) {
			ctx->port->CR1 &= (0xFFFFFFFF ^ USART_CR1_RE);
			ctx->port->CR1 &= (0xFFFFFFFF ^ USART_CR1_RXNEIE);

			ctx->srl_rx_state = SRL_RX_ERROR;

			ctx->srl_rx_error_reason = SRL_ERR_TIMEOUT_WAITING;
		}
	}
}

uint8_t srl_send_data(srl_context_t *ctx, uint8_t* data, uint8_t mode, uint16_t leng, uint8_t internal_external) {
	if (ctx->srl_tx_state == SRL_TXING)
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

	// the bytes counter needs to be set to 1 as the first byte is sent in this function
	ctx->srl_tx_bytes_counter = 1;

	// if an user want to send data using internal buffer
	if (internal_external == 0) {

		// if data at the input is too long to fit in the buffer
		if (leng >= ctx->srl_rx_buf_ln)
			return SRL_DATA_TOO_LONG;

		// setting a pointer to transmit buffer to the internal buffer inside the driver
		ctx->srl_tx_buf_pointer = srl_usart1_tx_buffer;

		ctx->srl_tx_buf_ln = TX_BUFFER_1_LN;

		// cleaning the buffer from previous content
		memset(ctx->srl_tx_buf_pointer, 0x00, TX_BUFFER_1_LN);

		// copying the data from provided pointer to internal buffer
		if (mode == 0) {
			// copying everything till the 0x00 is spoted or till the buffer border is reached
			for (i = 0; (i < TX_BUFFER_1_LN && *(data+i) != '\0'); i++)
				ctx->srl_tx_buf_pointer[i]=data[i];
			ctx->srl_tx_bytes_req = i;
		}
		else if (mode == 1) {
			// we don't need to check against buffer size because this was confirmed already
			for (i = 0; i<=leng ; i++)
				ctx->srl_tx_buf_pointer[i]=data[i];
			ctx->srl_tx_bytes_req = leng;
		}
	}
	else if (internal_external == 1) {
		ctx->srl_tx_buf_pointer = data;
		ctx->srl_tx_bytes_req = leng;
		ctx->srl_tx_buf_ln = leng;
	}
	else return SRL_WRONG_BUFFER_PARAM;

	// enabling transmitter
	ctx->port->CR1 |= USART_CR1_TE;
	ctx->port->ISR &= (0xFFFFFFFF ^ USART_ISR_TC);
	ctx->port->TDR = ctx->srl_tx_buf_pointer[0];
	ctx->srl_tx_state = SRL_TXING;
	ctx->port->CR1 |= USART_CR1_TXEIE;				// przerwanie zg�aszane kiedy rejsetr DR jest pusty
	ctx->port->CR1 |= USART_CR1_TCIE;				// przerwanie zg�aszane po transmisji bajtu
												// je�eli rejestr DR jest nadal pusty
	return SRL_OK;

}

/**
 * This function assumes than
 */
uint8_t srl_start_tx(srl_context_t *ctx, short leng) {
	if (ctx->srl_tx_state == SRL_TXING)
		return SRL_BUSY;

	// if data at the input is too long to fit in the buffer
	if (leng >= ctx->srl_rx_buf_ln)
		return SRL_DATA_TOO_LONG;

	ctx->srl_tx_bytes_req = leng;

	// setting a pointer to transmit buffer to the internal buffer inside the driver
	//ctx->srl_tx_buf_pointer = srl_usart1_tx_buffer;

//	if (ctx->te_port != 0)
//		GPIO_SetBits(ctx->te_port, ctx->te_pin);

	// check if delay should be applied to transmission
	if (ctx->srl_tx_start_time == 0xFFFFFFFFu) {
		// 0xFFFFFFFF is a magic number which disables the pre-tx delay
		ctx->port->CR1 |= USART_CR1_TE;
		ctx->port->ISR &= (0xFFFFFFFF ^ USART_ISR_TC);
		ctx->port->TDR = ctx->srl_tx_buf_pointer[0];

		// the bytes counter needs to be set to 1 as the first byte is sent in this function
		ctx->srl_tx_bytes_counter = 1;

		ctx->srl_tx_state = SRL_TXING;

		ctx->port->CR1 |= USART_CR1_TXEIE;
		ctx->port->CR1 |= USART_CR1_TCIE;

	}
	else {
		ctx->srl_tx_state = SRL_TX_WAITING;
		ctx->srl_tx_start_time = main_get_master_time();
	}

	return SRL_OK;
}

void srl_wait_for_tx_completion(srl_context_t *ctx) {
	  while(ctx->srl_tx_state != SRL_TX_IDLE && ctx->srl_tx_state != SRL_TX_ERROR);
}

uint8_t srl_wait_for_rx_completion_or_timeout(srl_context_t *ctx, uint8_t* output) {

	*output = SRL_UNINITIALIZED;

	// block the execution until the
	while(ctx->srl_rx_state != SRL_RX_DONE && ctx->srl_rx_state != SRL_RX_ERROR);

	switch (ctx->srl_rx_state) {
		case SRL_RX_DONE: {
			*output = SRL_OK;
			break;
		}

		case SRL_RX_ERROR: {
			*output = SRL_TIMEOUT;
			break;
		}

		default: {
			break;
		}
	}

	return *output;
}

uint8_t srl_receive_data(srl_context_t *ctx, int num, char start, char stop, char echo, char len_addr, char len_modifier) {
	if (ctx->srl_rx_state == SRL_RXING)
		return SRL_BUSY;

	//trace_printf("Serial:SrlReceiveData()\r\n");

	if (num >= ctx->srl_rx_buf_ln)
		return SRL_DATA_TOO_LONG;

	memset(ctx->srl_rx_buf_pointer, 0x00, ctx->srl_rx_buf_ln);

	ctx->srl_rx_error_reason = SRL_ERR_NONE;

	// checking if user want
	if (start != 0x00) {
		ctx->srl_triggered_start = 1;
		ctx->srl_start_trigger = start;
	}
	else {
		ctx->srl_triggered_start = 0;
		ctx->srl_start_trigger = 0;
	}

	if (stop != 0x00) {
		ctx->srl_triggered_stop = 1;
		ctx->srl_stop_trigger = stop;
	}
	else {
		ctx->srl_triggered_stop = 0;
		ctx->srl_stop_trigger = 0;
	}

	if (ctx->srl_triggered_start == 1 || ctx->srl_triggered_stop == 1) {
		if (num < 3)
			return SRL_WRONG_PARAMS_COMBINATION;

		ctx->srl_rx_state = SRL_WAITING_TO_RX;
		ctx->srl_rx_waiting_start_time = master_time;
	}
	else {
		ctx->srl_rx_state = SRL_RXING;
	}

	ctx->srl_enable_echo = echo;
	ctx->srl_rx_bytes_counter = 0;
	ctx->srl_rx_bytes_req = num;

	if (len_addr != 0) {
		ctx->srl_rx_lenght_param_addres = len_addr;
	}
	else {
		ctx->srl_rx_lenght_param_addres = num + 1;
	}
	ctx->srl_rx_lenght_param_modifier = len_modifier;

	ctx->srl_rx_timeout_calc_started = 0;

	ctx->port->CR1 |= USART_CR1_RE;					// uruchamianie odbiornika
	ctx->port->CR1 |= USART_CR1_RXNEIE;			// przerwanie od przepe�nionego bufora odbioru
// 	PORT->CR1 |= USART_CR1_IDLEIE;			// przerwanie od bezczynno�ci szyny RS przy odbiorze
												// spowodowanej zako�czeniem transmisji przez urz�dzenie
 	return SRL_OK;
}

/**
 * This function start the transfer with
 */
uint8_t srl_receive_data_with_instant_timeout(srl_context_t *ctx, int num, char start, char stop, char echo, char len_addr, char len_modifier) {
	if (ctx->srl_rx_state == SRL_RXING)
		return SRL_BUSY;

	if (num >= ctx->srl_rx_buf_ln)
		return SRL_DATA_TOO_LONG;

	memset(ctx->srl_rx_buf_pointer, 0x00, ctx->srl_rx_buf_ln);

	// checking if user want
	if (start != 0x00) {
		ctx->srl_triggered_start = 1;
		ctx->srl_start_trigger = start;
	}
	else {
		ctx->srl_triggered_start = 0;
	}

	if (stop != 0x00) {
		ctx->srl_triggered_stop = 1;
		ctx->srl_stop_trigger = stop;
	}
	else {
		ctx->srl_triggered_stop = 0;
	}

	if (ctx->srl_triggered_start == 1 || ctx->srl_triggered_stop == 1) {
		if (num < 3)
			return SRL_WRONG_PARAMS_COMBINATION;

		ctx->srl_rx_state = SRL_WAITING_TO_RX;
		ctx->srl_rx_waiting_start_time = master_time;
	}
	else {
		ctx->srl_rx_state = SRL_RXING;
	}

	ctx->srl_enable_echo = echo;
	ctx->srl_rx_bytes_counter = 0;
	ctx->srl_rx_bytes_req = num;

	if (len_addr != 0) {
		ctx->srl_rx_lenght_param_addres = len_addr;
	}
	else {
		ctx->srl_rx_lenght_param_addres = num + 1;
	}
	ctx->srl_rx_lenght_param_modifier = len_modifier;

	// set current time as receive start time
	ctx->srl_rx_start_time = master_time;

	ctx->srl_rx_timeout_calc_started = 1;

	ctx->port->CR1 |= USART_CR1_RE;					// uruchamianie odbiornika
	ctx->port->CR1 |= USART_CR1_RXNEIE;			// przerwanie od przepe�nionego bufora odbioru
// 	PORT->CR1 |= USART_CR1_IDLEIE;			// przerwanie od bezczynno�ci szyny RS przy odbiorze
												// spowodowanej zako�czeniem transmisji przez urz�dzenie
 	return SRL_OK;
}

uint8_t srl_receive_data_with_callback(srl_context_t *ctx, srl_rx_termination_callback_t cbk) {
	uint8_t retval = SRL_OK;

	if (ctx->srl_rx_state == SRL_RXING) {
		retval = SRL_BUSY;
	}
	else {
		// check if input pointers were set to something
		if (cbk == NULL || ctx == NULL) {
			retval = SRL_WRONG_PARAMS_COMBINATION;
		}
		else {
			// set the callback pointer within the context
			ctx->srl_rx_term = cbk;

			// set the amount of bytes to be received as the size
			// of the receive buffer (minus one byte for safety).
			// it will be up to the callback function to terminate the receiving
			ctx->srl_rx_bytes_req = ctx->srl_rx_buf_ln - 1;

			// clear the rx buffer
			memset(ctx->srl_rx_buf_pointer, 0x00, ctx->srl_rx_buf_ln);

			ctx->srl_rx_bytes_counter = 0;

			ctx->srl_rx_lenght_param_addres = ctx->srl_rx_buf_ln - 1;
			ctx->srl_rx_lenght_param_modifier = 0;

			ctx->srl_triggered_start = 0;
			ctx->srl_triggered_stop = 0;
			ctx->srl_enable_echo = 0;

			ctx->srl_rx_timeout_calc_started = 0;

			ctx->srl_rx_state = SRL_RXING;
			ctx->port->CR1 |= USART_CR1_RE;					// uruchamianie odbiornika
			ctx->port->CR1 |= USART_CR1_RXNEIE;
			ctx->port->CR1 |= USART_CR1_IDLEIE;

			ctx->srl_rx_waiting_start_time = master_time;
			ctx->srl_rx_start_time = master_time;

		}
	}

	return retval;
}

void srl_irq_handler(srl_context_t *ctx) {

	// local variable for recalculating a stream length (how many bytes the driver should receives)
	uint16_t len_temp = 0;

	// set to one if there are conditions to stop receiving
	uint8_t stop_rxing = 0;

	// local variable to store
	uint8_t value = 0;

	if ((ctx->port->ISR & USART_ISR_IDLE) == USART_ISR_IDLE) {
		ctx->srl_garbage_storage = (uint8_t)ctx->port->RDR;

		ctx->total_idle_counter++;
	}

	// if overrun happened, a byte hadn't been transferred from DR before the next byte is received
	if ((ctx->port->ISR & USART_ISR_ORE) == USART_ISR_ORE) {
		switch (ctx->srl_rx_state) {
			case SRL_RXING:
				ctx->srl_garbage_storage = (uint8_t)ctx->port->RDR;

				break;
			default:
				// if the UART driver is not receiving actually but hardware controler received any data
				// it is required to read value of DR register to clear the interrupt
				ctx->srl_garbage_storage = (uint8_t)ctx->port->RDR;
				break;
		}
	}

	// if any data has been received by the UART controller
	if ((ctx->port->ISR & USART_ISR_RXNE) == USART_ISR_RXNE) {

		// incremenet the received bytes counter
		ctx->total_rx_bytes++;

		switch (ctx->srl_rx_state) {
			case SRL_RXING: {

				ctx->srl_rx_start_time = master_time;

				// raise a flag to signalize that timeout shall be calulated from now.
				ctx->srl_rx_timeout_calc_started = 1;

				// disable the waiting timeout
				ctx->srl_rx_timeout_waiting_enable = 0;

				// if there is any data remaining to receive
				if (ctx->srl_rx_bytes_counter < ctx->srl_rx_bytes_req) {

					// storing received byte into buffer
					ctx->srl_rx_buf_pointer[ctx->srl_rx_bytes_counter] = (uint8_t)ctx->port->RDR;

					// check if termination callback pointer has been set
					if (ctx->srl_rx_term != NULL) {
						// if yes call it
						stop_rxing = ctx->srl_rx_term(	ctx->srl_rx_buf_pointer[ctx->srl_rx_bytes_counter],
														ctx->srl_rx_buf_pointer,
														ctx->srl_rx_bytes_counter);

						// and check the return value
						if (stop_rxing == 1) {
							// if this was the last byte of transmission switch the state
							// of receiving part to done
							ctx->srl_rx_state = SRL_RX_DONE;

							ctx->srl_triggered_stop = 0;
						}

					}

					// checking if this byte in stream holds the protocol information about
					// how many bytes needs to be received.
					if (ctx->srl_rx_lenght_param_addres == ctx->srl_rx_bytes_counter) {
						len_temp = ctx->srl_rx_buf_pointer[ctx->srl_rx_bytes_counter];

						// adding (or substracting) a length modifier
						len_temp += ctx->srl_rx_lenght_param_modifier;

						// if the target length is bigger than buffer size switch to error state
						if (len_temp >= ctx->srl_rx_buf_ln) {
							ctx->srl_rx_error_reason = SRL_ERR_OVERFLOW;
							ctx->srl_rx_state = SRL_RX_ERROR;
							stop_rxing = 1;
						}
						else {
							ctx->srl_rx_bytes_req = len_temp;
						}
					}

					// moving buffer pointer forward
					ctx->srl_rx_bytes_counter++;

				}

				// if the user want the driver to stop receiving after certain is received
				if (ctx->srl_triggered_stop == 1) {
					if (ctx->srl_rx_buf_pointer[ctx->srl_rx_bytes_counter - 1] == ctx->srl_stop_trigger) {
						ctx->srl_rx_state = SRL_RX_DONE;
						stop_rxing = 1;
					}
				}

				// if after incrementing a pointer we reached the end of the buffer
				if (ctx->srl_rx_bytes_counter >= ctx->srl_rx_bytes_req) {

					// enabling a flag to disble receiver
					stop_rxing = 1;

					// setting a state to receive done
					ctx->srl_rx_state = SRL_RX_DONE;
				}

				if (stop_rxing == 1) {

					ctx->srl_rx_timeout_calc_started = 0;

					// disabling UART receiver and its interrupt
					ctx->port->CR1 &= (0xFFFFFFFF ^ USART_CR1_RE);
					ctx->port->CR1 &= (0xFFFFFFFF ^ USART_CR1_RXNEIE);
					ctx->port->CR1 &= (0xFFFFFFFF ^ USART_CR1_IDLEIE);
				}

				break;
			}

			// the state when a driver is waiting for start character to appear on serial link
			case SRL_WAITING_TO_RX: {

				// storing the value of DR register into local variable to protect against data races
				// which may happened if this IT routine will be preempted by another (long) one
				value = (uint8_t)ctx->port->RDR;

				// checking if start character was received
				if (value == ctx->srl_start_trigger) {

					// storing received byte in buffer as firts one
					ctx->srl_rx_buf_pointer[ctx->srl_rx_bytes_counter] = value;

					// increasing the counter value
					ctx->srl_rx_bytes_counter++;

					// change state to receiving
					ctx->srl_rx_state = SRL_RXING;

					// as receiving is started there is no point to calculate waiting timeout
					ctx->srl_rx_timeout_waiting_enable = 0;
				}
				else {
					// if this is not start byte just store it in garbage buffer to clear interrupt condition
					ctx->srl_garbage_storage = value;
				}
				break;
			}
			default: break;
		}

	}

	// if one byte was successfully transferred from DR to shift register for transmission over USART
	if ((ctx->port->ISR & USART_ISR_TXE) == USART_ISR_TXE) {

		// increment the transmitted bytes counter
		ctx->total_tx_bytes++;

		switch (ctx->srl_tx_state) {
		case SRL_TXING:
			if (ctx->srl_tx_bytes_counter < ctx->srl_tx_bytes_req) {
				if (ctx->te_port != 0) {
					LL_GPIO_SetOutputPin(ctx->te_port, ctx->te_pin);
				}

				ctx->port->TDR = ctx->srl_tx_buf_pointer[ctx->srl_tx_bytes_counter++];
			}
			else {
				while((ctx->port->ISR & USART_ISR_TC) != USART_ISR_TC);
				ctx->port->CR1 &= (0xFFFFFFFF ^ USART_CR1_TE);		//wyġṗczanie nadajnika portu szeregowego
				ctx->port->CR1 &= (0xFFFFFFFF ^ USART_CR1_TXEIE);
				ctx->port->CR1 &= (0xFFFFFFFF ^ USART_CR1_TCIE);	// wyġṗczanie przerwañ od portu szeregowego
				ctx->port->ISR &= (0xFFFFFFFF ^ USART_ISR_TC);
				ctx->srl_tx_state = SRL_TX_IDLE;

				if (ctx->te_port != 0) {
					LL_GPIO_ResetOutputPin(ctx->te_port, ctx->te_pin);
				}

			}

			if (ctx->srl_tx_bytes_counter >= ctx->srl_tx_buf_ln ||
					ctx->srl_tx_bytes_req >= ctx->srl_tx_buf_ln) {

				ctx->port->CR1 &= (0xFFFFFFFF ^ USART_CR1_TE);		//wyġṗczanie nadajnika portu szeregowego
				ctx->port->CR1 &= (0xFFFFFFFF ^ USART_CR1_TXEIE);
				ctx->port->CR1 &= (0xFFFFFFFF ^ USART_CR1_TCIE);	// wyġṗczanie przerwañ od portu szeregowego
				ctx->port->ISR &= (0xFFFFFFFF ^ USART_ISR_TC);
				ctx->srl_tx_state = SRL_TX_IDLE;

				if (ctx->te_port != 0) {
					LL_GPIO_ResetOutputPin(ctx->te_port, ctx->te_pin);
				}

			}

			break;
			default: break;
		}
	}

}

uint16_t srl_get_num_bytes_rxed(srl_context_t *ctx) {
	return ctx->srl_rx_bytes_counter;
}

uint8_t* srl_get_rx_buffer(srl_context_t *ctx) {
	return ctx->srl_rx_buf_pointer;
}

void srl_keep_tx_delay(srl_context_t *ctx) {
	if (ctx != 0) {

		// check if pre tx delay is enabled by an user
		if (ctx->srl_tx_start_time != 0xFFFFFFFFu) {

			// if it is enabled then check if the serial port is currently set to waiting state
			if (ctx->srl_tx_state == SRL_TX_WAITING) {

				// check if a delay has expired
				if (main_get_master_time() - ctx->srl_tx_start_time >= SRL_TX_DELAY_IN_MS) {

					// if yes start the transmission
					ctx->port->CR1 |= USART_CR1_TE;
					ctx->port->ISR &= (0xFFFFFFFF ^ USART_ISR_TC);
					ctx->port->TDR = ctx->srl_tx_buf_pointer[0];

					ctx->srl_tx_bytes_counter = 1;

					ctx->srl_tx_state = SRL_TXING;

					ctx->port->CR1 |= USART_CR1_TXEIE;
					ctx->port->CR1 |= USART_CR1_TCIE;
				}
			}

		}
	}
}

void srl_switch_tx_delay(srl_context_t *ctx, uint8_t disable_enable) {
	if (ctx != 0) {

		if (disable_enable == 1) {
			ctx->srl_tx_start_time = 0x0u;
		}
		else {
			ctx->srl_tx_start_time = 0xFFFFFFFFu;
		}
	}
}

/**
 * This function controls the timeout which is calculated for data reception (when the
 * state is set to SRL_RXING). The time starts ticking after the first byte appears in
 * data register, so this protect against stalling in the middle of data transfer
 */
void srl_switch_timeout(srl_context_t *ctx, uint8_t disable_enable, uint32_t value) {
	if (disable_enable == 1)
		ctx->srl_rx_timeout_enable = 1;
	else if (disable_enable == 0)
		ctx->srl_rx_timeout_enable = 0;
	else {
		;
	}

	if (value != 0) {
		ctx->srl_rx_timeout_trigger_value_in_msec = value;
	}
	else {
		ctx->srl_rx_timeout_trigger_value_in_msec = SRL_DEFAULT_RX_TIMEOUT_IN_MS;
	}
}

/**
 * This function enables the timeout which is calculated for the waiting state for
 * the data reception to begin (the first byte on serial port). It must be called for
 * each RX transaction it is required as this timeout is cleared/disabled after the first
 * byte received by the serial port (when the state changes from SRL_WAITING_TO_RX to
 * SRL_RXING)
 */
void srl_switch_timeout_for_waiting(srl_context_t *ctx, uint8_t disable_enable) {
	if (disable_enable == 1)
		ctx->srl_rx_timeout_waiting_enable = 1;
	else if (disable_enable == 0)
		ctx->srl_rx_timeout_waiting_enable = 0;
	else {
		;
	}

	if (ctx->srl_rx_timeout_trigger_value_in_msec == 0)
		ctx->srl_rx_timeout_trigger_value_in_msec = SRL_DEFAULT_RX_TIMEOUT_IN_MS;

}
