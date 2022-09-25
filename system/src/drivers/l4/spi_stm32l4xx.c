/*
 * spi_stm32l4xx.c
 *
 *  Created on: Sep 22, 2022
 *      Author: mateusz
 */

#include "drivers/spi.h"

#include "main.h"
#include "etc/spi_slave_config.h"

#include <stm32l4xx.h>
#include <stm32l4xx_ll_spi.h>
#include <stm32l4xx_ll_gpio.h>
#include <stm32l4xx_ll_rcc.h>

#include <string.h>

#define SPI_BUFFER_LN	32

/**
 * State of RX part
 */
volatile spi_rx_state_t spi_rx_state;

/**
 * State of TX part
 */
volatile spi_tx_state_t spi_tx_state;

/**
 * ID of current slave the communication is established with.
 */
uint32_t spi_current_slave = 1;

/**
 * Amount of bytes requested to be received
 */
uint16_t spi_rx_bytes_rq;

/**
 * Current number of bytes recieved from slave
 */
uint16_t spi_current_rx_cntr;

/**
 * Amount of bytes requested to be transmitted
 */
uint16_t spi_tx_bytes_rq;

/**
 * Current number of bytes transmitted to slave device
 */
uint16_t spi_current_tx_cntr;

/**
 * receive buffer
 */
uint8_t spi_rx_buffer[SPI_BUFFER_LN];

/**
 * transmision buffer
 */
uint8_t spi_tx_buffer[SPI_BUFFER_LN];

/**
 * Pointer to receive buffer. A user can request to receive data into
 * internal buffer which will set that pointer to `spi_rx_buffer`.
 * This will point to an external buffer if it is requested by
 * an user.
 */
uint8_t * spi_rx_buffer_ptr;

uint8_t * spi_tx_buffer_ptr;

/**
 * A byte to store garbage and unwanted data when SPI got more data
 * than spi_rx_bytes_rq or reception hasn't been initiated (tx - only)
 */
uint8_t spi_garbage;

/**
 * Timestamp when receiving has been initiated
 */
uint32_t spi_timestamp_rx_start;

/**
 * How many bytes has been discarded into garbage storage
 */
uint8_t spi_garbage_counter = 0;

EVAL_SLAVE_ARR

