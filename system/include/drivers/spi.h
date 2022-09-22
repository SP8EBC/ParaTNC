/*
 * spi_stm32l4x.h
 *
 *  Created on: Sep 22, 2022
 *      Author: mateusz
 */

#ifndef INCLUDE_DRIVERS_SPI_H_
#define INCLUDE_DRIVERS_SPI_H_

#include <stdint.h>

#define SPI_OK		0

typedef enum spi_transfer_mode_t {
	SPI_MASTER_MOTOROLA,
	SPI_MASTER_TI,
	SPI_SLAVE_MOTOROLA,
	SPI_SLAVE_TI
}spi_transfer_mode_t;

typedef enum spi_clock_polarity_strobe_t {
	CLOCK_NORMAL_FALLING,
	CLOCK_NORMAL_RISING,
	CLOCK_REVERSED_FALLING,
	CLOCK_REVERSED_RISING
}spi_clock_polarity_strobe_t;

#define SPI_ENDIAN_LSB	0
#define SPI_ENDIAN_MSB	1

typedef enum spi_rx_state_t {
	SPI_RX_IDLE,
	SPI_RX_RXING,
	SPI_RX_DONE
}spi_rx_state_t;

typedef enum spi_tx_state_t {
	SPI_TX_IDLE,
	SPI_TX_TXING,
	SPI_TX_DONE
}spi_tx_state_t;

uint8_t spi_init_full_duplex(spi_transfer_mode_t mode, spi_clock_polarity_strobe_t strobe, int speed, int endianess);
uint8_t spi_rx_data(uint32_t slave_id, uint8_t * rx_buffer, uint16_t ln_to_rx);
uint8_t spi_tx_data(uint32_t slave_id, uint8_t * tx_buffer, uint16_t ln_to_tx);
uint8_t * spi_get_rx_data(void);

void spi_enable(void);
void spi_disable(void);

#endif /* INCLUDE_DRIVERS_SPI_H_ */
