/*
 * bma150.h
 *
 *  Created on: 21.06.2020
 *      Author: mateusz
 */

#ifndef INCLUDE_DRIVERS_BMA150_H_
#define INCLUDE_DRIVERS_BMA150_H_

#include "stdint.h"

#define BMA150_OK						0
#define BMA150_SENSOR_NOT_RESPONDING	-1
#define BMA150_SENSOR_NOT_AVALIABLE		-2

#define BMA150_LN_CALIBRATION 	41
#define BMA150_LN_RAW_DATA		8

typedef enum bma150_qf {
	BMA150_QF_FULL,
	BMA150_QF_NOT_AVAILABLE
}bma150_qf_t;

extern uint8_t bma150_data_buffer[BMA150_LN_CALIBRATION + 1];
extern uint8_t bma150_calibration_data[BMA150_LN_RAW_DATA + 1];

int32_t bma150_reset(bma150_qf_t* qf);
int32_t bma150_setup(void);
int32_t bma150_read_calibration(uint8_t* calibration, bma150_qf_t* qf);
int32_t bma150_read_raw_data(uint8_t* raw_data, bma150_qf_t* qf);

int32_t bma150_get_pressure(float* out, uint8_t* raw_data);
int32_t bma150_get_temperature(float* out, uint8_t* raw_data);
int32_t bma150_get_humidity(int8_t* out, uint8_t* raw_data);


#endif /* INCLUDE_DRIVERS_BMA150_H_ */
