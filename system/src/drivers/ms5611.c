#include "drivers/ms5611.h"
#include "drivers/i2c.h"
#include "../include/delay.h"

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

int32_t SensorCalData[8];
double SensorDT = 0.0;

uint8_t ms5611_sensor_avaliable = 0;

// resetowanie sensora i pobieranie jego danych kalibracyjnych
int32_t ms5611_reset(ms5611_qf_t *qf) {

	int txbuf[] = {0x1E, '\0' };				// komenda 0x1E resetuj�ca czujnik 

	i2cSendData(TX_ADDR, txbuf, 0);				// wys�anie danych pod adres 0xEC czyli do czujnika

	while (i2c_state != I2C_IDLE && i2c_state != I2C_ERROR);

	// if reset was successfull enable a driver
	if (i2c_state == I2C_IDLE) {
		ms5611_sensor_avaliable = 1;

		// wait for sensor reset
		delay_fixed(50);

	}
	else {
		*qf = MS5611_QF_NOT_AVALIABLE;

		return MS5611_SENSOR_NOT_AVALIABLE;
	}

	return MS5611_OK;
}

int32_t ms5611_read_calibration(int32_t* cal_data, ms5611_qf_t *out) {

	if (ms5611_sensor_avaliable == 0) {
		*out = MS5611_QF_NOT_AVALIABLE;

		return MS5611_SENSOR_NOT_AVALIABLE;
	}

	int i,j;
	int txbuf[2];	
	int rxbuf[] = {0x00, 0x00};
	j = 0;
	for (i=0; i<=0xE; i+=2) {

		//

		txbuf[0] = 0xA0 + i;					// 0xA0 to adres pierwszej sta�ej kalibracyjnej, ka�da z nich ma 16 bit�w
		txbuf[1] = '\0'; 
		i2cSendData(TX_ADDR, txbuf, 0);			// wysy�anie adresu do odczytania

		while (i2c_state != I2C_IDLE && i2c_state != I2C_ERROR);

		if (i2c_state == I2C_ERROR) {
			*out = MS5611_QF_NOT_AVALIABLE;

			return MS5611_TIMEOUT_DURING_MEASURMENT;
		}

		i2cReceiveData(RX_ADDR, rxbuf, 2);			// odbi�r danych z czujnika

		while (i2c_state != I2C_IDLE && i2c_state != I2C_ERROR);

		if (i2c_state == I2C_ERROR) {
			*out = MS5611_QF_NOT_AVALIABLE;

			return MS5611_TIMEOUT_DURING_MEASURMENT;
		}

		*(cal_data + j) = ((i2cRXData[0] << 8) | i2cRXData[1]);		// przepisywanie danych z bufor�w do tablicy
		j++;
	}

	uint8_t rxed_crc = cal_data[7] & 0xF;
	uint8_t calculated_crc = crc4(cal_data);

	if (rxed_crc == calculated_crc)					// sprawdzanie poprawno�ci odebranych danych
		return 0;
	else
		return -1;
}

int32_t ms5611_trigger_measure(int param_to_meas, int32_t* out) {
	int txbuf[] = { 0x00, 0x00};
	long int output;

	if (ms5611_sensor_avaliable == 0) {
		return MS5611_SENSOR_NOT_AVALIABLE;
	}

	if(param_to_meas == 0x00) {
		////////////////////////////
		//// POMIAR TEMPERATURY ////
		////////////////////////////
		txbuf[0] = 0x54;						// oversampling 1024
		i2cSendData(TX_ADDR,txbuf, 0);				// wys�anie rozkazu rozpocz�cia pomiaru

		while (i2c_state != I2C_IDLE && i2c_state != I2C_ERROR);

		if (i2c_state == I2C_IDLE)
			return MS5611_OK;
		else
			return MS5611_TIMEOUT_DURING_MEASURMENT;
	}
	else if(param_to_meas == 0x01) {					// pomiar D1
		////////////////////////////
		//// ODCZYT TEMPERATURY ////
		////////////////////////////
		txbuf[0] = 0x00;
		i2cSendData(TX_ADDR,txbuf, 0x01);			// wys�anie rozkazu odczytu wyniku

		while (i2c_state != I2C_IDLE && i2c_state != I2C_ERROR);

		if (i2c_state == I2C_ERROR) {
			return MS5611_TIMEOUT_DURING_MEASURMENT;
		}

		i2cReceiveData(RX_ADDR, txbuf, 3);

		while (i2c_state != I2C_IDLE && i2c_state != I2C_ERROR);

		if (i2c_state == I2C_ERROR) {
			return MS5611_TIMEOUT_DURING_MEASURMENT;
		}

		*out = ((i2cRXData[0] << 16) | (i2cRXData[1] << 8) | i2cRXData[2]);
		////////////////////////////
		//// POMIAR CI�NIENIA   ////
		////////////////////////////
		txbuf[0] = 0x44;						// oversampling 1024
		i2cSendData(TX_ADDR,txbuf, 0);				// wys�anie rozkazu rozpocz�cia pomiaru

		while (i2c_state != I2C_IDLE && i2c_state != I2C_ERROR);

		if (i2c_state == I2C_ERROR) {
			return MS5611_TIMEOUT_DURING_MEASURMENT;
		}

		return MS5611_OK;
	}
	else if(param_to_meas == 0x02) {					// pomiar D2
		//////////////////////////
		//// ODCZYT CI�NIENIA ////
		//////////////////////////
		txbuf[0] = 0x00;
		i2cSendData(TX_ADDR,txbuf, 0x01);			// wys�anie rozkazu odczytu wyniku

		while (i2c_state != I2C_IDLE && i2c_state != I2C_ERROR);

		if (i2c_state == I2C_ERROR) {
			return MS5611_TIMEOUT_DURING_MEASURMENT;
		}

		i2cReceiveData(RX_ADDR, txbuf, 3);

		while (i2c_state != I2C_IDLE && i2c_state != I2C_ERROR);

		if (i2c_state == I2C_ERROR) {
			return MS5611_TIMEOUT_DURING_MEASURMENT;
		}

		*out = ((i2cRXData[0] << 16) | (i2cRXData[1] << 8) | i2cRXData[2]);
		////////////////////////////
		//// POMIAR TEMPERATURY ////
		////////////////////////////
		txbuf[0] = 0x54;						// oversampling 1024
		i2cSendData(TX_ADDR,txbuf, 0);				// wys�anie rozkazu rozpocz�cia pomiaru

		while (i2c_state != I2C_IDLE && i2c_state != I2C_ERROR);

		if (i2c_state == I2C_ERROR) {
			return MS5611_TIMEOUT_DURING_MEASURMENT;
		}

		return MS5611_OK;
	}

	return MS5611_WRONG_PARAM_VALUE;
}

