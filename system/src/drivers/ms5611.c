#include "drivers/ms5611.h"
#include "drivers/i2c.h"
#include "../include/delay.h"
#include "station_config.h"

#include "rte_wx.h"

// adres do zapisu: 0xEC
// adres do oczytu: 0xED

char state;	// zmienna sygnalizuj�ca przebieg pomiaru..
			// 0: zmierz temperature
			// 1: odczytaj temperature i zmierz ci�nienie
			// 2: odczytaj ci�nienie i zmierz temperature
			// sekwencja stan�w 0 -> 1 -> 2 -> 1 -> 2 -> 1 (...)

#define TX_ADDR 0xEE
#define RX_ADDR 0xEF

// An array to store Calibration data
int32_t SensorCalData[8];
double SensorDT = 0.0;

// This variable is set on bootup if the sensor is detected and correct calibration data have been read.
// If the initialization function will fail the driver will disable communication completely and no
// measuremenets will be returned
uint8_t ms5611_sensor_avaliable = 0;

// This function will reset the sensor. It should be ran as a first thing during sensor initialization
int32_t ms5611_reset(ms5611_qf_t *qf) {

	int out = MS5611_OK;

	// Preparing a buffer with 0x1E command ID which will reset the sensor
	int txbuf[] = {0x1E, '\0' };

	// Send a data to sensor
	i2c_send_data(TX_ADDR, txbuf, 0);

	// Wait until the transmission will finish or fail (due to timeout or any other error)
	while (i2c_state != I2C_IDLE && i2c_state != I2C_ERROR);

	// If reset was successfull enable a driver
	if (i2c_state == I2C_IDLE) {
		// Enable sensor comms
		ms5611_sensor_avaliable = 1;

		// wait for sensor reset
		delay_fixed(50);

	}
	else {
		// Set Quality Factor to unavaliable
		*qf = MS5611_QF_NOT_AVALIABLE;

		// Return with keeping 'ms5611_sensor_abaliable' set to zero which will
		// disable comms
		out = MS5611_SENSOR_NOT_AVALIABLE;
	}

	return out;
}

// Function will read a calibration data from sensor and chceck if the CRC is correct
int32_t ms5611_read_calibration(int32_t* cal_data, ms5611_qf_t *out) {

	// Check if sensor is avaliable
	if (ms5611_sensor_avaliable == 0) {
		// If not stop further actions
		*out = MS5611_QF_NOT_AVALIABLE;

		return MS5611_SENSOR_NOT_AVALIABLE;
	}

	int i,j;
	int txbuf[2];	
	j = 0;

	// Reading calibration constants one after another. Each constant is uint16_t, so a loop iterator
	// is incremeneted by 2
	for (i=0; i<=0xE; i+=2) {

		// 0xA0 is an address of first calibration constant in PROM
		txbuf[0] = 0xA0 + i;
		txbuf[1] = '\0'; 

		// Transmitting a command
		i2c_send_data(TX_ADDR, txbuf, 0);

		// Waiting to transmission completion or failure
		while (i2c_state != I2C_IDLE && i2c_state != I2C_ERROR);

		// Check if transmission was successfull
		if (i2c_state == I2C_ERROR) {
			// If not break calibtaion reading but don't touch 'sensor_avaliable'
			// so function could be called once again.
			*out = MS5611_QF_NOT_AVALIABLE;

			return MS5611_TIMEOUT_DURING_MEASURMENT;
		}

		// Receiving the data with calibration coefficient.
		i2c_receive_data(RX_ADDR, 2);

		// Waiting until receiving will be completed
		while (i2c_state != I2C_IDLE && i2c_state != I2C_ERROR);

		if (i2c_state == I2C_ERROR) {
			*out = MS5611_QF_NOT_AVALIABLE;

			return MS5611_TIMEOUT_DURING_MEASURMENT;
		}

		// Storing calibration coefficient into target array
		*(cal_data + j) = ((i2c_rx_data[0] << 8) | i2c_rx_data[1]);

		// increase an iterator used to walk through target array
		j++;
	}

	// Getting a CRC4 value from received data
	uint8_t rxed_crc = cal_data[7] & 0xF;

	// Calculating a CRC4 locally
	uint8_t calculated_crc = crc4(cal_data);

	if (rxed_crc == calculated_crc)					// sprawdzanie poprawno�ci odebranych danych
		return 0;
	else
		return -1;
}

