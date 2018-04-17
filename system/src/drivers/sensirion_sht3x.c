/*
 * sensirion_sht3x.c
 *
 *  Created on: 02.01.2018
 *      Author: mateusz
 */


#include "drivers/sensirion_sht3x.h"

void sht3x_start_measurement(void) {
	uint32_t txbuff[3] = {0, 0, 0};
	txbuff[0] = (uint32_t)(SHT3X_SS_LOW_NOSTREACH & 0xFF00) >> 8;
	txbuff[1] = (uint32_t)SHT3X_SS_LOW_NOSTREACH & 0xFF;

	while(i2cTXing != 0 && i2cRXing !=0);		// je�eli magistala i2c nie jest zaj�ta, tj nie nadaj� i nie odbiera
	i2cSendData(SHT3X_WADDR, txbuff, 0);				// wys�anie danych pod adres 0xEC czyli do czujnika
	while(i2cTXing != 0);
 }

void sht3x_read_measurements(uint16_t *temperature, uint16_t *humidity) {
//	int rxbuff[6];
//	memset(rxbuff, 0x00, 6);

	uint16_t temperature_raw = 0;
	uint16_t humidity_raw = 0;

	float temperature_phy = 0;
	float himidity_phy = 0;

	while(i2cTXing != 0 && i2cRXing !=0);		// je�eli magistala i2c nie jest zaj�ta, tj nie nadaj� i nie odbiera
	i2cReceiveData(SHT3X_RADDR, 0, 9);			// odbi�r danych z czujnika
	while(i2cRXing !=0);		// je�eli magistala i2c nie jest zaj�ta, tj nie nadaj� i nie odbiera

	temperature_raw = ((i2cRXData[0] << 8) | i2cRXData[1]);
	humidity_raw = ((i2cRXData[3] << 8) | i2cRXData[4]);

	temperature_phy = -45.0f + 175.0f * (float)( temperature_raw / 65535.0f);

	*temperature = (uint16_t)temperature_phy;

	return;
}