uint8_t spi_init_full_duplex_pio(spi_transfer_mode_t mode, spi_clock_polarity_strobe_t strobe, int speed, int endianess) {

	LL_GPIO_InitTypeDef GPIO_InitTypeDef;
	LL_SPI_InitTypeDef SPI_InitTypeDef;

	RCC->APB1RSTR1 |= RCC_APB1RSTR1_SPI2RST;

	spi_rx_state = SPI_RX_IDLE;
	spi_tx_state = SPI_TX_IDLE;

	/**
	 *	PB12 - SPI_NSS
	 *	PB13 - SPI_CLK
	 *	PB14 - SPI_MISO
	 *	PB15 - SPI_MOSI
	 *	PA12 - SPI_NSS_PT100
	 */

	GPIO_InitTypeDef.Mode = LL_GPIO_MODE_ALTERNATE;
	GPIO_InitTypeDef.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitTypeDef.Pin = LL_GPIO_PIN_13;
	GPIO_InitTypeDef.Pull = LL_GPIO_PULL_NO;
	GPIO_InitTypeDef.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitTypeDef.Alternate = LL_GPIO_AF_5;
	LL_GPIO_Init(GPIOB, &GPIO_InitTypeDef);		// SPI_CLK

	GPIO_InitTypeDef.Mode = LL_GPIO_MODE_ALTERNATE;
	GPIO_InitTypeDef.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitTypeDef.Pin = LL_GPIO_PIN_14;
	GPIO_InitTypeDef.Pull = LL_GPIO_PULL_NO;
	GPIO_InitTypeDef.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitTypeDef.Alternate = LL_GPIO_AF_5;
	LL_GPIO_Init(GPIOB, &GPIO_InitTypeDef);		// SPI_MISO

	GPIO_InitTypeDef.Mode = LL_GPIO_MODE_ALTERNATE;
	GPIO_InitTypeDef.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitTypeDef.Pin = LL_GPIO_PIN_15;
	GPIO_InitTypeDef.Pull = LL_GPIO_PULL_NO;
	GPIO_InitTypeDef.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitTypeDef.Alternate = LL_GPIO_AF_5;
	LL_GPIO_Init(GPIOB, &GPIO_InitTypeDef);		// SPI_MOSI

	GPIO_InitTypeDef.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitTypeDef.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitTypeDef.Pin = LL_GPIO_PIN_12;
	GPIO_InitTypeDef.Pull = LL_GPIO_PULL_NO;
	GPIO_InitTypeDef.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitTypeDef.Alternate = LL_GPIO_AF_5;
	LL_GPIO_Init(GPIOA, &GPIO_InitTypeDef);		// SPI_NSS_PT100
	LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_12);

	GPIO_InitTypeDef.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitTypeDef.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitTypeDef.Pin = LL_GPIO_PIN_12;
	GPIO_InitTypeDef.Pull = LL_GPIO_PULL_NO;
	GPIO_InitTypeDef.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitTypeDef.Alternate = LL_GPIO_AF_5;
	LL_GPIO_Init(GPIOB, &GPIO_InitTypeDef);		// SPI_NSS
	LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_12);

	RCC->APB1RSTR1 &= (0xFFFFFFFF ^ RCC_APB1RSTR1_SPI2RST);

	LL_SPI_Disable(SPI2);
	LL_SPI_StructInit(&SPI_InitTypeDef);

	// Configure the serial clock baud rate
	if (mode != SPI_SLAVE_MOTOROLA) {
		SPI_InitTypeDef.BaudRate = (uint32_t)speed;
	}

	switch (mode) {
	case SPI_MASTER_MOTOROLA:
		SPI_InitTypeDef.Mode = LL_SPI_MODE_MASTER;
		LL_SPI_SetStandard(SPI2, LL_SPI_PROTOCOL_MOTOROLA);
		break;
	case SPI_MASTER_TI:
		SPI_InitTypeDef.Mode = LL_SPI_MODE_MASTER;
		LL_SPI_SetStandard(SPI2, LL_SPI_PROTOCOL_TI);
		break;
	case SPI_SLAVE_MOTOROLA:
		SPI_InitTypeDef.Mode = LL_SPI_MODE_SLAVE;
		LL_SPI_SetStandard(SPI2, LL_SPI_PROTOCOL_MOTOROLA);
		break;
	case SPI_SLAVE_TI:
		SPI_InitTypeDef.Mode = LL_SPI_MODE_SLAVE;
		LL_SPI_SetStandard(SPI2, LL_SPI_PROTOCOL_TI);
		break;
	}

	// Configure the CPOL and CPHA bits combination
	switch(strobe) {
		case CLOCK_NORMAL_FALLING:
			SPI_InitTypeDef.ClockPolarity = LL_SPI_POLARITY_LOW;		// CPOL
			SPI_InitTypeDef.ClockPhase = LL_SPI_PHASE_2EDGE;			// CPHA

			break;
		case CLOCK_NORMAL_RISING:
			SPI_InitTypeDef.ClockPolarity = LL_SPI_POLARITY_LOW;
			SPI_InitTypeDef.ClockPhase = LL_SPI_PHASE_1EDGE;

			break;
		case CLOCK_REVERSED_FALLING:
			SPI_InitTypeDef.ClockPolarity = LL_SPI_POLARITY_HIGH;
			SPI_InitTypeDef.ClockPhase = LL_SPI_PHASE_1EDGE;

			break;
		case CLOCK_REVERSED_RISING:
			SPI_InitTypeDef.ClockPolarity = LL_SPI_POLARITY_HIGH;
			SPI_InitTypeDef.ClockPhase = LL_SPI_PHASE_2EDGE;

			break;
	}

	// Select simplex or half-duplex mode by configuring RXONLY or BIDIMODE and BIDIOE
	SPI_InitTypeDef.TransferDirection = LL_SPI_FULL_DUPLEX;

	// Configure the LSBFIRST bit to define the frame format
	if (endianess == SPI_ENDIAN_LSB)
		SPI_InitTypeDef.BitOrder = LL_SPI_LSB_FIRST;
	else
		SPI_InitTypeDef.BitOrder = LL_SPI_MSB_FIRST;

	// Configure the CRCL and CRCEN bits if CRC is needed
	SPI_InitTypeDef.CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE;

	// Configure SSM and SSI
	SPI_InitTypeDef.NSS = LL_SPI_NSS_SOFT;

	// Configure the MSTR bit (in multimaster NSS configuration, avoid conflict state on
	// NSS if master is configured to prevent MODF error)	-> SPI_LL_EC_MODE Operation Mode

	// Configure the DS[3:0] bits to select the data length for the transfer.
	SPI_InitTypeDef.DataWidth = LL_SPI_DATAWIDTH_8BIT;

	LL_SPI_Init(SPI2, &SPI_InitTypeDef);

	// Set the NSSP bit if the NSS pulse mode between two data units is required (keep
	// CHPA and TI bits cleared in NSSP mode).
	LL_SPI_DisableNSSPulseMgt(SPI2);

	// Configure the FRXTH bit. The RXFIFO threshold must be aligned to the read
	// access size for the SPIx_DR register.
	LL_SPI_SetRxFIFOThreshold(SPI2, LL_SPI_RX_FIFO_TH_QUARTER);

	// enable interrupts
	LL_SPI_EnableIT_ERR(SPI2);
	LL_SPI_EnableIT_RXNE(SPI2);
	LL_SPI_EnableIT_TXE(SPI2);

	NVIC_EnableIRQ(SPI2_IRQn);

	return SPI_OK;
}