// This is main function to trigger and receive raw value of measuremenets
int32_t ms5611_trigger_measure(int param_to_meas, int32_t* out) {
	int txbuf[] = { 0x00, 0x00};

	if (ms5611_sensor_avaliable == 0) {
		return MS5611_SENSOR_NOT_AVALIABLE;
	}

	if(param_to_meas == 0x00) {
		////////////////////////////////////////////
		//// TRIGGERING TEMPERATURE MESUREMENET ////
		////////////////////////////////////////////
		txbuf[0] = 0x54;						// oversampling 1024
		i2c_send_data(TX_ADDR,txbuf, 0);				// wys�anie rozkazu rozpocz�cia pomiaru

		while (i2c_state != I2C_IDLE && i2c_state != I2C_ERROR);

		if (i2c_state == I2C_IDLE)
			return MS5611_OK;
		else
			return MS5611_TIMEOUT_DURING_MEASURMENT;
	}
	else if(param_to_meas == 0x01) {
		/////////////////////////////
		//// READING TEMPERATURE ////
		/////////////////////////////
		txbuf[0] = 0x00;
		i2c_send_data(TX_ADDR,txbuf, 0x01);

		while (i2c_state != I2C_IDLE && i2c_state != I2C_ERROR);

		if (i2c_state == I2C_ERROR) {
			return MS5611_TIMEOUT_DURING_MEASURMENT;
		}

		i2c_receive_data(RX_ADDR, 3);

		while (i2c_state != I2C_IDLE && i2c_state != I2C_ERROR);

		if (i2c_state == I2C_ERROR) {
			return MS5611_TIMEOUT_DURING_MEASURMENT;
		}

		*out = ((i2c_rx_data[0] << 16) | (i2c_rx_data[1] << 8) | i2c_rx_data[2]);
		////////////////////////////////////////////
		//// TRIGGERING PRESSURE MEASUREMENET   ////
		////////////////////////////////////////////
		txbuf[0] = 0x44;
		i2c_send_data(TX_ADDR,txbuf, 0);

		while (i2c_state != I2C_IDLE && i2c_state != I2C_ERROR);

		if (i2c_state == I2C_ERROR) {
			return MS5611_TIMEOUT_DURING_MEASURMENT;
		}

		return MS5611_OK;
	}
	else if(param_to_meas == 0x02) {					// pomiar D2
		//////////////////////////
		//// READING PRESSURE ////
		//////////////////////////
		txbuf[0] = 0x00;
		i2c_send_data(TX_ADDR,txbuf, 0x01);			// wys�anie rozkazu odczytu wyniku

		while (i2c_state != I2C_IDLE && i2c_state != I2C_ERROR);

		if (i2c_state == I2C_ERROR) {
			return MS5611_TIMEOUT_DURING_MEASURMENT;
		}

		i2c_receive_data(RX_ADDR, 3);

		while (i2c_state != I2C_IDLE && i2c_state != I2C_ERROR);

		if (i2c_state == I2C_ERROR) {
			return MS5611_TIMEOUT_DURING_MEASURMENT;
		}

		*out = ((i2c_rx_data[0] << 16) | (i2c_rx_data[1] << 8) | i2c_rx_data[2]);
		/////////////////////////////////////////////
		//// TRIGGERING TEMPERATURE MEASUREMENET ////
		/////////////////////////////////////////////
		txbuf[0] = 0x54;
		i2c_send_data(TX_ADDR,txbuf, 0);

		while (i2c_state != I2C_IDLE && i2c_state != I2C_ERROR);

		if (i2c_state == I2C_ERROR) {
			return MS5611_TIMEOUT_DURING_MEASURMENT;
		}

		return MS5611_OK;
	}

	return MS5611_WRONG_PARAM_VALUE;
}

