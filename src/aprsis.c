/*
 * aprsis.c
 *
 *  Created on: Feb 20, 2022
 *      Author: mateusz
 */

#include "aprsis.h"
#include "main.h"

#include <stdio.h>
#include <string.h>

srl_context_t * aprsis_serial_port;

gsm_sim800_state_t * aprsis_gsm_modem_state;

/**
 * Buffer to store string representation of callsign with SSID
 */
char aprsis_callsign_with_ssid[10];

/**
 * Buffer for sending packet to aprs-is
 */
char aprsis_packet_tx_buffer[512];

/**
 *
 */
uint16_t aprsis_packet_tx_message_size = 0;

/**
 * Passocde to APRS-IS server
 */
int32_t aprsis_passcode;

/**
 * Buffer for generate and then permanently store login string
 */
char aprsis_login_string[64];

/**
 * Default APRS-IS address to be used by
 */
const char * aprsis_default_server_address;

uint16_t aprsis_default_server_address_ln = 0;

uint16_t aprsis_default_server_port;

/**
 * Set to one if connections is established AND user is logged
 */
uint8_t aprsis_logged = 0;

/**
 * Set to one if connection to server is established (but maybe not logged)
 */
uint8_t aprsis_connected = 0;

const char * aprsis_sucessfull_login = "# logresp\0";

/**
 * A timestamp when server has send anything
 */
uint32_t aprsis_last_keepalive_ts = 0;

#define APRSIS_TIMEOUT_MS	123000//123000

void aprsis_init(srl_context_t * context, gsm_sim800_state_t * gsm_modem_state, char * callsign, uint8_t ssid, uint32_t passcode, char * default_server, uint16_t default_port) {
	aprsis_serial_port = context;

	aprsis_gsm_modem_state = gsm_modem_state;

	aprsis_passcode = (int32_t)passcode;

	sprintf(aprsis_callsign_with_ssid, "%s-%d", callsign, ssid);

	memset(aprsis_login_string, 0x00, 0x40);

	sprintf(aprsis_login_string, "user %s pass %ld vers ParaMETEO %s \r\n", aprsis_callsign_with_ssid, aprsis_passcode, SW_VER);

	aprsis_logged = 0;

	aprsis_default_server_port = default_port;

	aprsis_default_server_address = default_server;

	aprsis_default_server_address_ln = strlen(aprsis_default_server_address);

}

uint8_t aprsis_connect_and_login(char * address, uint8_t address_ln, uint16_t port) {
	// this function has blocking io
	uint8_t out = APRSIS_WRONG_STATE;

	if (aprsis_logged == 1) {
		return APRSIS_ALREADY_CONNECTED;
	}

	if (aprsis_serial_port == 0 || aprsis_gsm_modem_state == 0) {
		return APRSIS_NOT_CONFIGURED;
	}

	if (*aprsis_gsm_modem_state == SIM800_ALIVE) {

		char port_str[6];

		uint8_t * receive_buff;

		int8_t retval = 0xFF;

		uint8_t offset = 0;

		memset(port_str, 0x00, 0x6);

		snprintf(port_str, 6, "%d", port);

		// connecting has blocking I/O
		retval = gsm_sim800_tcpip_connect(address, address_ln, port_str, 0x6, aprsis_serial_port, aprsis_gsm_modem_state);

		// if connection was successful
		if (retval == 0) {
			// wait for hello message '# aprsc 2.1.10-gd72a17c'
			retval = gsm_sim800_tcpip_receive(0, 0, aprsis_serial_port, aprsis_gsm_modem_state, 0, 2000);

			if (retval == 0) {
				receive_buff = srl_get_rx_buffer(aprsis_serial_port);

				// if hello message has been received
				if (*receive_buff == '#' && *(receive_buff + 1) == ' ') {
					// send long string to server
					gsm_sim800_tcpip_write((uint8_t *)aprsis_login_string, strlen(aprsis_login_string), aprsis_serial_port, aprsis_gsm_modem_state);

					// wait for server response
					retval = gsm_sim800_tcpip_receive(0, 0, aprsis_serial_port, aprsis_gsm_modem_state, 0, 2000);

					if (retval == 0) {
						receive_buff = srl_get_rx_buffer(aprsis_serial_port);

						aprsis_connected = 1;

						// fast forward to beginning of response
						for (offset = 0; offset < 8; offset++) {
							if (*(receive_buff + offset) == '#') {
								break;
							}
						}

						// check if authorization has been successfull
						retval = strncmp(aprsis_sucessfull_login, (const char * )(receive_buff + offset), (size_t)9);
						if (retval == 0) {
							aprsis_logged = 1;

							aprsis_send_beacon(0);

							// wait for consecutive data
							gsm_sim800_tcpip_async_receive(aprsis_serial_port, aprsis_gsm_modem_state, 0, 61000, aprsis_receive_callback);

							out = APRSIS_OK;

						}
						else {
							// if authoruzation wasn't successfull drop a connection
							gsm_sim800_tcpip_close(aprsis_serial_port, aprsis_gsm_modem_state, 0);
						}
					}
				}
				else {
					gsm_sim800_tcpip_close(aprsis_serial_port, aprsis_gsm_modem_state, 0);
				}
			}
			else {
				gsm_sim800_tcpip_close(aprsis_serial_port, aprsis_gsm_modem_state, 1);
			}
		}
	}

	return out;

}

