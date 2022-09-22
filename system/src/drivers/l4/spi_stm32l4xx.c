/*
 * spi_stm32l4xx.c
 *
 *  Created on: Sep 22, 2022
 *      Author: mateusz
 */

#include "drivers/spi.h"

#include "etc/spi_slave_config.h"

#include <stm32l4xx.h>
#include <stm32l4xx_ll_spi.h>
#include <stm32l4xx_ll_gpio.h>

#define SPI_BUFFER_LN	32

spi_rx_state_t spi_rx_state;

spi_tx_state_t spi_tx_state;

uint8_t spi_rx_buffer[SPI_BUFFER_LN];

uint8_t spi_tx_buffer[SPI_BUFFER_LN];

EVAL_SLAVE_ARR

uint8_t spi_init_full_duplex(spi_transfer_mode_t mode, spi_clock_polarity_strobe_t strobe, int speed, int endianess) {

	LL_GPIO_InitTypeDef GPIO_InitTypeDef;
	LL_SPI_InitTypeDef SPI_InitTypeDef;

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

	GPIO_InitTypeDef.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitTypeDef.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitTypeDef.Pin = LL_GPIO_PIN_12;
	GPIO_InitTypeDef.Pull = LL_GPIO_PULL_NO;
	GPIO_InitTypeDef.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitTypeDef.Alternate = LL_GPIO_AF_5;
	LL_GPIO_Init(GPIOB, &GPIO_InitTypeDef);		// SPI_NSS

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
			SPI_InitTypeDef.ClockPolarity = LL_SPI_POLARITY_LOW;
			SPI_InitTypeDef.ClockPhase = LL_SPI_PHASE_2EDGE;

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

}

uint8_t spi_tx_data(uint32_t slave_id, uint8_t * tx_buffer, uint16_t ln_to_tx) {

}

uint8_t * spi_get_rx_data(void) {

}

void spi_enable(void) {
	LL_SPI_Enable(SPI2);
}

void spi_disable(void) {

}