// This function will get the physical value of measured temperature
int32_t ms5611_get_temperature(float* out, ms5611_qf_t *qf) {
	int32_t raw = 0;

	double raw_temp, dt, temp;
	float output;

	int32_t return_val = 0;

	// Check if sensor is avaliable
	if (ms5611_sensor_avaliable == 0) {
		*qf = MS5611_QF_NOT_AVALIABLE;

		return MS5611_SENSOR_NOT_AVALIABLE;
	}

	// Read temperature value and trigger pressure meas in the same time
	return_val = ms5611_trigger_measure(0x01, &raw);

	if (return_val != MS5611_OK) {
		*qf = MS5611_QF_NOT_AVALIABLE;

		return MS5611_TIMEOUT_DURING_MEASURMENT;
	}

	// copy conversion result to local float variable
	raw_temp = raw;

	// use calibation coefficient to convert to physical value
	dt = raw_temp - SensorCalData[5] * pow(2,8);
	temp = 2000 + dt * SensorCalData[5] / pow(2,23);
	output = (float)temp;
	output /= 100;
	SensorDT = dt;

	// store physical value in output variable
	*out = output;

	// Check if the physical value of temperature lies within the range which reflects
	// the achievable in reality values. This check is done to protect against situation
	// when the sensor is responsible on an i2c bus, but the measured values are
	// incorrect due to sensor damage or reading the value before the measurement is
	// done.
	if (output > MS5611_MIN_TEMPERATURE_OK && output < MS5611_MAX_TEMPERATURE_OK) {
		*qf = MS5611_QF_FULL;
		return MS5611_OK;
	}
	else {
		*qf = MS5611_QF_DEGRADATED;
		return MS5611_SENSOR_NOT_AVALIABLE;
	}



}

// This function will get the physical vale of pressure and then trigger temperature meas.
int32_t ms5611_get_pressure(float* out, ms5611_qf_t *qf) {
	int32_t raw = 0;

	int32_t return_val = 0;

	long long int raw_p, off, sens, p;	// int64_t
	float output_p;

	// Check if sensor is avaliable
	if (ms5611_sensor_avaliable == 0) {
		*qf = MS5611_QF_NOT_AVALIABLE;

		return MS5611_SENSOR_NOT_AVALIABLE;
	}

	return_val = ms5611_trigger_measure(0x02, &raw);

	if (return_val != MS5611_OK) {
		*qf = MS5611_QF_NOT_AVALIABLE;
		//ms5611_sensor_avaliable = 0;

		return MS5611_TIMEOUT_DURING_MEASURMENT;
	}

	 raw_p = raw;

	off = SensorCalData[2] * pow(2,16) + (SensorCalData[4] * SensorDT) / pow(2,7);
	sens = SensorCalData[1] * pow(2,15) + (SensorCalData[3] * SensorDT) / pow(2,8);
	p = (raw_p * sens / pow(2,21) - off) / pow(2,15);
	output_p = (double)p;
	output_p /= 100;

	*out = output_p;

	if (output_p > MS5611_MIN_PRESSURE_OK && output_p < MS5611_MAX_PRESSURE_OK) {
		*qf = MS5611_QF_FULL;
		return MS5611_OK;
	}
	else {
		*qf = MS5611_QF_DEGRADATED;
		return MS5611_SENSOR_NOT_AVALIABLE;
	}


}


unsigned char crc4(int32_t* n_prom)
{
	int cnt; // simple counter
	unsigned int n_rem; // crc reminder
	unsigned int crc_read; // original value of the crc
	unsigned char n_bit;
	n_rem = 0x00;
	crc_read=n_prom[7]; //save read CRC
	n_prom[7]=(0xFF00 & (n_prom[7])); //CRC byte is replaced by 0
	for (cnt = 0; cnt < 16; cnt++) // operation is performed on bytes
	{// choose LSB or MSB
		if (cnt%2 == 1) n_rem ^= (unsigned short) ((n_prom[cnt>>1]) & 0x00FF);
		else n_rem ^= (unsigned short) (n_prom[cnt>>1]>>8);
		for (n_bit = 8; n_bit > 0; n_bit--)
		{
			if (n_rem & (0x8000))
			{
				n_rem = (n_rem << 1) ^ 0x3000;
			}
			else
			{
				n_rem = (n_rem << 1);
			}
		}
	}
	n_rem= (0x000F & (n_rem >> 12)); // final 4-bit reminder is CRC code
	n_prom[7]=crc_read; // restore the crc_read to its original place
	return (n_rem ^ 0x0);
}


float CalcQNHFromQFE(float qfe, float alti, float temp) {

	float hprim = 8000 * ( (1 + 0.004f * temp) / qfe);
	double p = qfe + (alti/hprim);


	float psr = (qfe + p) * 0.5f;
	float tpm = temp + (0.6f * alti) / 100.0f;

	float tsr = (temp + tpm)/2.0f;
	hprim = 8000 * ((1 + 0.004f * tsr) / psr);
	float qnh = qfe + (alti/hprim);

	return qnh;
}
