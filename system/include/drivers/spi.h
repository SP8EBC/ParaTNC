/*
 * spi.h
 *
 * This is a general interface of SPI driver. Please read notes
 * in platform specific implementation. Each of them might have
 * it's own limitations
 *
 *  Created on: Sep 22, 2022
 *      Author: mateusz
 */

#ifndef INCLUDE_DRIVERS_SPI_H_
#define INCLUDE_DRIVERS_SPI_H_

#include <stdint.h>

/**
 * Return codes
 */
#define SPI_OK					0	//!< Function has succeeded
#define SPI_BUSY_DIFF_SLAVE		10	//!< Communication has been requested for different slave than what transmission is ongoing for
#define SPI_BUSY				11	//!< SPI bus is currently busy and cannot be used for requested operation
#define SPI_TX_DATA_TO_LONG		20	//!< requested data is too long to fit in internal SPI tx buffer
#define SPI_WRONG_SLAVE_ID		30	//!< unknown SPI slave (missing chip select mapping)
#define SPI_UKNOWN				255 //!< undesignated SPI error

/**
 * SPI transfer mode using for initialization
 */
typedef enum spi_transfer_mode_t {
	SPI_MASTER_MOTOROLA,
	SPI_MASTER_TI,
	SPI_SLAVE_MOTOROLA,
	SPI_SLAVE_TI
}spi_transfer_mode_t;

/**
 * SPI clock mode using for initialization, look at CPOL and CPHA values for each enum values.
 */
typedef enum spi_clock_polarity_strobe_t {
	CLOCK_NORMAL_FALLING,				//!< CPOL 0, CPHA 1
	CLOCK_NORMAL_RISING,				//!< CPOL 0, CPHA 0
	CLOCK_REVERSED_FALLING,				//!< CPOL 1, CPHA 0
	CLOCK_REVERSED_RISING				//!< CPOL 1, CPHA 1
}spi_clock_polarity_strobe_t;

#define SPI_ENDIAN_LSB	0
#define SPI_ENDIAN_MSB	1

#define SPI_TX_FROM_INTERNAL	1
#define SPI_TX_FROM_EXTERNAL	2

#define SPI_USE_INTERNAL_RX_BUF	0

/**
 * Current SPI reception state
 */
typedef enum spi_rx_state_t {
	SPI_RX_IDLE,			//!< SPI is configured and ready for operation
	SPI_RX_RXING,			//!< SPI is currently receiving
	SPI_RX_DONE,			//!< Reception is done
	SPI_RX_WAITING_FOR_RX,	//!< SPI is currently transmitting and reception is queued
	SPI_RX_ERROR_OVERRUN,	//!< Overrun had been detected and reception was terminated
	SPI_RX_ERROR_MODF,
}spi_rx_state_t;

/**
 * Current SPI transmission state
 */
typedef enum spi_tx_state_t {
	SPI_TX_IDLE,
	SPI_TX_TXING,
	SPI_TX_DONE,
	SPI_TX_ERROR_MODF
}spi_tx_state_t;

//! This function initializes SPI bus to work as full duplex two-wire connection, w/o DMA data flow
uint8_t spi_init_full_duplex_pio(spi_transfer_mode_t mode, spi_clock_polarity_strobe_t strobe, int speed, int endianess);

//! Sends data asynchronously to selected SPI slave, discards anything what will come in over MISO line
uint8_t spi_tx_data(uint32_t slave_id, uint8_t tx_from_internal, uint8_t * tx_buffer, uint16_t ln_to_tx);

//! Sends data synchronously to selected slave and after that receives data from it. Everything received before TX is finish is discarded.
uint8_t spi_rx_tx_data(uint32_t slave_id, uint8_t tx_from_internal, uint8_t * rx_buffer, uint8_t * tx_buffer, uint16_t ln_to_rx, uint16_t ln_to_tx);

//! Sends data to selected slave and in the same moment receive everything sent from slave. It receive exactly ln_to_exchange bytes
uint8_t spi_rx_tx_exchange_data(uint32_t slave_id, uint8_t tx_from_internal, uint8_t * rx_buffer, uint8_t * tx_buffer, uint16_t ln_to_exchange);

//! Returns pointer to receive buffer and if reception is done resets RX state back to idle.
uint8_t * spi_get_rx_data(void);

//! Blocking wait until both RX and TX is done
uint8_t spi_wait_for_comms_done(void);

//! Resets all errors and reverts state back to IDLE
void spi_reset_errors(void);

spi_rx_state_t spi_get_rx_state(void);
spi_tx_state_t spi_get_tx_state(void);

//! Returns an ID of current SPI slave or zero if SPI is idle
uint32_t spi_get_current_slave(void);

void spi_irq_handler(void);

#endif /* INCLUDE_DRIVERS_SPI_H_ */
