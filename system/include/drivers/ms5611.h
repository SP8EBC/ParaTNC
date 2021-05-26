#ifndef __MS5611_H
#define __MS5611_H

#include <math.h>

#include "stdint.h"

/* C++ detection */
#ifdef __cplusplus
extern "C" {
#endif

#define MS5611_MIN_PRESSURE_OK	600.0f
#define MS5611_MAX_PRESSURE_OK	1100.0f

#define MS5611_MIN_TEMPERATURE_OK	-40.0f
#define MS5611_MAX_TEMPERATURE_OK	70.0f

#define MS5611_WRONG_PARAM_VALUE	-3
#define MS5611_TIMEOUT_DURING_MEASURMENT	-2
#define MS5611_SENSOR_NOT_AVALIABLE -1
#define MS5611_OK	0

typedef enum ms5611_qf {
	MS5611_QF_UNKNOWN = 0,
	MS5611_QF_FULL = 1,
	MS5611_QF_NOT_AVALIABLE = 2,
	MS5611_QF_DEGRADATED = 3
}ms5611_qf_t;

int32_t ms5611_reset(ms5611_qf_t *qf);
int32_t ms5611_read_calibration(int32_t* cal_data, ms5611_qf_t* qf);
unsigned char crc4(int32_t* n_prom);
int32_t ms5611_trigger_measure(int param_to_meas, int32_t* out);
int32_t ms5611_get_temperature(float* out, ms5611_qf_t* qf);
int32_t ms5611_get_pressure(float* out, ms5611_qf_t *qf);

float CalcQNHFromQFE(float qfe, float alti, float temp);


extern char state;
extern int32_t SensorCalData[8];


/* C++ detection */
#ifdef __cplusplus
}
#endif

#endif

