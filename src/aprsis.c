/*
 * aprsis.c
 *
 *  Created on: Feb 20, 2022
 *      Author: mateusz
 */

#include "aprsis.h"
#include "main.h"
#include "text.h"

#include "etc/aprsis_config.h"

#include "gsm/sim800c.h"

#include <stdio.h>
#include <string.h>

srl_context_t * aprsis_serial_port;

/**
 * Pointer to used gsm_modem_state
 */
gsm_sim800_state_t * aprsis_gsm_modem_state;

/**
 * Size of transmit buffer
 */
#define APRSIS_TX_BUFFER_LN	512

/**
 * Buffer for sending packet to aprs-is
 */
char aprsis_packet_tx_buffer[APRSIS_TX_BUFFER_LN];

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
 * Default APRS-IS address to be used by 'aprsis_connect_and_login_default' function
 */
const char * aprsis_default_server_address;

/**
 * String with a callsign and a ssid in aprsis format. Used sent auto beacon
 * when connection to APRS-IS server is established
 */
const char * aprsis_callsign_with_ssid;

/**
 * Lenght of APRS-IS server address string
 */
uint16_t aprsis_default_server_address_ln = 0;

/**
 * TCP port used to connect to default APRS-IS server.
 */
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
 * Counter of unsuccessful connects to APRS-IS, to trigger GSM modem reset.
 * Please note that it works differently that 'aprsis_reset_on_timeout' and
 * has nothing to do with a timeouts of already established connection. This
 * counter will trigger GSM modem reset even if no APRS-IS connection has
 * been established at all. It protects against a situation when GSM modem
 * is not able to register in cellular network, SIM card is not working for
 * some reason etc. Of course there is no guarantee that a reset in such
 * case will help, but there is nothing better to do.
 */
uint8_t aprsis_unsucessfull_conn_counter = 0;

/**
 * A timestamp when server has send anything
 */
uint32_t aprsis_last_keepalive_ts = 0;

/**
 * Set to one if the GSM modem shall be immediately reset when APRS-IS communication
 * time out. Please note that this is different that 'aprsis_unsucessfull_conn_counter'
 * and reseting GSM mode after many unsuccessfull connection attempts! This
 * comes in when the connection has been established at least one time. It won't
 * help if there is some problem with establishing connection at all.
 */
uint32_t aprsis_reset_on_timeout = 0;

#define APRSIS_TIMEOUT_MS	123000//123000

/**
 * Lenght of 6 letter callsign + two digit SSID + end character like '>' or ","
 */
#define MAXIMUM_CALL_SSID_DASH_LN	10

void aprsis_init(
		srl_context_t * context,
		gsm_sim800_state_t * gsm_modem_state,
		const char * callsign,
		uint8_t ssid,
		uint32_t passcode,
		const char * default_server,
		const uint16_t default_port,
		uint8_t reset_on_timeout,
		const char * callsign_with_ssid) {
	aprsis_serial_port = context;

	aprsis_gsm_modem_state = gsm_modem_state;

	aprsis_passcode = (int32_t)passcode;

	aprsis_callsign_with_ssid = callsign_with_ssid;

	memset(aprsis_login_string, 0x00, 0x40);

	sprintf(aprsis_login_string, "user %s pass %ld vers ParaMETEO %s \r\n", callsign_with_ssid, aprsis_passcode, SW_VER);

	aprsis_logged = 0;

	aprsis_default_server_port = default_port;

	aprsis_default_server_address = default_server;

	aprsis_default_server_address_ln = strlen(aprsis_default_server_address);

	aprsis_reset_on_timeout = reset_on_timeout;

}

/**
 * Connect and login to APRS-IS server
 * @param address	ip or dns address to aprs-is server
 * @param address_ln	lenht of a buffer with an address
 * @param port	TCP port to use (typically 14580)
 * @param auto_send_beacon
 * @return
 */
