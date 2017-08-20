#ifndef __AFSK_PR_H
#define __AFSK_PR_H

#ifndef __AFSK_PR



#define __SERIAL
#include "drivers/serial.h"

//#define __USER_INTERF
//#include "user_interf.h"

#include <string.h>
#include <math.h>

//int DeStuffing(uint8_t bity);

#endif

// #ifdef __AFSK_PR

extern unsigned char nrzi_bity;

#ifdef __cplusplus
extern "C"
{
#endif

void ADCStartConfig(void);
void DACStartConfig(void);
//int ADCSendViaSerial(unsigned short int adc_val);
//int SampleData(short int input);
//void InitFilter(void);
//int SynchroNRZI(unsigned char bit);
//int DeStuffing(uint8_t bity);

#ifdef __cplusplus
}
#endif


// #endif


#endif
