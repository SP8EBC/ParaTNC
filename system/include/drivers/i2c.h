#ifndef __I2C_H
#define __I2C_H

#include <stdint.h>

/* C++ detection */
#ifdef __cplusplus
extern "C" {
#endif

typedef enum i2c_state {
	I2C_IDLE,
	I2C_RXING,
	I2C_TXING,
	I2C_ERROR
}i2c_state_t;

void i2cConfigure(void);
int i2c_send_data(int addr, uint8_t* data, int null);
int i2c_receive_data(int addr, int num);
void i2cIrqHandler(void);
void i2cErrIrqHandler(void);
void i2cVariableReset(void);

void i2cStop(void);
void i2cStart(void);

void i2cKeepTimeout(void);

//void I2C1_EV_IRQHandler(void);
//void I2C1_ER_IRQHandler(void);

extern volatile i2c_state_t i2c_state;

extern volatile uint16_t i2c_remote_addr;
extern volatile uint8_t i2c_tx_data[32];
extern volatile uint8_t i2c_rx_data[32];
extern volatile uint8_t i2c_rxing;
extern volatile uint8_t i2c_txing;
extern volatile uint8_t i2c_done;
extern volatile uint8_t i2c_tx_queue_len;
extern volatile uint8_t i2c_trx_data_counter;
extern volatile uint8_t i2c_rx_bytes_number;
extern volatile uint8_t i2c_error_counter;

/* C++ detection */
#ifdef __cplusplus
}
#endif

#endif
