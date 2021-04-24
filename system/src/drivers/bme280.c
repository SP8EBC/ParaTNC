/*
 * bma150.c
 *
 *  Created on: 21.06.2020
 *      Author: mateusz
 */


#include <drivers/bme280.h>
#include "../drivers/i2c.h"

#include <string.h>

#define RESET_MAGIC_WORD 0xB6

#define TX_ADDR 0xEC // 11101100
#define RX_ADDR 0xED // 11101101

#define MODE 						3	// Normal aka 'cyclic' mode
#define FILTER_COEFFICIENT_LN 		16
#define HUMIDITY_OVERSAMPLING		4	// oversampling x8
#define PRESSURE_OVERSAMPLING		3	// oversampling x4
#define TEMPERATURE_OVERSAMPLING	3	// oversampling x4
#define STANDBY_TIME				5	// one second

#define CTRL_HUM_ADDR		0xF2
#define STATUS_ADDR			0xF3
#define CTRL_MEAS_ADDR		0xF4
#define CONFIG_ADDR			0xF5
#define RESET_ADDR			0xE0

#define PRESS_MSB_ADDR		0xF7
#define VALUES_LN			8

#define CALIB00_ADDR		0x88
#define CALIB00_READ_LN		0x1A

#define CALIB26_ADDR		0xE1
#define CALIB26_READ_LN		0x10

uint8_t bme280_sensor_avaliable = 0;

uint8_t bme280_data_buffer[BME280_LN_RAW_DATA + 1];
uint8_t bme280_calibration_data[BME280_LN_CALIBRATION + 1];

int32_t t_fine = 0;

/**
 * This function resets the BMA150 sensor to initial state. After it is called
 * the sensor must be reconfigured once again.
 */
int32_t bme280_reset(bme280_qf_t* qf) {
	int32_t out = BME280_OK;

	// i2c transmit buffer
	uint8_t tx_buf[] = {RESET_ADDR, RESET_MAGIC_WORD, 0};

	// Send a data to sensor
	i2c_send_data(TX_ADDR, tx_buf, 0);

	// Wait until the transmission will finish or fail (due to timeout or any other error)
	while (i2c_state != I2C_IDLE && i2c_state != I2C_ERROR);

	// If reset was successfull enable a driver
	if (i2c_state == I2C_IDLE) {
		// Enable sensor comms
		bme280_sensor_avaliable = 1;

		// wait for sensor reset
		delay_fixed(50);

	}
	else {
		// Set Quality Factor to unavaliable
		*qf = BME280_QF_NOT_AVAILABLE;

		// Return with keeping 'ms5611_sensor_abaliable' set to zero which will
		// disable comms
		return BME280_SENSOR_NOT_RESPONDING;
	}

	return out;
}

int32_t bme280_setup(void) {
	int32_t out = BME280_OK;

	// local variables to represent the data wrote to configuration registers
	uint8_t ctrl_meas = 0;
	uint8_t ctrl_hum = 0;
	uint8_t config = 0;

	if (bme280_sensor_avaliable == 0) {
		return BME280_SENSOR_NOT_AVALIABLE;
	}

	// setting humidity measurement configutation
	ctrl_hum = HUMIDITY_OVERSAMPLING;

	// set mode, and oversampling for humidity and temperature
	ctrl_meas = MODE | (PRESSURE_OVERSAMPLING << 2) | (TEMPERATURE_OVERSAMPLING << 5);

	// set standby time and filter lenght
	config = (FILTER_COEFFICIENT_LN << 2) | (STANDBY_TIME << 5);

	// transmit buffer
	uint8_t tx_buff[] = {CTRL_HUM_ADDR, ctrl_hum, CTRL_MEAS_ADDR, ctrl_meas, CONFIG_ADDR, config, 0};

	// Send a data to sensor
	i2c_send_data(TX_ADDR, tx_buff, 0);

	// Wait until the transmission will finish or fail (due to timeout or any other error)
	while (i2c_state != I2C_IDLE && i2c_state != I2C_ERROR);

	// If reset was successfull enable a driver
	if (i2c_state == I2C_IDLE) {
		;
	}
	else {
		out = BME280_SENSOR_NOT_RESPONDING;
	}

	return out;
}

