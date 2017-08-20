#ifndef __I2C_H
#define __I2C_H



/* C++ detection */
#ifdef __cplusplus
extern "C" {
#endif


void i2cConfigure(void);
int i2cSendData(int addr, int* data, int null);
int i2cReceiveData(int addr, int* data, int num);
void I2C1_EV_IRQHandler(void);
void I2C1_ER_IRQHandler(void);
void i2cVariableReset(void);


extern volatile int i2cRemoteAddr;		
extern volatile int i2cTXData[32];
extern volatile int i2cRXData[32];
extern volatile int i2cRXing;			
extern volatile int i2cTXing;			
extern volatile int i2cDone;			
extern volatile int i2cTXQueueLen;		
extern volatile int i2cTRXDataCounter;	
extern volatile int i2cRXBytesNumber;	
extern volatile int i2cErrorCounter;	

/* C++ detection */
#ifdef __cplusplus
}
#endif

#endif
