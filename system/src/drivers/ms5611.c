#include "drivers/ms5611.h"
#include "drivers/i2c.h"

// adres do zapisu: 0xEC
// adres do oczytu: 0xED

char state;	// zmienna sygnalizuj�ca przebieg pomiaru..
			// 0: zmierz temperature
			// 1: odczytaj temperature i zmierz ci�nienie
			// 2: odczytaj ci�nienie i zmierz temperature
			// sekwencja stan�w 0 -> 1 -> 2 -> 1 -> 2 -> 1 (...)


int SensorCalData[8];
double SensorDT = 0.0;

// resetowanie sensora i pobieranie jego danych kalibracyjnych
void SensorReset(int addr) {
	int txbuf[] = {0x1E, '\0' };				// komenda 0x1E resetuj�ca czujnik 
	while(i2cTXing != 0 && i2cRXing !=0);		// je�eli magistala i2c nie jest zaj�ta, tj nie nadaj� i nie odbiera
	i2cSendData(0xEC, txbuf, 0);				// wys�anie danych pod adres 0xEC czyli do czujnika
	while(i2cTXing != 0);						// czekanie na zako�czenie transmisji
//	Delay10ms();								// oczekiwanie 10ms, tyle miej wi�cej trwa resetowanie czujnika
//	SensorReadCalData(0xEC, SensorCalData);		// odczytywanie danych kalibracyjnych
}

int SensorReadCalData(int addr, int* cal_data) {
	int i,j;
	int txbuf[2];	
	int rxbuf[] = {0x00, 0x00};
	j = 0;
	for (i=0; i<=0xE; i+=2) {
		while(i2cTXing != 0 && i2cRXing !=0);	// je�eli magistrala nie jest zaj�ta
		txbuf[0] = 0xA0 + i;					// 0xA0 to adres pierwszej sta�ej kalibracyjnej, ka�da z nich ma 16 bit�w
		txbuf[1] = '\0'; 
		i2cSendData(0xEC, txbuf, 0);			// wysy�anie adresu do odczytania
		while(i2cTXing != 0);					// oczekiwanie na zako�czenie transmisji
		i2cReceiveData(0xED, rxbuf, 2);			// odbi�r danych z czujnika
		while(i2cRXing != 0);
		*(cal_data + j) = ((i2cRXData[0] << 8) | i2cRXData[1]);		// przepisywanie danych z bufor�w do tablicy
		j++;
	}
	if (crc4(cal_data) == 0x08)					// sprawdzanie poprawno�ci odebranych danych
		return 0;
	else
		return -1;
}

long int SensorStartMeas(int param_to_meas) {
	int txbuf[] = { 0x00, 0x00};
	long int output;
	if(param_to_meas == 0x00) {
		////////////////////////////
		//// POMIAR TEMPERATURY ////
		////////////////////////////
		txbuf[0] = 0x54;						// oversampling 1024
		i2cSendData(0xEC,txbuf, 0);				// wys�anie rozkazu rozpocz�cia pomiaru
		while (i2cTXing != 0);
		return 0;
	}
	else if(param_to_meas == 0x01) {					// pomiar D1
		////////////////////////////
		//// ODCZYT TEMPERATURY ////
		////////////////////////////
		txbuf[0] = 0x00;
		i2cSendData(0xEC,txbuf, 0x01);			// wys�anie rozkazu odczytu wyniku
		while (i2cTXing != 0);
		i2cReceiveData(0xED, txbuf, 3);
		while (i2cRXing != 0);
		output = ((i2cRXData[0] << 16) | (i2cRXData[1] << 8) | i2cRXData[2]); 
		////////////////////////////
		//// POMIAR CI�NIENIA   ////
		////////////////////////////
		txbuf[0] = 0x44;						// oversampling 1024
		i2cSendData(0xEC,txbuf, 0);				// wys�anie rozkazu rozpocz�cia pomiaru
		while (i2cTXing != 0);
//		Delay10ms();							// oczekiwanie na zako�czenie pomiaru
		return output;
	}
	else if(param_to_meas == 0x02) {					// pomiar D2
		//////////////////////////
		//// ODCZYT CI�NIENIA ////
		//////////////////////////
		txbuf[0] = 0x00;
		i2cSendData(0xEC,txbuf, 0x01);			// wys�anie rozkazu odczytu wyniku
		while (i2cTXing != 0);
		i2cReceiveData(0xED, txbuf, 3);
		while (i2cRXing != 0);
		output = ((i2cRXData[0] << 16) | (i2cRXData[1] << 8) | i2cRXData[2]);
		////////////////////////////
		//// POMIAR TEMPERATURY ////
		////////////////////////////
		txbuf[0] = 0x54;						// oversampling 1024
		i2cSendData(0xEC,txbuf, 0);				// wys�anie rozkazu rozpocz�cia pomiaru
		while (i2cTXing != 0);
//		Delay10ms();							// oczekiwanie na zako�czenie pomiaru
		return output;
	}
	return -1;
}

// pomiar temperatury
float SensorBringTemperature(void) {
	double raw_temp, dt, temp;
	float output;
	raw_temp = SensorStartMeas(0x01);
	dt = raw_temp - SensorCalData[5] * pow(2,8);
	temp = 2000 + dt * SensorCalData[5] / pow(2,23);
	output = (float)temp;
	output /= 100;
	SensorDT = dt;
	return output;


}

// pomiar ci�nienia
double SensorBringPressure(void) {
	long long int raw_p, off, sens, p;
	float output_p;
	raw_p = SensorStartMeas(0x02);
	off = SensorCalData[2] * pow(2,16) + (SensorCalData[4] * SensorDT) / pow(2,7);
	sens = SensorCalData[1] * pow(2,15) + (SensorCalData[3] * SensorDT) / pow(2,8);
	p = (raw_p * sens / pow(2,21) - off) / pow(2,15);
	output_p = (double)p;
	output_p /= 100;
//	output_p += 435;
	return output_p;
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