aprsis_return_t aprsis_connect_and_login(const char * address, uint8_t address_ln, uint16_t port, uint8_t auto_send_beacon) {
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

		int offset = 0;

		memset(port_str, 0x00, 0x6);

		snprintf(port_str, 6, "%d", port);

		// result of a disconnecting from APRS-IS server
		sim800_return_t disconnection_result = SIM800_UNSET;

		// connecting has blocking I/O
		retval = gsm_sim800_tcpip_connect(address, address_ln, port_str, 0x6, aprsis_serial_port, aprsis_gsm_modem_state);

		// if connection was successful
		if (retval == SIM800_OK) {
			// wait for hello message '# aprsc 2.1.10-gd72a17c'
			retval = gsm_sim800_tcpip_receive(0, 0, aprsis_serial_port, aprsis_gsm_modem_state, 0, 2000);

			if (retval == 0) {
				receive_buff = srl_get_rx_buffer(aprsis_serial_port);

				// check if receive buffer starts from printable character and needs fast forward or not
				// maybe APRS-IS put a newline before hello message
				offset = text_fast_forward_to_first_printable((char*)receive_buff, srl_get_num_bytes_rxed(aprsis_serial_port));

				// if hello message has been received
				if (offset >= 0 && (*(receive_buff + offset) == '#' && *(receive_buff + offset + 1) == ' ')) {
					// send long string to server
					gsm_sim800_tcpip_write((uint8_t *)aprsis_login_string, strlen(aprsis_login_string), aprsis_serial_port, aprsis_gsm_modem_state);

					// wait for server response
					retval = gsm_sim800_tcpip_receive(0, 0, aprsis_serial_port, aprsis_gsm_modem_state, 0, 2000);

					if (retval == SIM800_OK) {
						receive_buff = srl_get_rx_buffer(aprsis_serial_port);

						aprsis_connected = 1;

						// fast forward to the beginning of a response
						offset = text_fast_forward_to_first_printable((char*)receive_buff, srl_get_num_bytes_rxed(aprsis_serial_port));

						// check if authorization has been successfull
						retval = strncmp(aprsis_sucessfull_login, (const char * )(receive_buff + offset), (size_t)9);
						if (retval == 0) {
							aprsis_logged = 1;

							// set current timestamp as last
							aprsis_last_keepalive_ts = master_time;

							if (auto_send_beacon != 0) {
								aprsis_send_beacon(0, aprsis_callsign_with_ssid, main_string_latitude, main_symbol_f, main_string_longitude, main_symbol_s, main_config_data_basic);
							}

							// set timeout for aprs-is server
							srl_switch_timeout(aprsis_serial_port, 1, APRSIS_TIMEOUT_MS);

							// wait for consecutive data
							gsm_sim800_tcpip_async_receive(aprsis_serial_port, aprsis_gsm_modem_state, 0, 61000, aprsis_receive_callback);

							out = APRSIS_OK;

						}
						else {
							// if authoruzation wasn't successfull drop a connection
							disconnection_result = aprsis_disconnect();

							// increase failure counter
							aprsis_unsucessfull_conn_counter++;
						}
					}
				}
				else {
					disconnection_result = aprsis_disconnect();

					// increase failure counter
					aprsis_unsucessfull_conn_counter++;
				}
			}
			else {
				disconnection_result = aprsis_disconnect();

				// increase failure counter
				aprsis_unsucessfull_conn_counter++;
			}
		}

		// if a connection has been ordered to close, but there were severe errors during that
		if (disconnection_result == SIM800_TCP_CLOSE_UNCERTAIN ||
			disconnection_result == SIM800_RECEIVING_TIMEOUT ||
			aprsis_unsucessfull_conn_counter > APRSIS_FAILED_CONN_ATTEMPTS_TO_RESET_GSM) {

			// reset unsuccesfull connection counter back to zero
			aprsis_unsucessfull_conn_counter = 0;

			// and reset GSM modem
			gsm_sim800_reset(aprsis_gsm_modem_state);
		}

	}

	return out;

}

/**
 * Connect and login to APRS-IS using default credentials using during initialization
 * @param auto_send_beacon
 * @return
 */
aprsis_return_t aprsis_connect_and_login_default(uint8_t auto_send_beacon) {

	return aprsis_connect_and_login(aprsis_default_server_address, aprsis_default_server_address_ln, aprsis_default_server_port, auto_send_beacon);
}