/**
 * void SPI_SD_Init( void )
//{
//    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
//    RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
//
//    // GPIOB - SCK, MISO, MOSI
//	GPIOB->MODER |= GPIO_MODER_MODER13_1 | GPIO_MODER_MODER14_1 | GPIO_MODER_MODER15_1;
//	GPIOB->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR13 | GPIO_OSPEEDER_OSPEEDR14 | GPIO_OSPEEDER_OSPEEDR15;
//	GPIOB->AFR[1] = 0x55500000;
//
//	// GPIOB - PB11( CS )
//	GPIOB->MODER |= GPIO_MODER_MODER11_0;
//	GPIOB->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR11;
//
//	// init spi2
//	RCC->APB1RSTR |= RCC_APB1RSTR_SPI2RST;
//    delay_ms( 10 );
//    RCC->APB1RSTR &= ~RCC_APB1RSTR_SPI2RST;
//
//	SPI2->CR1 |= SPI_CR1_MSTR | SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_CPHA | SPI_CR1_CPOL | SPI_CR1_BR_0;
//    SPI2->CR1 |= SPI_CR1_SPE;
//
//	DESELECT();
//}
 *
 *
//#define SPI_CR1_CPHA_Pos         (0U)
//#define SPI_CR1_CPHA_Msk         (0x1UL << SPI_CR1_CPHA_Pos)                   /*!< 0x00000001 */
//#define SPI_CR1_CPHA             SPI_CR1_CPHA_Msk                              /*!<Clock Phase      */
//#define SPI_CR1_CPOL_Pos         (1U)
//#define SPI_CR1_CPOL_Msk         (0x1UL << SPI_CR1_CPOL_Pos)                   /*!< 0x00000002 */
//#define SPI_CR1_CPOL             SPI_CR1_CPOL_Msk                              /*!<Clock Polarity   */

uint8_t spi_rx_data(uint32_t slave_id, uint8_t * rx_buffer, uint16_t ln_to_rx) {
	return 0xFF;
}

/**
 * Initiate tx only transaction. Data will be sent to chosen slave, any receive data will be discarded
 */