int32_t bme280_read_calibration(uint8_t* calibration) {
	int32_t out = BME280_OK;

	if (bme280_sensor_avaliable == 0) {
		return BME280_SENSOR_NOT_AVALIABLE;
	}

	// transmit buffer
	uint8_t tx_buf[] = {CALIB00_ADDR, 0, 0};

	// Send a data to sensor
	i2c_send_data(TX_ADDR, tx_buf, 0);

	// Wait until the transmission will finish or fail (due to timeout or any other error)
	while (i2c_state != I2C_IDLE && i2c_state != I2C_ERROR);

	// check if transmission was successfull
	if (i2c_state == I2C_IDLE) {
		;
	}
	else {

		return BME280_SENSOR_NOT_RESPONDING;
	}

	// clearing receive buffer
	memset(calibration, 0x00, BME280_LN_CALIBRATION);

	// reading first segment of calibration data
	i2c_receive_data(RX_ADDR, CALIB00_READ_LN);

	// Wait until receiving will finish or fail (due to timeout or any other error)
	while (i2c_state != I2C_IDLE && i2c_state != I2C_ERROR);

	// check if receive was successfull
	if (i2c_state == I2C_IDLE) {
		;
	}
	else {

		return BME280_SENSOR_NOT_RESPONDING;
	}

	// copying read data
	memcpy(calibration, (uint8_t*)i2c_rx_data, CALIB00_READ_LN);

	// preparing the buffer to receive second segment of calibration data
	tx_buf[0] = CALIB26_ADDR;

	// Send the second request to the sensor
	i2c_send_data(TX_ADDR, tx_buf, 0);

	// Wait until the transmission will finish or fail (due to timeout or any other error)
	while (i2c_state != I2C_IDLE && i2c_state != I2C_ERROR);

	// check if transmission was successfull
	if (i2c_state == I2C_IDLE) {
		;
	}
	else {

		return BME280_SENSOR_NOT_RESPONDING;
	}

	// reading second segment of calibration data
	i2c_receive_data(RX_ADDR, CALIB26_READ_LN);

	// Wait until receiving will finish or fail (due to timeout or any other error)
	while (i2c_state != I2C_IDLE && i2c_state != I2C_ERROR);

	// check if receive was successfull
	if (i2c_state == I2C_IDLE) {
		;
	}
	else {

		return BME280_SENSOR_NOT_RESPONDING;
	}

	// copying read data
	memcpy(calibration + CALIB00_READ_LN, (uint8_t*)i2c_rx_data, CALIB26_READ_LN);

	return out;
}

int32_t bme280_read_raw_data(uint8_t* raw_data) {
	int32_t out = BME280_OK;

	if (bme280_sensor_avaliable == 0) {
		return BME280_SENSOR_NOT_AVALIABLE;
	}

	// transmit buffer
	uint8_t tx_buf[] = {PRESS_MSB_ADDR, 0, 0};

	// Send a data to sensor
	i2c_send_data(TX_ADDR, tx_buf, 0);

	// Wait until the transmission will finish or fail (due to timeout or any other error)
	while (i2c_state != I2C_IDLE && i2c_state != I2C_ERROR);

	// check if transmission was successfull
	if (i2c_state == I2C_IDLE) {
		;
	}
	else {

		return BME280_SENSOR_NOT_RESPONDING;
	}

	// clearing receive buffer
	memset(raw_data, 0x00, BME280_LN_RAW_DATA);

	// reading first segment of calibration data
	i2c_receive_data(RX_ADDR, BME280_LN_RAW_DATA);

	// Wait until receiving will finish or fail (due to timeout or any other error)
	while (i2c_state != I2C_IDLE && i2c_state != I2C_ERROR);

	// check if receive was successfull
	if (i2c_state == I2C_IDLE) {
		;
	}
	else {
		return BME280_SENSOR_NOT_RESPONDING;
	}

	// copying read data
	memcpy(raw_data, (uint8_t*)i2c_rx_data, BME280_LN_RAW_DATA);

	return out;
}

