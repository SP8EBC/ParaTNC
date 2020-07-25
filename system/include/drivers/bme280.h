/*
 * bma150.h
 *
 *  Created on: 21.06.2020
 *      Author: mateusz
 */

#ifndef INCLUDE_DRIVERS_BME280_H_
#define INCLUDE_DRIVERS_BME280_H_

#include "stdint.h"

#define BME280_OK						0
#define BME280_SENSOR_NOT_RESPONDING	-1
#define BME280_SENSOR_NOT_AVALIABLE		-2
#define BME280_WRONG_PRESSURE_READOUT	-3
#define BME280_WRONG_HUMIDITY_READOUT	-4

#define BME280_LN_CALIBRATION 	41
#define BME280_LN_RAW_DATA		8

typedef enum bme280_qf {
	BME280_QF_FULL,
	BME280_QF_NOT_AVAILABLE,
	BME280_QF_HUMIDITY_DEGRADED,
	BME280_QF_PRESSURE_DEGRADED,
	BME280_QF_GEN_DEGRADED,
	BME280_QF_UKNOWN
}bme280_qf_t;

extern uint8_t bme280_data_buffer[BME280_LN_RAW_DATA + 1];
extern uint8_t bme280_calibration_data[BME280_LN_CALIBRATION + 1];

int32_t bme280_reset(bme280_qf_t* qf);
int32_t bme280_setup(void);
int32_t bme280_read_calibration(uint8_t* calibration);
int32_t bme280_read_raw_data(uint8_t* raw_data);

int32_t bme280_get_pressure(float* out, uint32_t raw_data, bme280_qf_t* qf);
int32_t bme280_get_temperature(float* out, uint32_t raw_data, bme280_qf_t* qf);
int32_t bme280_get_humidity(int8_t* out, uint16_t raw_data, bme280_qf_t* qf);

//#define BME280_CONCAT_BYTES(msb, lsb)            (((uint16_t)msb << 8) | (uint16_t)lsb)

inline uint32_t bme280_get_adc_t(void) {
	return (bme280_data_buffer[3] << 12) | (bme280_data_buffer[4] << 4) | (bme280_data_buffer[5] >> 4);
}

inline uint32_t bme280_get_adc_p(void) {
	return (bme280_data_buffer[0] << 12) | (bme280_data_buffer[1] << 4) | (bme280_data_buffer[2] >> 4);

}

inline uint16_t bme280_get_adc_h(void) {
	return (bme280_data_buffer[6] << 8) | (bme280_data_buffer[7] );
}


inline uint16_t bme280_get_dig_T1(void) {
	return ((uint16_t)bme280_calibration_data[0] | (uint16_t)bme280_calibration_data[1] << 8);
}

inline int16_t bme280_get_dig_T2(void) {
	return (uint16_t)bme280_calibration_data[2] | (uint16_t)bme280_calibration_data[3] << 8;

}

inline int16_t bme280_get_dig_T3(void) {
	return (uint16_t)bme280_calibration_data[4] | (uint16_t)bme280_calibration_data[5] << 8;
}

inline uint16_t bme280_get_dig_P1(void) {
	return ((uint16_t)bme280_calibration_data[6] | (uint16_t)bme280_calibration_data[7] << 8);

}

inline int16_t bme280_get_dig_P2(void) {
	return ((uint16_t)bme280_calibration_data[8] | (uint16_t)bme280_calibration_data[9] << 8);

}

inline int16_t bme280_get_dig_P3(void) {
	return ((uint16_t)bme280_calibration_data[10] | (uint16_t)bme280_calibration_data[11] << 8);

}

inline int16_t bme280_get_dig_P4(void) {
	return ((uint16_t)bme280_calibration_data[12] | (uint16_t)bme280_calibration_data[13] << 8);

}

inline int16_t bme280_get_dig_P5(void) {
	return ((uint16_t)bme280_calibration_data[14] | (uint16_t)bme280_calibration_data[15] << 8);

}

inline int16_t bme280_get_dig_P6(void) {
	return ((uint16_t)bme280_calibration_data[16] | (uint16_t)bme280_calibration_data[17] << 8);

}

inline int16_t bme280_get_dig_P7(void) {
	return ((uint16_t)bme280_calibration_data[18] | (uint16_t)bme280_calibration_data[19] << 8);

}

inline int16_t bme280_get_dig_P8(void) {
	return ((uint16_t)bme280_calibration_data[20] | (uint16_t)bme280_calibration_data[21] << 8);

}

inline int16_t bme280_get_dig_P9(void) {
	return ((uint16_t)bme280_calibration_data[22] | (uint16_t)bme280_calibration_data[23] << 8);

}

inline uint8_t bme280_get_dig_H1(void) {
	return bme280_calibration_data[25];
}

inline int16_t bme280_get_dig_H2(void) {	//
	return ((uint16_t)bme280_calibration_data[26] | (uint16_t)bme280_calibration_data[27] << 8);

}

inline uint8_t bme280_get_dig_H3(void) {
	return bme280_calibration_data[28];

}

inline int16_t bme280_get_dig_H4(void) {
    int16_t dig_h4_lsb;
    int16_t dig_h4_msb;
    dig_h4_msb = (int16_t)(int8_t)bme280_calibration_data[29] * 16;
    dig_h4_lsb = (int16_t)(bme280_calibration_data[30] & 0x0F);
	return (dig_h4_msb | dig_h4_lsb);

}

inline int16_t bme280_get_dig_H5(void) {
    int16_t dig_h5_lsb;
    int16_t dig_h5_msb;
    dig_h5_msb = (int16_t)(int8_t)bme280_calibration_data[31] * 16;
    dig_h5_lsb = (int16_t)(bme280_calibration_data[30] >> 4);
	return (dig_h5_msb | dig_h5_lsb);

}

inline int8_t bme280_get_dig_H6(void) {
	return bme280_calibration_data[32];

}


#endif /* INCLUDE_DRIVERS_BME280_H_ */