uint8_t spi_tx_data(uint32_t slave_id, uint8_t * tx_buffer, uint16_t ln_to_tx) {

	uint8_t out = SPI_UKNOWN;

	if (slave_id == 0 || slave_id > 2) {
		return SPI_WRONG_SLAVE_ID;
	}

	// check if tx is idle
	if (spi_tx_state == SPI_TX_IDLE || spi_tx_state == SPI_TX_DONE) {

		// usually SPI communications conducts in two schemas
		// 1. Transmit w/o reception (like writing to register)
		// 2. Transmit and then receive (writing an address of memory/register
		//								 to read and then receive that data)
		// In both cases transmission is a first communication relation,
		// so it can't be initialized if other reception is currently
		// pending.
		if (spi_rx_state == SPI_RX_IDLE || spi_rx_state == SPI_RX_DONE) {

			spi_tx_state = SPI_TX_TXING;

			spi_current_slave = slave_id;

			// if yes clear counter
			spi_current_tx_cntr = 0;

			// check if external or internal buffer shall be used
			if (tx_buffer == 0) {
				spi_tx_buffer_ptr = spi_tx_buffer;

				// check if internal buffer has enought room for data
				if (ln_to_tx <= SPI_BUFFER_LN) {
					// clear the buffer
					memset(spi_tx_buffer, 0x00, SPI_BUFFER_LN);

					// set the lenght
					spi_tx_buffer_ptr = spi_tx_buffer;

					// copy the content into a buffer
					memcpy(spi_tx_buffer_ptr, tx_buffer, ln_to_tx);

					// set amount of data for transmission
					spi_tx_bytes_rq = ln_to_tx;
				}
				else {
					out = SPI_TX_DATA_TO_LONG;
				}
			}
			else {
				// if external buffer shall be sent
				spi_tx_buffer_ptr = tx_buffer;

				spi_tx_bytes_rq = ln_to_tx;
			}

			spi_enable();
		}
	}
	else {
		out = SPI_BUSY;
	}

	return out;
}

/**
 *	Initiate full duplex communication with the slave. After this call the SPI bus
 *	will be enabled and all data from tx_buffer will be send. SPI bus will be disabled
 *	after requested amount of data will be received or timeout occured.
 */
uint8_t spi_rx_tx_data(uint32_t slave_id, uint8_t * rx_buffer, uint8_t * tx_buffer, uint16_t ln_to_rx, uint16_t ln_to_tx) {

	uint8_t out = SPI_UKNOWN;

	if (slave_id == 0 || slave_id > 2) {
		return SPI_WRONG_SLAVE_ID;
	}

	// check if SPI is busy
	if (spi_rx_state == SPI_RX_IDLE && (spi_tx_state == SPI_TX_IDLE || spi_tx_state == SPI_TX_DONE)) {

		spi_current_slave = slave_id;

		spi_current_rx_cntr = 0;
		spi_current_tx_cntr = 0;

		// check if external RX buffer shall be used or not
		if (rx_buffer == 0) {
			// no, internal one is needed
			spi_rx_buffer_ptr = spi_rx_buffer;

			// clear the buffer
			memset (spi_rx_buffer_ptr, 0x00, SPI_BUFFER_LN);
		}
		else {
			spi_rx_buffer_ptr = rx_buffer;

			// clear the buffer
			memset (spi_rx_buffer_ptr, 0x00, ln_to_rx);
		}

		// set the lenght
		spi_rx_bytes_rq = ln_to_rx;

		if ((SPI2->SR & SPI_SR_RXNE) != 0) {
			// clear RX fifo queue
			do {
				spi_garbage_counter++;
				spi_garbage = SPI2->DR & 0xFF;
			} while ((SPI2->SR & SPI_SR_RXNE) != 0);
		}

		// check if external TX buffer shall be user or not
		if (tx_buffer == 0) {
			spi_tx_buffer_ptr = spi_tx_buffer;

			// check if internal buffer has enought room for data
			if (ln_to_tx <= SPI_BUFFER_LN) {
				// clear the buffer
				memset(spi_tx_buffer, 0x00, SPI_BUFFER_LN);

				// set the lenght
				spi_tx_buffer_ptr = spi_tx_buffer;

				// copy the content into a buffer
				memcpy(spi_tx_buffer_ptr, tx_buffer, ln_to_tx);

				// set amount of data for transmission
				spi_tx_bytes_rq = ln_to_tx;
			}
			else {
				out = SPI_TX_DATA_TO_LONG;
			}
		}
		else {
			// if external buffer shall be sent
			spi_tx_buffer_ptr = tx_buffer;

			spi_tx_bytes_rq = ln_to_tx;
		}

		// latch a timestamp when reception has been triggered
		spi_timestamp_rx_start = master_time;

		// set first byte for transmission
		SPI2->DR = spi_tx_buffer_ptr[0];

		spi_rx_state = SPI_RX_RXING;
		spi_tx_state = SPI_TX_TXING;

		// start trasmission
		spi_enable();
	}
	else {
		// exit if either transmission or reception is ongoing
		out = SPI_BUSY;
	}

	return out;
}

