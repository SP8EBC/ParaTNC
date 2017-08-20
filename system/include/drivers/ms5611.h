#ifndef __MS5611_H
#define __MS5611_H

#include <math.h>

/* C++ detection */
#ifdef __cplusplus
extern "C" {
#endif


void SensorReset(int addr);
int SensorReadCalData(int addr, int* cal_data);
unsigned char crc4(int n_prom[]);
long int SensorStartMeas(int param_to_meas);
float SensorBringTemperature(void);
double SensorBringPressure(void);


extern char state;
extern int SensorCalData[8];


/* C++ detection */
#ifdef __cplusplus
}
#endif

#endif

