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

#include "station_config.h"

#include <string.h>
#include <stdio.h>

#include "tm_stm32_rtc.h"

typedef struct wx_history_entry_t {
	float temperature;
	uint32_t timestamp;
	uint16_t seconds_from_last_frame;
}wx_history_entry_t;

#define WX_HISTORY_LN		4

static wx_history_entry_t wx_history[WX_HISTORY_LN] = {0u};

static uint8_t wx_history_iterator = 0;

static TM_RTC_t wx_last_datetime;				//!< Last time weather pilot has been sent


void SendWXFrame(uint16_t windspeed, uint16_t windgusts, uint16_t winddirection, float temperatura, float cisnienie, uint8_t humidity) {

	float max_wind_speed = 0.0f, temp = 0.0f;
	uint8_t wind_speed_mph = 0, wind_gusts_mph = 0;

	TM_RTC_GetDateTime(&wx_last_datetime, TM_RTC_Format_BIN);

	wx_history[wx_history_iterator].temperature = temperatura;
	wx_history[wx_history_iterator++].timestamp = wx_last_datetime.Unix;

	if (wx_history_iterator > WX_HISTORY_LN) {
		wx_history_iterator = 0;
	}

	for (short i = 0; i < WX_HISTORY_LN; i++) {
		if (wx_history[i].timestamp == 0) {
			continue;
		}
		else {
			wx_history[i].seconds_from_last_frame = (uint16_t)(wx_last_datetime.Unix - wx_history[i].timestamp);
		}
	}

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

 	memset(main_own_aprs_msg, 0x00, sizeof(main_own_aprs_msg));

 	// 	  main_own_aprs_msg_len = sprintf(main_own_aprs_msg, "=%s%c%c%s%c%c %s", main_string_latitude, main_config_data_basic->n_or_s, main_symbol_f, main_string_longitude, main_config_data_basic->e_or_w, main_symbol_s, main_config_data_basic->comment);
	main_own_aprs_msg_len = sprintf(main_own_aprs_msg, "_%02d%02d%02d%02dc...s...g...t%03d[0_%d_%03d][1_%d_%03d][2_%d_%03d][3_%d_%03d]",
										wx_last_datetime.Month, wx_last_datetime.Day, wx_last_datetime.Hours, wx_last_datetime.Minutes,
										/*temperatura */(short)(temperatura*1.8+32),
										wx_history[0].seconds_from_last_frame, (uint16_t)(wx_history[0].temperature * 10.0f),
										wx_history[1].seconds_from_last_frame, (uint16_t)(wx_history[1].temperature * 10.0f),
										wx_history[2].seconds_from_last_frame, (uint16_t)(wx_history[2].temperature * 10.0f),
										wx_history[3].seconds_from_last_frame, (uint16_t)(wx_history[3].temperature * 10.0f));
	main_own_aprs_msg[main_own_aprs_msg_len] = 0;
 	ax25_sendVia(&main_ax25, main_own_path, main_own_path_ln, main_own_aprs_msg, main_own_aprs_msg_len);
	after_tx_lock = 1;
	WAIT_FOR_CHANNEL_FREE();
 	afsk_txStart(&main_afsk);
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

	return;
}