uint8_t * spi_get_rx_data(void) {
	spi_rx_state = SPI_RX_IDLE;

	return spi_rx_buffer_ptr;
}

uint8_t spi_wait_for_comms_done(void) {
	if (spi_tx_state == SPI_TX_TXING) {
		while (spi_tx_state == SPI_TX_TXING);
	}

	// set the tx state to ile
	spi_tx_state = SPI_TX_IDLE;

	// check if read operation was successful or not
	if (spi_rx_state == SPI_RX_DONE) {
		spi_rx_state = SPI_RX_IDLE;

		return SPI_OK;
	}
	else {
		spi_rx_state = SPI_RX_IDLE;

		return SPI_UKNOWN;
	}
}

void spi_reset_errors(void) {
	if (spi_rx_state == SPI_RX_ERROR_MODF || spi_rx_state == SPI_RX_ERROR_OVERRUN || spi_rx_state == SPI_RX_ERROR_TIMEOUT) {
		spi_rx_state = SPI_RX_IDLE;
	}
}

void spi_irq_handler(void) {

	//	The RXNE flag is set depending on the FRXTH bit value in the SPIx_CR2 register:
	//	• If FRXTH is set, RXNE goes high and stays high until the RXFIFO level is greater or
	//	equal to 1/4 (8-bit).
	//	• If FRXTH is cleared, RXNE goes high and stays high until the RXFIFO level is greater
	//	than or equal to 1/2 (16-bit).
	//	An interrupt can be generated if the RXNEIE bit in the SPIx_CR2 register is set.
	//	The RXNE is cleared by hardware automatically when the above conditions are no longer
	//	true.
	if ((SPI2->SR & SPI_SR_RXNE) != 0) {

		// check if receiving is pending
		if (spi_rx_state == SPI_RX_RXING) {

			// get all data from RX FIFO
			do {
				// put received data into a buffer
				spi_rx_buffer[spi_current_rx_cntr++] = SPI2->DR & 0xFF;


				// check if all data has been received
				if (spi_current_rx_cntr >= spi_rx_bytes_rq) {
					// set slave select line high
					LL_GPIO_SetOutputPin((GPIO_TypeDef *)spi_slaves_cfg[spi_current_slave - 1][1], spi_slaves_cfg[spi_current_slave - 1][2]);

					// and switch the state
					spi_rx_state = SPI_RX_DONE;

					// and exit the loop
					break;

					// RXFIFO will be purged by 'spi_disable'
				}
			} while ((SPI2->SR & SPI_SR_RXNE) != 0);
		}
	}

	//	The TXE flag is set when transmission TXFIFO has enough space to store data to send.
	//	TXE flag is linked to the TXFIFO level. The flag goes high and stays high until the TXFIFO
	//	level is lower or equal to 1/2 of the FIFO depth. An interrupt can be generated if the TXEIE
	//	bit in the SPIx_CR2 register is set. The bit is cleared automatically when the TXFIFO level
	//	becomes greater than 1/2.
	if ((SPI2->SR & SPI_SR_TXE) != 0) {

		// check if transmission is pending
		if (spi_tx_state == SPI_TX_TXING) {

			do {

				if (spi_current_tx_cntr < spi_tx_bytes_rq) {
					// put next byte into the data register
					SPI2->DR = spi_tx_buffer[spi_current_tx_cntr++];
				}
				else {
					// finish transmission
					spi_tx_state = SPI_TX_DONE;

					break;
				}
				// if there are only two or one byte for slave device
				// TXE flag won't be cleared
			} while ((SPI2->SR & SPI_SR_TXE));
		}
	}

	//	Mode fault occurs when the master device has its internal NSS signal (NSS pin in NSS
	//	hardware mode, or SSI bit in NSS software mode) pulled low. This automatically sets the
	//	MODF bit. Master mode fault affects the SPI interface in the following ways:
	//	• The MODF bit is set and an SPI interrupt is generated if the ERRIE bit is set.
	//	• The SPE bit is cleared. This blocks all output from the device and disables the SPI
	//	interface.
	//	• The MSTR bit is cleared, thus forcing the device into slave mode.
	if ((SPI2->SR & SPI_SR_MODF) != 0) {
		spi_tx_state = SPI_TX_ERROR_MODF;
		spi_rx_state = SPI_RX_ERROR_MODF;

		spi_disable(1);
	}

	//	An overrun condition occurs when data is received by a master or slave and the RXFIFO
	//has not enough space to store this received data. This can happen if the software or the
	//DMA did not have enough time to read the previously received data (stored in the RXFIFO)
	//or when space for data storage is limited e.g. the RXFIFO is not available when CRC is
	//enabled in receive only mode so in this case the reception buffer is limited into a single data
	//frame buffer
	if ((SPI2->SR & SPI_SR_OVR) != 0) {

		// get all data from RX FIFO
		do {
			// check if all data has been received
			if (spi_current_rx_cntr >= spi_rx_bytes_rq) {
				// if yes empty the FIFO
				spi_garbage = SPI2->DR & 0xFF;
			}
			else {
				spi_rx_buffer[spi_current_rx_cntr++] = SPI2->DR & 0xFF;
			}
		} while ((SPI2->SR & SPI_SR_RXNE) != 0 && (SPI2->SR & SPI_SR_OVR) != 0);

		spi_rx_state = SPI_RX_ERROR_OVERRUN;
	}

	//	A TI mode frame format error is detected when an NSS pulse occurs during an ongoing
	//	communication when the SPI is operating in slave mode and configured to conform to the TI
	//	mode protocol. When this error occurs, the FRE flag is set in the SPIx_SR register. The SPI
	//	is not disabled when an error occurs, the NSS pulse is ignored, and the SPI waits for the
	//	next NSS pulse before starting a new transfer. The data may be corrupted since the error
	//	detection may result in the loss of two data bytes.
	if ((SPI2->SR & SPI_SR_FRE) != 0) {
		spi_disable(1);

	}

	// disable SPI if all communication is done
	if ((spi_rx_state == SPI_RX_IDLE || spi_rx_state == SPI_RX_DONE) && spi_tx_state == SPI_TX_DONE) {
		spi_disable(0);
	}

	if (spi_rx_state == SPI_RX_ERROR_OVERRUN) {
		spi_disable(1);
	}
}