uint8_t aprsis_connect_and_login_default(void) {

	return aprsis_connect_and_login(aprsis_default_server_address, aprsis_default_server_address_ln, aprsis_default_server_port);
}

void aprsis_disconnect(void) {
	gsm_sim800_tcpip_close(aprsis_serial_port, aprsis_gsm_modem_state, 0);

	aprsis_logged = 0;

	aprsis_connected = 0;
}

void aprsis_receive_callback(srl_context_t* srl_context) {

	// if something was actually received
	if (srl_context->srl_rx_state == SRL_RX_DONE) {
		// check if this is keepalive message
		if (*(srl_get_rx_buffer(srl_context)) == '#') {
			aprsis_last_keepalive_ts = main_get_master_time();

			gsm_sim800_tcpip_async_receive(aprsis_serial_port, aprsis_gsm_modem_state, 0, 61000, aprsis_receive_callback);
		}
		else {

		}
	}
}

void aprsis_check_alive(void) {

	uint32_t timestamp = 0;

	timestamp = master_time;

	// check if connection is alive
	if (aprsis_logged == 1 && (timestamp > (aprsis_last_keepalive_ts + APRSIS_TIMEOUT_MS))) {
		// reset the flag
		aprsis_logged = 0;

		gsm_sim800_tcpip_close(aprsis_serial_port, aprsis_gsm_modem_state, 1);
	}
}

void aprsis_send_wx_frame(uint16_t windspeed, uint16_t windgusts, uint16_t winddirection, float temperatura, float cisnienie, uint8_t humidity) {

	if (aprsis_logged == 0) {
		return;
	}

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

 	pressure = (unsigned)(cisnienie * 10);

 	memset(aprsis_packet_tx_buffer, 0x00, sizeof(aprsis_packet_tx_buffer));
 	// 	  main_own_aprs_msg_len = sprintf(main_own_aprs_msg, "=%s%c%c%s%c%c %s", main_string_latitude, main_config_data_basic->n_or_s, main_symbol_f, main_string_longitude, main_config_data_basic->e_or_w, main_symbol_s, main_config_data_basic->comment);
 	aprsis_packet_tx_message_size = sprintf(aprsis_packet_tx_buffer, "%s>AKLPRZ,qAR,%s:!%s%c%c%s%c%c%03d/%03dg%03dt%03dr...p...P...b%05ldh%02d\r\n", aprsis_callsign_with_ssid, aprsis_callsign_with_ssid, main_string_latitude, main_config_data_basic->n_or_s, '/', main_string_longitude, main_config_data_basic->e_or_w, '_', /* kierunek */direction, /* predkosc*/(int)wind_speed_mph, /* porywy */(short)(wind_gusts_mph), /*temperatura */(short)(temperatura*1.8+32), pressure, humidity);
 	aprsis_packet_tx_buffer[aprsis_packet_tx_message_size] = 0;

 	gsm_sim800_tcpip_async_write((uint8_t *)aprsis_packet_tx_buffer, aprsis_packet_tx_message_size, aprsis_serial_port, aprsis_gsm_modem_state);
}

void aprsis_send_beacon(uint8_t async) {

	if (aprsis_logged == 0) {
		return;
	}

	aprsis_packet_tx_message_size = sprintf(aprsis_packet_tx_buffer, "%s>AKLPRZ,qAR,%s:=%s%c%c%s%c%c %s\r\n", aprsis_callsign_with_ssid, aprsis_callsign_with_ssid, main_string_latitude, main_config_data_basic->n_or_s, main_symbol_f, main_string_longitude, main_config_data_basic->e_or_w, main_symbol_s, main_config_data_basic->comment);
	  aprsis_packet_tx_buffer[aprsis_packet_tx_message_size] = 0;

	  if (async > 0) {
		  gsm_sim800_tcpip_async_write((uint8_t *)aprsis_packet_tx_buffer, aprsis_packet_tx_message_size, aprsis_serial_port, aprsis_gsm_modem_state);
	  }
	  else {
		 	gsm_sim800_tcpip_write((uint8_t *)aprsis_packet_tx_buffer, aprsis_packet_tx_message_size, aprsis_serial_port, aprsis_gsm_modem_state);
	  }
}