sim800_return_t aprsis_disconnect(void) {

	sim800_return_t out;

	out = gsm_sim800_tcpip_close(aprsis_serial_port, aprsis_gsm_modem_state, 0);

	aprsis_logged = 0;

	aprsis_connected = 0;

	return out;
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

/**
 * Pooler function which check periodically if APRS-IS connection is alive.
 */
void aprsis_check_alive(void) {

	uint32_t timestamp = 0;

	timestamp = master_time;

	// check if connection is alive
	if (aprsis_logged == 1 && (timestamp > (aprsis_last_keepalive_ts + APRSIS_TIMEOUT_MS))) {
		// reset the flag
		aprsis_logged = 0;

		aprsis_connected = 0;

		// check if it is intendend to reset GSM modem in case of timeout
		if (aprsis_reset_on_timeout == 0) {
			// close connection with force flag as it is uncertain if a remote server
			// finished connection explicitly, or the connection is stuck for
			// some other reason
			gsm_sim800_tcpip_close(aprsis_serial_port, aprsis_gsm_modem_state, 1);
		}
		else {
			gsm_sim800_reset(aprsis_gsm_modem_state);
		}
	}
}

/**
 *
 * @param windspeed
 * @param windgusts
 * @param winddirection
 * @param temperatura
 * @param cisnienie
 * @param humidity
 * @param callsign_with_ssid
 * @param string_latitude
 * @param string_longitude
 * @param config_data_basic
 */
void aprsis_send_wx_frame(
		uint16_t windspeed,
		uint16_t windgusts,
		uint16_t winddirection,
		float temperatura,
		float cisnienie,
		uint8_t humidity,
		const char * callsign_with_ssid,
		const char * string_latitude,
		const char * string_longitude,
		const config_data_basic_t * config_data_basic) {

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
 	aprsis_packet_tx_message_size = sprintf(
 			aprsis_packet_tx_buffer,
			"%s>AKLPRZ,qAR,%s:!%s%c%c%s%c%c%03d/%03dg%03dt%03dr...p...P...b%05ldh%02d\r\n",
			callsign_with_ssid,
			callsign_with_ssid,
			string_latitude,
			config_data_basic->n_or_s,
			'/',
			string_longitude,
			config_data_basic->e_or_w,
			'_',
			/* kierunek */direction,
			/* predkosc*/(int)wind_speed_mph,
			/* porywy */(short)(wind_gusts_mph),
			/*temperatura */(short)(temperatura*1.8+32),
			pressure,
			humidity);
 	aprsis_packet_tx_buffer[aprsis_packet_tx_message_size] = 0;

 	gsm_sim800_tcpip_async_write((uint8_t *)aprsis_packet_tx_buffer, aprsis_packet_tx_message_size, aprsis_serial_port, aprsis_gsm_modem_state);
}

/**
 * Sends beacon packet to APRS-IS
 * @param async zero for blocking io, which lock this function during transmission.
 * 				non zero for non blocking io, function will return immediately and sending will be done in background
 */
void aprsis_send_beacon(
		uint8_t async,
		const char * callsign_with_ssid,
		const char * string_latitude,
		char symbol_f,
		const char * string_longitude,
		char symbol_s,
		const config_data_basic_t * config_data_basic
		) {

	if (aprsis_logged == 0) {
		return;
	}

	aprsis_packet_tx_message_size = sprintf(
			aprsis_packet_tx_buffer,
			"%s>AKLPRZ,qAR,%s:=%s%c%c%s%c%c %s\r\n",
			callsign_with_ssid,
			callsign_with_ssid,
			string_latitude,
			config_data_basic->n_or_s,
			symbol_f,
			string_longitude,
			config_data_basic->e_or_w,
			symbol_s,
			config_data_basic->comment);
	  aprsis_packet_tx_buffer[aprsis_packet_tx_message_size] = 0;

	  if (async > 0) {
		  gsm_sim800_tcpip_async_write((uint8_t *)aprsis_packet_tx_buffer, aprsis_packet_tx_message_size, aprsis_serial_port, aprsis_gsm_modem_state);
	  }
	  else {
		 	gsm_sim800_tcpip_write((uint8_t *)aprsis_packet_tx_buffer, aprsis_packet_tx_message_size, aprsis_serial_port, aprsis_gsm_modem_state);
	  }
}

void aprsis_igate_to_aprsis(AX25Msg *msg, const char * callsign_with_ssid) {

	// iterator to move across tx buffer
	uint16_t tx_buffer_it = 0;

	uint16_t payload_ln = 0;

	// string lenght returned by snprintf
	int snprintf_size = 0;

	// exif if APRSIS is not logged
	if (aprsis_logged == 0 || msg == 0) {
		return;
	}

	// prepare buffer for message
	memset(aprsis_packet_tx_buffer, 0x00, APRSIS_TX_BUFFER_LN);

	// put callsign
	strncat(aprsis_packet_tx_buffer, msg->src.call, 6);

	// put source call
	if ((msg->src.ssid & 0xF) != 0) {
		snprintf_size = snprintf(aprsis_packet_tx_buffer, MAXIMUM_CALL_SSID_DASH_LN, "%.6s-%d>", msg->src.call, msg->src.ssid & 0xF);
	}
	else {
		// callsign without SSID
		snprintf_size = snprintf(aprsis_packet_tx_buffer, MAXIMUM_CALL_SSID_DASH_LN, "%.6s>", msg->src.call);
	}

	// move iterator forward
	tx_buffer_it = (uint8_t) snprintf_size & 0xFFU;

	// put destination call - for sake of simplicity ignore SSID
	snprintf_size = snprintf(aprsis_packet_tx_buffer + tx_buffer_it, MAXIMUM_CALL_SSID_DASH_LN, "%.6s,", msg->dst.call);

	// move iterator
	tx_buffer_it += (uint8_t) snprintf_size & 0xFFU;

	// put original path
	for (int i = 0; i < msg->rpt_cnt; i++) {
		if ((msg->rpt_lst[i].ssid & 0x0F) == 0) {
			if ((msg->rpt_lst[i].ssid & 0x40) == 0x40)
				snprintf_size = sprintf(aprsis_packet_tx_buffer + tx_buffer_it, "%.6s*,", msg->rpt_lst[i].call);
			else
				snprintf_size = sprintf(aprsis_packet_tx_buffer + tx_buffer_it, "%.6s,", msg->rpt_lst[i].call);
		}
		else {
			if ((msg->rpt_lst[i].ssid & 0x40) == 0x40)
				snprintf_size = sprintf(aprsis_packet_tx_buffer + tx_buffer_it, "%.6s-%d*,", msg->rpt_lst[i].call, (msg->rpt_lst[i].ssid & 0x0F));
			else
				snprintf_size = sprintf(aprsis_packet_tx_buffer + tx_buffer_it, "%.6s-%d,", msg->rpt_lst[i].call, (msg->rpt_lst[i].ssid & 0x0F));
		}

		// move iterator
		tx_buffer_it += (uint8_t) snprintf_size & 0xFFU;
	}

	snprintf_size = snprintf(aprsis_packet_tx_buffer + tx_buffer_it, MAXIMUM_CALL_SSID_DASH_LN + 5, "qAR,%s:", callsign_with_ssid);

	// move iterator
	tx_buffer_it += (uint8_t) snprintf_size & 0xFFU;

	// cut the data field if it is too long to fit in transmission buffer
	if (msg->len + tx_buffer_it >= APRSIS_TX_BUFFER_LN) {
		payload_ln = APRSIS_TX_BUFFER_LN - tx_buffer_it - 1;	// keep one byte for null terminator
	}
	else {
		payload_ln = msg->len;
	}

	memcpy(aprsis_packet_tx_buffer + tx_buffer_it, msg->info, payload_ln);


}

char * aprsis_get_tx_buffer(void) {
	return aprsis_packet_tx_buffer;
}

uint8_t aprsis_get_aprsis_logged(void) {
	return aprsis_logged;
}