/**
 * This function should be caller from SysTick Interrupt
 */
void spi_timeout_handler(void) {

	if (spi_rx_state == SPI_RX_RXING) {
		if (master_time - spi_timestamp_rx_start > SPI_RX_ERROR_TIMEOUT) {
			spi_rx_state = SPI_RX_ERROR_TIMEOUT;
			spi_tx_state = SPI_TX_IDLE;

			spi_disable(1);
		}
	}
 }

void spi_enable(void) {

	SPI2->CR2 |= SPI_CR2_ERRIE;
	SPI2->CR2 |= SPI_CR2_RXNEIE;
	SPI2->CR2 |= SPI_CR2_TXEIE;

	LL_GPIO_ResetOutputPin((GPIO_TypeDef *)spi_slaves_cfg[spi_current_slave - 1][1], spi_slaves_cfg[spi_current_slave - 1][2]);

	LL_SPI_Enable(SPI2);
}

/**
 * This function will disable SPI and close bus communication, optionally
 * it can wait until the peripheral will be released
 */
void spi_disable(uint8_t immediately) {

	if (immediately == 0) {
		while ((SPI2->SR & SPI_SR_BSY) != 0);

		LL_GPIO_SetOutputPin((GPIO_TypeDef *)spi_slaves_cfg[spi_current_slave - 1][1], spi_slaves_cfg[spi_current_slave - 1][2]);

		LL_SPI_Disable(SPI2);

		if ((SPI2->SR & SPI_SR_RXNE) != 0) {
			// clear RX fifo queue
			do {
				spi_garbage_counter++;

				spi_garbage = SPI2->DR & 0xFF;
			} while ((SPI2->SR & SPI_SR_RXNE) != 0);
		}
	}
	else {
		// disable all interrupts
		SPI2->CR2 &= (0xFFFFFFFF ^ SPI_CR2_ERRIE);
		SPI2->CR2 &= (0xFFFFFFFF ^ SPI_CR2_RXNEIE);
		SPI2->CR2 &= (0xFFFFFFFF ^ SPI_CR2_TXEIE);

		LL_SPI_Disable(SPI2);
	}

}
