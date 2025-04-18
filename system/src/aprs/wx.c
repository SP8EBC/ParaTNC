/*
 * wx.c
 *
 *  Created on: 28.05.2017
 *      Author: mateusz
 */

#include "aprs/wx.h"
#include "aprs/digi.h"
#include "main.h"
#include "rte_main.h"

#include "supervisor.h"

#include "station_config.h"

#include <string.h>
#include <stdio.h>


void SendWXFrame(uint16_t windspeed, uint16_t windgusts, uint16_t winddirection, float temperatura, float cisnienie, uint8_t humidity) {

	float max_wind_speed = 0.0f, temp = 0.0f;
	uint8_t wind_speed_mph = 0, wind_gusts_mph = 0;
	uint32_t pressure = 0;

	uint16_t direction = winddirection;

	// windspeed is stored as an increment of 0.1 meters per second in 16bit unsigned integer
	temp =   ((float)windspeed / 10.0f);
	max_wind_speed =  ((float)windgusts / 10.0f);

	temp /= 0.45;																						// Konwersja na mile na godzine
	max_wind_speed /= 0.45;
	if ((temp - (short)temp) >= 0.5)												// Zaokraglanie wartosci
		/* Odejmuje od wartosci zmiennoprzecinkowej w milach nad godzine wartosc
			 po zrzutowaniu na typ short czyli z odcienta czescia po przecinku.
			 Jezeli wynik jest wiekszy albo rowny 0.5 to trzeba zaokraglic w gore */
		wind_speed_mph = (short)temp + 1;
	else
		/* A jezeli jest mniejsza niz 0.5 to zaokraglamy w dol */
		wind_speed_mph = (short)temp;
	if ((max_wind_speed - (short)max_wind_speed) >= 0.5)
		/* Analogiczna procedura ma miejsce dla porywow wiatru*/
		wind_gusts_mph = (short)max_wind_speed + 1;
	else
		wind_gusts_mph = (short)max_wind_speed;

 	if (wind_speed_mph > wind_gusts_mph) {
 		return;
 	}

 	pressure = (unsigned)(cisnienie * 10);

 	memset(main_own_aprs_msg, 0x00, sizeof(main_own_aprs_msg));

 	// 	  main_own_aprs_msg_len = sprintf(main_own_aprs_msg, "=%s%c%c%s%c%c %s", main_string_latitude, main_config_data_basic->n_or_s, main_symbol_f, main_string_longitude, main_config_data_basic->e_or_w, main_symbol_s, main_config_data_basic->comment);
	main_own_aprs_msg_len = sprintf(main_own_aprs_msg, "!%s%c%c%s%c%c%03d/%03dg%03dt%03dr...p...P...b%05ldh%02d", main_string_latitude, main_config_data_basic->n_or_s, '/', main_string_longitude, main_config_data_basic->e_or_w, '_', /* kierunek */direction, /* predkosc*/(int)wind_speed_mph, /* porywy */(short)(wind_gusts_mph), /*temperatura */(short)(temperatura*1.8+32), pressure, humidity);
	main_own_aprs_msg[main_own_aprs_msg_len] = 0;
 	ax25_sendVia(&main_ax25, main_own_path, main_own_path_ln, main_own_aprs_msg, main_own_aprs_msg_len);
	after_tx_lock = 1;
	WAIT_FOR_CHANNEL_FREE();
 	afsk_txStart(&main_afsk);

	supervisor_iam_alive(SUPERVISOR_THREAD_SEND_WX);
}


void SendWXFrameToKissBuffer(uint16_t windspeed, uint16_t windgusts, uint16_t winddirection, float temperatura, float cisnienie, uint8_t humidity, uint8_t* buffer, uint16_t buffer_ln, uint16_t* output_ln) {

	uint16_t output_frame_ln = 0;

	float max_wind_speed = 0.0f, temp = 0.0f;
	uint8_t wind_speed_mph = 0, wind_gusts_mph = 0;
	uint32_t pressure = 0;

	uint16_t direction = winddirection;

	// windspeed is stored as an increment of 0.1 meters per second in 16bit unsigned integer
	temp =  (float) (windspeed / 10.0f);
	max_wind_speed = (float) (windgusts / 10.0f);

	temp /= 0.45;																						// Konwersja na mile na godzine
	max_wind_speed /= 0.45;
	if ((temp - (short)temp) >= 0.5)												// Zaokraglanie wartosci
		/* Odejmuje od wartosci zmiennoprzecinkowej w milach nad godzine wartosc
			 po zrzutowaniu na typ short czyli z odcienta czescia po przecinku.
			 Jezeli wynik jest wiekszy albo rowny 0.5 to trzeba zaokraglic w gore */
		wind_speed_mph = (short)temp + 1;
	else
		/* A jezeli jest mniejsza niz 0.5 to zaokraglamy w dol */
		wind_speed_mph = (short)temp;
	if ((max_wind_speed - (short)max_wind_speed) >= 0.5)
		/* Analogiczna procedura ma miejsce dla porywow wiatru*/
		wind_gusts_mph = (short)max_wind_speed + 1;
	else
		wind_gusts_mph = (short)max_wind_speed;

 	pressure = (unsigned)(cisnienie * 10);

	main_own_aprs_msg_len = sprintf(main_own_aprs_msg, "!%s%c%c%s%c%c%03d/%03dg%03dt%03dr...p...P...b%05ldh%02d", main_string_latitude, main_config_data_basic->n_or_s, '/', main_string_longitude, main_config_data_basic->e_or_w, '_', /* kierunek */direction, /* predkosc*/(int)wind_speed_mph, /* porywy */(short)(wind_gusts_mph), /*temperatura */(short)(temperatura*1.8+32), pressure, humidity);
	main_own_aprs_msg[main_own_aprs_msg_len] = 0;

	output_frame_ln = ax25_sendVia_toBuffer(main_own_path, main_own_path_ln, main_own_aprs_msg, main_own_aprs_msg_len, buffer, buffer_ln);
	*output_ln = output_frame_ln;

	supervisor_iam_alive(SUPERVISOR_THREAD_SEND_WX);

	return;
}



