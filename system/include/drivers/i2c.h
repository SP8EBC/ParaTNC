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
int i2cSendData(int addr, int* data, int null);
int i2cReceiveData(int addr, int* data, int num);
void i2cIrqHandler(void);
void i2cErrIrqHandler(void);
void i2cVariableReset(void);

void i2cStop(void);
void i2cStart(void);

void i2cKeepTimeout(void);

//void I2C1_EV_IRQHandler(void);
//void I2C1_ER_IRQHandler(void);

extern volatile i2c_state_t i2c_state;

extern volatile uint16_t i2cRemoteAddr;
extern volatile uint8_t i2cTXData[32];
extern volatile uint8_t i2cRXData[32];
extern volatile uint8_t i2cRXing;
extern volatile uint8_t i2cTXing;
extern volatile uint8_t i2cDone;
extern volatile uint8_t i2cTXQueueLen;
extern volatile uint8_t i2cTRXDataCounter;
extern volatile uint8_t i2cRXBytesNumber;
extern volatile uint8_t i2cErrorCounter;

/* C++ detection */
#ifdef __cplusplus
}
#endif

#endif