// pomiar temperatury
int32_t ms5611_get_temperature(float* out, ms5611_qf_t *qf) {
	int32_t raw = 0;

	double raw_temp, dt, temp;
	float output;

	int32_t return_val = 0;

	if (ms5611_sensor_avaliable == 0) {
		//return_val = ms5611_reset(qf);

		//if (return_val == MS5611_OK)
		//	ms5611_sensor_avaliable = 1;
		//else {
			*qf = MS5611_QF_NOT_AVALIABLE;

			return MS5611_SENSOR_NOT_AVALIABLE;
		//}
	}

	return_val = ms5611_trigger_measure(0x01, &raw);

	if (return_val != MS5611_OK) {
		*qf = MS5611_QF_NOT_AVALIABLE;
		//ms5611_sensor_avaliable = 0;

		return MS5611_TIMEOUT_DURING_MEASURMENT;
	}

	raw_temp = raw;

	dt = raw_temp - SensorCalData[5] * pow(2,8);
	temp = 2000 + dt * SensorCalData[5] / pow(2,23);
	output = (float)temp;
	output /= 100;
	SensorDT = dt;

	*out = output;

	if (output > MS5611_MIN_TEMPERATURE_OK && output < MS5611_MAX_TEMPERATURE_OK) {
		*qf = MS5611_QF_FULL;
		return MS5611_OK;
	}
	else {
		*qf = MS5611_QF_DEGRADATED;
		return MS5611_SENSOR_NOT_AVALIABLE;
	}



}

// pomiar ci�nienia
int32_t ms5611_get_pressure(float* out, ms5611_qf_t *qf) {
	int32_t raw = 0;

	int32_t return_val = 0;

	long long int raw_p, off, sens, p;	// int64_t
	float output_p;

	if (ms5611_sensor_avaliable == 0) {
		//return_val = ms5611_reset(qf);

		//if (return_val == MS5611_OK)
		//	ms5611_sensor_avaliable = 1;
		//else {
			*qf = MS5611_QF_NOT_AVALIABLE;

			return MS5611_SENSOR_NOT_AVALIABLE;
		//}
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

// funkcja obliczaj�ca sum� kontroln� CRC4 z wsp�czynnik�w kalibracyjncyh
unsigned char crc4(int n_prom[])
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
if (cnt%2==1) n_rem ^= (unsigned short) ((n_prom[cnt>>1]) & 0x00FF);
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
//	float qfe = 1001.9f;
//	float alti = 114.0f;
//	float temp = 26.6f;

	float hprim = 8000 * ( (1 + 0.004f * temp) / qfe);
//	printf("hprim: %f\r\n", hprim);
	double p = qfe + (alti/hprim);
//	printf("p: %f\r\n", p);


	float psr = (qfe + p) * 0.5f;
//	printf("psr: %f\r\n", psr);
	float tpm = temp + (0.6f * alti) / 100.0f;
//	printf("tpm: %f\r\n", tpm);

	float tsr = (temp + tpm)/2.0f;
//	printf("tsr: %f\r\n", tsr);
	hprim = 8000 * ((1 + 0.004f * tsr) / psr);
//	printf("new hprim: %f\r\n", hprim);
	float qnh = qfe + (alti/hprim);
//	float qnh = (845.58f / pow (2.7182818d, (-0.000127605011d * 1500) ));

	return qnh;
}
