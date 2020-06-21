/*
 * bma150.c
 *
 *  Created on: 21.06.2020
 *      Author: mateusz
 */


#include "../drivers/bma150.h"
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

#define CALIB00_ADDR		0x88
#define CALIB00_READ_LN		0x1A

#define CALIB26_ADDR		0xE1
#define CALIB26_READ_LN		0x10

uint8_t bma150_sensor_avaliable = 0;

uint8_t bma150_data_buffer[BMA150_LN_CALIBRATION + 1];
uint8_t bma150_calibration_data[BMA150_LN_RAW_DATA + 1];

/**
 * This function resets the BMA150 sensor to initial state. After it is called
 * the sensor must be reconfigured once again.
 */
int32_t bma150_reset(bma150_qf_t* qf) {
	int32_t out = BMA150_OK;

	// i2c transmit buffer
	uint8_t tx_buf[] = {RESET_ADDR, RESET_MAGIC_WORD, 0};

	// Send a data to sensor
	i2c_send_data(TX_ADDR, tx_buf, 0);

	// Wait until the transmission will finish or fail (due to timeout or any other error)
	while (i2c_state != I2C_IDLE && i2c_state != I2C_ERROR);

	// If reset was successfull enable a driver
	if (i2c_state == I2C_IDLE) {
		// Enable sensor comms
		bma150_sensor_avaliable = 1;

		// wait for sensor reset
		delay_fixed(50);

	}
	else {
		// Set Quality Factor to unavaliable
		*qf = BMA150_QF_NOT_AVAILABLE;

		// Return with keeping 'ms5611_sensor_abaliable' set to zero which will
		// disable comms
		return BMA150_SENSOR_NOT_RESPONDING;
	}

	return out;
}

int32_t bma150_setup(void) {
	int32_t out = BMA150_OK;

	// local variables to represent the data wrote to configuration registers
	uint8_t ctrl_meas = 0;
	uint8_t ctrl_hum = 0;
	uint8_t config = 0;

	if (bma150_sensor_avaliable == 0) {
		return BMA150_SENSOR_NOT_AVALIABLE;
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
		out = BMA150_SENSOR_NOT_RESPONDING;
	}

	return out;
}

int32_t bma150_read_calibration(uint8_t* calibration, bma150_qf_t* qf) {
	int32_t out = BMA150_OK;

	if (bma150_sensor_avaliable == 0) {
		return BMA150_SENSOR_NOT_AVALIABLE;
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
		*qf = BMA150_QF_NOT_AVAILABLE;

		return BMA150_SENSOR_NOT_RESPONDING;
	}

	// clearing receive buffer
	memset(calibration, 0x00, BMA150_LN_CALIBRATION + 1);

	// reading first segment of calibration data
	i2c_receive_data(RX_ADDR, CALIB00_READ_LN);

	// Wait until receiving will finish or fail (due to timeout or any other error)
	while (i2c_state != I2C_IDLE && i2c_state != I2C_ERROR);

	// check if receive was successfull
	if (i2c_state == I2C_IDLE) {
		;
	}
	else {
		*qf = BMA150_QF_NOT_AVAILABLE;

		return BMA150_SENSOR_NOT_RESPONDING;
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
		*qf = BMA150_QF_NOT_AVAILABLE;

		return BMA150_SENSOR_NOT_RESPONDING;
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
		*qf = BMA150_QF_NOT_AVAILABLE;

		return BMA150_SENSOR_NOT_RESPONDING;
	}

	// copying read data
	memcpy(calibration + CALIB00_READ_LN, (uint8_t*)i2c_rx_data, CALIB26_READ_LN);

	return out;
}

int32_t bma150_read_raw_data(uint8_t* raw_data, bma150_qf_t* qf) {
	int32_t out = BMA150_OK;

	if (bma150_sensor_avaliable == 0) {
		return BMA150_SENSOR_NOT_AVALIABLE;
	}

	return out;
}

int32_t bma150_get_pressure(float* out, uint8_t* raw_data) {
	int32_t ret = BMA150_OK;

	return ret;
}

int32_t bma150_get_temperature(float* out, uint8_t* raw_data) {
	int32_t ret = BMA150_OK;

	return ret;
}

int32_t bma150_get_humidity(int8_t* out, uint8_t* raw_data) {
	int32_t ret = BMA150_OK;

	return ret;
}