int32_t bme280_get_pressure(float* out, uint32_t raw_data, bme280_qf_t* qf) {
	int32_t ret = BME280_OK;

	int32_t var1, var2;
	uint32_t p;

	uint32_t adc_P = raw_data;

	var1 = (((int32_t)t_fine)>>1) - (int32_t)64000;
	var2 = (((var1>>2) * (var1>>2)) >> 11 ) * ((int32_t)bme280_get_dig_P6());
	var2 = var2 + ((var1*((int32_t)bme280_get_dig_P5()))<<1);
	var2 = (var2>>2)+(((int32_t)bme280_get_dig_P4())<<16);
	var1 = (((bme280_get_dig_P3() * (((var1>>2) * (var1>>2)) >> 13 )) >> 3) + ((((int32_t)bme280_get_dig_P2()) * var1)>>1))>>18;
	var1 =((((32768+var1))*((int32_t)bme280_get_dig_P1()))>>15);
	if (var1 == 0)
	{
		*qf = BME280_QF_PRESSURE_DEGRADED;
		return BME280_WRONG_PRESSURE_READOUT; // avoid exception caused by division by zero
	}
		p = (((uint32_t)(((int32_t)1048576)-adc_P)-(var2>>12)))*3125;
	if (p < 0x80000000)
	{
		p = (p << 1) / ((uint32_t)var1);
	}
	else
	{
		p = (p / (uint32_t)var1) * 2;
	}
	var1 = (((int32_t)bme280_get_dig_P9()) * ((int32_t)(((p>>3) * (p>>3))>>13)))>>12;
	var2 = (((int32_t)(p>>2)) * ((int32_t)bme280_get_dig_P8()))>>13;
	p = (uint32_t)((int32_t)p + ((var1 + var2 + bme280_get_dig_P7()) >> 4));

	*out = p / 100.0f;

	return ret;
}

int32_t bme280_get_temperature(float* out, uint32_t raw_data, bme280_qf_t* qf) {
	int32_t ret = BME280_OK;

	uint32_t adc_T = raw_data;

	int32_t var1, var2, T;
    var1 = (((( adc_T >> 3 ) - ((int32_t)bme280_get_dig_T1() << 1))) * ((int32_t)bme280_get_dig_T2())) >> 11;
    var2 = (((((adc_T >> 4 ) - ((int32_t)bme280_get_dig_T1())) * ((adc_T>>4) - ((int32_t)bme280_get_dig_T1()))) >> 12) * ((int32_t)bme280_get_dig_T3())) >> 14;

	t_fine = var1 + var2;
	T = (t_fine * 5 + 128) >> 8;

	*out = (T / 100.0f);

	return ret;
}

int32_t bme280_get_humidity(int8_t* out, uint16_t raw_data, bme280_qf_t* qf) {
	int32_t ret = BME280_OK;

	int32_t v_x1_u32r;
	uint32_t val = 0;

	int32_t adc_H = raw_data;

	v_x1_u32r = (t_fine - ((int32_t)76800));
	v_x1_u32r = (((((adc_H << 14) - (((int32_t)bme280_get_dig_H4()) << 20) - (((int32_t)bme280_get_dig_H5()) * v_x1_u32r)) +
	((int32_t)16384)) >> 15) * (((((((v_x1_u32r * ((int32_t)bme280_get_dig_H6())) >> 10) * (((v_x1_u32r *
	((int32_t)bme280_get_dig_H3())) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) *
	((int32_t)bme280_get_dig_H2()) + 8192) >> 14));
	v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((int32_t)bme280_get_dig_H1())) >> 4));
	v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
	v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
	val = (uint32_t)(v_x1_u32r>>12);

	*out = (val / 1024);

	if (*out > 99 || *out < 0) {
		*qf = BME280_QF_HUMIDITY_DEGRADED;
		ret = BME280_WRONG_HUMIDITY_READOUT;
	}

	return ret;
}
