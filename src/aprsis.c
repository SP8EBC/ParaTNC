/*
 * aprsis.c
 *
 *  Created on: Feb 20, 2022
 *      Author: mateusz
 */

#include "aprsis.h"
#include "etc/aprsis_config.h"
#include "text.h"
#include "backup_registers.h"

#include "aprs/status.h"
#include "aprs/message.h"

#include "gsm/sim800c.h"
#include "gsm/sim800c_poolers.h"

#include "main.h"
#include "rte_main.h"

#include <stdio.h>
#include <string.h>

#ifdef UNIT_TEST
#define STATIC
#else
#define STATIC static
#endif

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
static char aprsis_packet_tx_buffer[APRSIS_TX_BUFFER_LN];

/**
 * Lenght of buffer
 */
#define APRSIS_TELEMETRY_BUFFER_LN	96

/**
 * Buffer used to sent telemetry frames to APRS-IS. It is also
 * used to construct frame with GPS connection status, which is sent
 * only once after APRS-IS connection is established
 */
char aprsis_packet_telemetry_buffer[APRSIS_TELEMETRY_BUFFER_LN];

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
 * Pointer to callsign from configuration
 */
const char * aprsis_callsign;

/**
 * ssid from configuration
 */
uint8_t aprsis_ssid;

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

static uint8_t aprsis_successfull_conn_counter = 0;

/**
 * Counter of unsuccessful connects to APRS-IS, to trigger GSM modem reset.
 * Please note that it works differently that 'aprsis_reset_on_timeout' and
 * has nothing to do with a timeout of already established connection. This
 * counter will trigger GSM modem reset even if no APRS-IS connection has
 * been established at all. It protects against a situation when GSM modem
 * is not able to register in cellular network, SIM card is not working for
 * some reason etc. Of course there is no guarantee that a reset in such
 * case will help, but there is nothing better to do.
 */
static uint8_t aprsis_unsucessfull_conn_counter = 0;

/**
 * Set to one if the GSM modem shall be immediately reset when APRS-IS communication
 * time out. Please note that this is different that 'aprsis_unsucessfull_conn_counter'
 * and reseting GSM mode after many unsuccessfull connection attempts! This
 * comes in when the connection has been established at least one time. It won't
 * help if there is some problem with establishing connection at all.
 */
static uint32_t aprsis_reset_on_timeout = 0;

/**
 * Number of RF packets igated to APRS-IS system
 */
static uint16_t aprsis_igated_counter = 0;

/**
 * Counter of all packets originated from the station transmitted to APRS-IS server.
 * This doesn't include igated packets!
 */
static uint16_t aprsis_tx_counter = 0;

/**
 * Amount of keepalive packet received from the server. It is reset to zero
 * every connect event
 */
static uint16_t aprsis_keepalive_received_counter = 0;

/**
 * Amount of packets which are not keepalive
 */
static uint16_t aprsis_another_received_counter = 0;

/**
 * A timestamp when server has send anything
 */
static uint32_t aprsis_last_keepalive_ts = 0;

/**
 * This is the second timestamp of last keepalive message
 * from APRS-IS server. It is used by 'aprsis_check_connection_attempt_alive'
 * and not incremented anywhere except receive callback. Where a timeout
 * calculated using this value is too long the controller is restarted.
 */
static uint32_t aprsis_last_keepalive_long_ts = 0;

/**
 * A timestamp when any packet has been sent to
 */
static uint32_t aprsis_last_packet_transmit_ts = 0;

/**
 *
 */
static uint32_t aprsis_last_packet_transmit_long_ts = 0;

/**
 * Only for debugging purposes
 */
static uint8_t aprsis_debug_simulate_timeout = 0;

#define APRSIS_LOGIN_STRING_RECEIVED_LN	64

char aprsis_login_string_reveived[APRSIS_LOGIN_STRING_RECEIVED_LN];

#define APRSIS_TIMEOUT_MS	123000//123000

/**
 * Lenght of 6 letter callsign + two digit SSID + end character like '>' or ","
 */
#define MAXIMUM_CALL_SSID_DASH_LN	10

/**
 * Checks if data in a buffer contains APRS message
 * @param message
 * @param message_ln
 * @return position at which content of message starts
 */
STATIC uint16_t aprsis_check_is_message(const uint8_t * const message, const uint16_t message_ln) {
	// example message
	//			Details:"SP8EBC>APX216,TCPIP*,qAC,NINTH::SR9WXZ   :tedt{0s}\r\n", '\0' <repeats 715 times>

	// go through a buffer and look for double ':'

	uint16_t message_start_position = 0;

	for (int i = 0; i < message_ln; i++) {
		const uint8_t * this_character = message + i;

		const uint8_t * next_character = message + i + 1;

		if (*this_character == 0x00) {
			break;
		}

		if ((*this_character == ':') && (*next_character == ':')) {
			message_start_position = i + 2;
			break;
		}
	}

	return message_start_position;
}

/**
 *
 * @param srl_context
 */
STATIC void aprsis_receive_callback(srl_context_t* srl_context) {

	const uint8_t * buffer = srl_get_rx_buffer(srl_context);

	const uint16_t message_ln = srl_context->srl_rx_bytes_counter;

	// if something was actually received
	if (srl_context->srl_rx_state == SRL_RX_DONE) {
		// check if this is keepalive message
		if (*buffer == '#') {
			// set last timestamps
			aprsis_last_keepalive_ts = main_get_master_time();
			aprsis_last_keepalive_long_ts = main_get_master_time();

			// increase received keepalive counter
			aprsis_keepalive_received_counter++;

			// restart receiving from serial port
			gsm_sim800_tcpip_async_receive(aprsis_serial_port, aprsis_gsm_modem_state, 0, 61000, aprsis_receive_callback);
		}
		else {
			// check if this is an aprs message
			const int message_position = aprsis_check_is_message(buffer, message_ln);

			// if yes try to decode it
			if (message_position != 0) {

				// prevent overwriting message received from radio channel if it hasn't been serviced yet
				if (rte_main_received_message.source == MESSAGE_SOURCE_UNINITIALIZED) {

					// if decoding was successfull
					if (message_decode(buffer, message_ln, message_position, MESSAGE_SOURCE_APRSIS, &rte_main_received_message) == 0) {

						// check if it is for me
						if (message_is_for_me(aprsis_callsign, aprsis_ssid, &rte_main_received_message) == 0) {
							// trigger preparing ACK
							rte_main_trigger_message_ack = 1;
						}


					}
				}
			}
			else {
				;
			}

			aprsis_another_received_counter++;

			gsm_sim800_tcpip_async_receive(aprsis_serial_port, aprsis_gsm_modem_state, 0, 61000, aprsis_receive_callback);

		}
	}
}

/**
 *
 * @param context
 * @param gsm_modem_state
 * @param callsign
 * @param ssid
 * @param passcode
 * @param default_server
 * @param default_port
 * @param reset_on_timeout
 * @param callsign_with_ssid
 */
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

	aprsis_callsign = callsign;

	aprsis_ssid = ssid;

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

		aprsis_tx_counter = 0;

		aprsis_keepalive_received_counter = 0;

		aprsis_another_received_counter = 0;

		aprsis_igated_counter = 0;

		memset(port_str, 0x00, 0x6);

		memset(aprsis_login_string_reveived, 0x00, APRSIS_LOGIN_STRING_RECEIVED_LN);

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

					// store received login string (with an information about which server is connected now)
					strncpy(aprsis_login_string_reveived, receive_buff + offset, APRSIS_LOGIN_STRING_RECEIVED_LN);

					// wait for server response
					retval = gsm_sim800_tcpip_receive(0, 0, aprsis_serial_port, aprsis_gsm_modem_state, 0, 2000);

					if (retval == SIM800_OK) {
						receive_buff = srl_get_rx_buffer(aprsis_serial_port);

						aprsis_connected = 1;

						aprsis_successfull_conn_counter++;

						// fast forward to the beginning of a response
						offset = text_fast_forward_to_first_printable((char*)receive_buff, srl_get_num_bytes_rxed(aprsis_serial_port));

						// check if authorization has been successfull
						retval = strncmp(aprsis_sucessfull_login, (const char * )(receive_buff + offset), (size_t)9);
						if (retval == 0) {
							aprsis_logged = 1;

							// trigger GSM status APRS-IS packet, when connection is ready
							rte_main_trigger_gsm_status = 1;

							// set current timestamp as last
							aprsis_last_keepalive_ts = master_time;

							aprsis_last_keepalive_long_ts = aprsis_last_keepalive_ts;

							if (auto_send_beacon != 0) {
								aprsis_send_beacon(0, aprsis_callsign_with_ssid, main_string_latitude, main_symbol_f, main_string_longitude, main_symbol_s, main_config_data_basic);
							}

							// trigger GSM status packet
							rte_main_trigger_gsm_loginstring_packet = 1;

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
			//disconnection_result == SIM800_RECEIVING_TIMEOUT ||
			aprsis_unsucessfull_conn_counter > APRSIS_FAILED_CONN_ATTEMPTS_TO_RESET_GSM) {

			// and reset GSM modem
			gsm_sim800_reset(aprsis_gsm_modem_state);

			// reset unsuccesfull connection counter back to zero
			aprsis_unsucessfull_conn_counter = 0;
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

	sim800_return_t out = SIM800_UNSET;

	if (aprsis_connected == 1) {

		out = gsm_sim800_tcpip_close(aprsis_serial_port, aprsis_gsm_modem_state, 0);

		aprsis_logged = 0;

		aprsis_connected = 0;
	}

	return out;
}

/**
 * Pooler function which check periodically if APRS-IS connection is alive.
 */
void aprsis_check_alive(void) {

	const uint32_t timestamp = master_time;

	uint8_t dead = 0;

	if (aprsis_debug_simulate_timeout == 1) {
		dead = 1;
	}

	if (aprsis_successfull_conn_counter > 0) {
		if (timestamp > (aprsis_last_keepalive_ts + APRSIS_TIMEOUT_MS)) {
			dead =  1;
		}

		if (timestamp > (aprsis_last_packet_transmit_ts + APRSIS_TIMEOUT_MS * 3 )) {
			dead = 1;
		}
	}

	// check if connection is alive
	if (dead == 1){
		// reset the flag
		aprsis_logged = 0;

		aprsis_connected = 0;

		aprsis_debug_simulate_timeout = 0;

		aprsis_last_keepalive_ts = master_time;

		aprsis_last_packet_transmit_ts = master_time;

		if (rte_main_curret_powersave_mode != PWSAVE_AGGRESV) {
			// send a status message that APRS-IS connectios is gone
			status_send_aprsis_timeout(aprsis_unsucessfull_conn_counter);
		}

		// check if it is intended to reset GSM modem in case of timeout
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
 * This is another alive check which is fully independent from
 * if the connection has been even already established and how many times
 * it waas. The intention here is to reset the whole controller if
 * for some reason APRS-++IS connection cannot be established for very long time
 * @return
 */
int aprsis_check_connection_attempt_alive(void) {

	int out = 0;

	const uint32_t timestamp = main_get_master_time();

	if (timestamp > (aprsis_last_keepalive_long_ts + APRSIS_TIMEOUT_MS * 6)) {
		out =  1;
	}

	if (timestamp > (aprsis_last_packet_transmit_long_ts + APRSIS_TIMEOUT_MS * 6 )) {
		out = 1;
	}

	return out;
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

	if (gsm_sim800_tcpip_tx_busy() == 1) {
		// will die here
		backup_assert(BACKUP_REG_ASSERT_CONCURENT_ACCES_APRSIS_WX);
	}

	aprsis_tx_counter++;

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
 	aprsis_packet_tx_message_size = snprintf(
 			aprsis_packet_tx_buffer,
			APRSIS_TX_BUFFER_LN,
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

 	aprsis_last_packet_transmit_ts = main_get_master_time();

 	aprsis_last_packet_transmit_long_ts = main_get_master_time();

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

	if (gsm_sim800_tcpip_tx_busy() == 1) {
		// will die here
		backup_assert(BACKUP_REG_ASSERT_CONCURENT_ACCES_APRSIS_BEACON);
	}

	aprsis_tx_counter++;

	aprsis_packet_tx_message_size = snprintf(
			aprsis_packet_tx_buffer,
			APRSIS_TX_BUFFER_LN,
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

	  aprsis_last_packet_transmit_ts = main_get_master_time();
}

/**
 * Prepare telemetry packets to be sent later to the APRS-IS. Just store a string
 * with all values, which will be then embedded into packet to be sent to APRS-IS
 * @param _telemetry_counter
 * @param _rx_pkts
 * @param _tx_pkts
 * @param _digi_pkts
 * @param _scaled_vbatt_voltage
 * @param _viscous_drop_pkts
 * @param _scaled_temperature
 * @param _telemetry_qf
 * @param _telemetry_degr
 * @param _telemetry_nav
 * @param _telemetry_pressure_qf_navaliable
 * @param _telemetry_humidity_qf_navaliable
 * @param _telemetry_anemometer_degradated
 * @param _telemetry_anemometer_navble
 * @param _telemetry_vbatt_low
 * @param _config_mode
 */
void aprsis_prepare_telemetry(
		uint16_t _telemetry_counter,
		uint8_t _rx_pkts,
		uint8_t _tx_pkts,
		uint8_t _digi_pkts,
		uint8_t _scaled_vbatt_voltage,
		uint8_t _viscous_drop_pkts,
		uint8_t _scaled_temperature,
		char _telemetry_qf,
		char _telemetry_degr,
		char _telemetry_nav,
		char _telemetry_pressure_qf_navaliable,
		char _telemetry_humidity_qf_navaliable,
		char _telemetry_anemometer_degradated,
		char _telemetry_anemometer_navble,
		char _telemetry_vbatt_low,
		const config_data_mode_t * const _config_mode) {

	// clear buffer before doin anything
	memset(aprsis_packet_telemetry_buffer, 0x00, APRSIS_TELEMETRY_BUFFER_LN);

	// string lenght returned by snprintf
	int snprintf_size = 0;

	if (_config_mode->digi_viscous == 0) {
		snprintf_size = snprintf(
				aprsis_packet_telemetry_buffer,
				APRSIS_TELEMETRY_BUFFER_LN,
				"T#%03d,%03d,%03d,%03d,%03d,%03d,%c%c%c%c%c%c%c%c\r\n",
				_telemetry_counter,
				_rx_pkts,
				_tx_pkts,
				_digi_pkts,
				_scaled_vbatt_voltage,
				_scaled_temperature,
				_telemetry_qf,
				_telemetry_degr,
				_telemetry_nav,
				_telemetry_pressure_qf_navaliable,
				_telemetry_humidity_qf_navaliable,
				_telemetry_anemometer_degradated,
				_telemetry_anemometer_navble,
				_telemetry_vbatt_low);

	}
	else {
		snprintf_size = snprintf(
				aprsis_packet_telemetry_buffer,
				APRSIS_TELEMETRY_BUFFER_LN,
				"T#%03d,%03d,%03d,%03d,%03d,%03d,%c%c%c%c%c%c%c%c\r\n",
				_telemetry_counter,
				_rx_pkts,
				_viscous_drop_pkts,
				_digi_pkts,
				_scaled_vbatt_voltage,
				_scaled_temperature,
				_telemetry_qf,
				_telemetry_degr,
				_telemetry_nav,
				_telemetry_pressure_qf_navaliable,
				_telemetry_humidity_qf_navaliable,
				_telemetry_anemometer_degradated,
				_telemetry_anemometer_navble,
				_telemetry_vbatt_low);
	}

	aprsis_packet_telemetry_buffer[snprintf_size] = 0;

}

/**
 * Sends to APRS-IS prepared telemetry frame prepared in advance
 * @param async
 * @param callsign_with_ssid
 */
void aprsis_send_telemetry(uint8_t async, const char * callsign_with_ssid) {

	// exif if APRSIS is not logged
	if (aprsis_logged == 0) {
		return;
	}

	// exit if message is empty
	if (*aprsis_packet_telemetry_buffer == 0) {
		return;
	}

	if (gsm_sim800_tcpip_tx_busy() == 1) {
		// will die here
		backup_assert(BACKUP_REG_ASSERT_CONCURENT_ACCES_APRSIS_TELEMETRY);
	}

	aprsis_tx_counter++;

	aprsis_packet_tx_message_size = snprintf(
			aprsis_packet_tx_buffer,
			APRSIS_TX_BUFFER_LN,
			"%s>AKLPRZ,qAR,%s:%s\r\n",
			callsign_with_ssid,
			callsign_with_ssid,
			aprsis_packet_telemetry_buffer);
	  aprsis_packet_tx_buffer[aprsis_packet_tx_message_size] = 0;

	  if (async > 0) {
		  gsm_sim800_tcpip_async_write((uint8_t *)aprsis_packet_tx_buffer, aprsis_packet_tx_message_size, aprsis_serial_port, aprsis_gsm_modem_state);
	  }
	  else {
		 	gsm_sim800_tcpip_write((uint8_t *)aprsis_packet_tx_buffer, aprsis_packet_tx_message_size, aprsis_serial_port, aprsis_gsm_modem_state);
	  }

	  aprsis_last_packet_transmit_ts = main_get_master_time();

	// clear buffer after it has been used
	memset(aprsis_packet_telemetry_buffer, 0x00, APRSIS_TELEMETRY_BUFFER_LN);
}

/**
 * Sends
 * @param async
 * @param what kind of telemetry description packet should be sent now
 * @param callsign_with_ssid
 * @return what kind of telemetry description should be sent next
 */
telemetry_description_t aprsis_send_description_telemetry(uint8_t async,
								const telemetry_description_t what,
								const config_data_basic_t * const config_basic,
								const config_data_mode_t * const config_mode,
								const char * callsign_with_ssid) {

	telemetry_description_t next = TELEMETRY_NOTHING;

	// check what we want to send and what
	switch (what) {
		case TELEMETRY_PV_PARM: 		next = TELEMETRY_PV_EQNS; break;
		case TELEMETRY_PV_EQNS: 		next = TELEMETRY_PV_UNIT; break;
		case TELEMETRY_PV_UNIT: 		next = TELEMETRY_NOTHING; break;
		case TELEMETRY_NORMAL_PARAM: 	next = TELEMETRY_NORMAL_EQNS; break;
		case TELEMETRY_NORMAL_EQNS: 	next = TELEMETRY_NORMAL_UNIT; break;
		case TELEMETRY_NORMAL_UNIT: 	next = TELEMETRY_NOTHING; break;
		case TELEMETRY_VISCOUS_PARAM:	next = TELEMETRY_VISCOUS_EQNS; break;
		case TELEMETRY_VISCOUS_EQNS:	next = TELEMETRY_VISCOUS_UNIT; break;
		case TELEMETRY_VISCOUS_UNIT:	next = TELEMETRY_NOTHING; break;
		case TELEMETRY_NOTHING:
		default:						next = TELEMETRY_NOTHING; break;
	}

	// exif if APRSIS is not logged
	if (aprsis_logged == 0) {
		return next;
	}

	if (gsm_sim800_tcpip_tx_busy() == 1) {
		// will die here
		backup_assert(BACKUP_REG_ASSERT_CONCURENT_ACCES_APRSIS_DESCR);
	}

	telemetry_create_description_string(config_basic, what, main_own_aprs_msg, OWN_APRS_MSG_LN);

	aprsis_tx_counter++;

	aprsis_packet_tx_message_size = snprintf(
			aprsis_packet_tx_buffer,
			APRSIS_TX_BUFFER_LN,
			"%s>AKLPRZ,qAR,%s:%s\r\n",
			callsign_with_ssid,
			callsign_with_ssid,
			main_own_aprs_msg);
	  aprsis_packet_tx_buffer[aprsis_packet_tx_message_size] = 0;

	if (async > 0) {
	  gsm_sim800_tcpip_async_write((uint8_t *)aprsis_packet_tx_buffer, aprsis_packet_tx_message_size, aprsis_serial_port, aprsis_gsm_modem_state);
	}
	else {
		gsm_sim800_tcpip_write((uint8_t *)aprsis_packet_tx_buffer, aprsis_packet_tx_message_size, aprsis_serial_port, aprsis_gsm_modem_state);
	}

	return next;
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

	if (gsm_sim800_tcpip_tx_busy() == 1) {
		// will die here
		backup_assert(BACKUP_REG_ASSERT_CONCURENT_ACCES_APRSIS_IGATE);
	}

	aprsis_igated_counter++;

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

	// move iterator forward by payload size
	tx_buffer_it += payload_ln;

	// put newline at the end
	aprsis_packet_tx_buffer[tx_buffer_it++] = '\r';
	aprsis_packet_tx_buffer[tx_buffer_it++] = '\n';

	aprsis_packet_tx_message_size = strlen(aprsis_packet_tx_buffer);


	gsm_sim800_tcpip_async_write((uint8_t *)aprsis_packet_tx_buffer, aprsis_packet_tx_message_size, aprsis_serial_port, aprsis_gsm_modem_state);


}

void aprsis_send_server_comm_counters(const char * callsign_with_ssid) {

	if (aprsis_logged == 0) {
		return;
	}

	if (gsm_sim800_tcpip_tx_busy() == 1) {
		// will die here
		backup_assert(BACKUP_REG_ASSERT_CONCURENT_ACCES_APRSIS_CNTRS);
	}

	memset (aprsis_packet_tx_buffer, 0x00, APRSIS_TX_BUFFER_LN);

	aprsis_tx_counter++;

	aprsis_packet_tx_message_size = snprintf(
									aprsis_packet_tx_buffer,
									APRSIS_TX_BUFFER_LN - 1,
									"%s>AKLPRZ,qAR,%s:>[aprsis][igated: %d][transmited: %d][keepalive: %d][another: %d]\r\n",
									callsign_with_ssid,
									callsign_with_ssid,
									aprsis_igated_counter,
									aprsis_tx_counter,
									aprsis_keepalive_received_counter,
									aprsis_another_received_counter);

 	aprsis_last_packet_transmit_ts = main_get_master_time();

 	gsm_sim800_tcpip_async_write((uint8_t *)aprsis_packet_tx_buffer, aprsis_packet_tx_message_size, aprsis_serial_port, aprsis_gsm_modem_state);
}

void aprsis_send_loginstring(const char * callsign_with_ssid, uint8_t rtc_ok, uint16_t voltage) {

	if (aprsis_logged == 0) {
		return;
	}

	if (gsm_sim800_tcpip_tx_busy() == 1) {
		// will die here
		backup_assert(BACKUP_REG_ASSERT_CONCURENT_ACCES_APRSIS_LOGINSTRING);
	}

	memset (aprsis_packet_tx_buffer, 0x00, APRSIS_TX_BUFFER_LN);

	aprsis_tx_counter++;

	aprsis_packet_tx_message_size = snprintf(
									aprsis_packet_tx_buffer,
									APRSIS_TX_BUFFER_LN - 1,
									"%s>AKLPRZ,qAR,%s:>[rtc_ok: %d][vbat: %d][register_reset_check_fail: 0x%X][aprsis]%s\r\n",
									callsign_with_ssid,
									callsign_with_ssid,
									rtc_ok,
									voltage,
									backup_reg_get_register_reset_check_fail(),
									aprsis_login_string_reveived);

 	gsm_sim800_tcpip_async_write((uint8_t *)aprsis_packet_tx_buffer, aprsis_packet_tx_message_size, aprsis_serial_port, aprsis_gsm_modem_state);

}

void aprsis_send_gsm_status(const char * callsign_with_ssid) {
	if (aprsis_logged == 0) {
		return;
	}

	memset (aprsis_packet_tx_buffer, 0x00, APRSIS_TX_BUFFER_LN);

	aprsis_tx_counter++;

	// reuse a buffer for telemetry for this one occasion
	gsm_sim800_create_status(aprsis_packet_telemetry_buffer, APRSIS_TELEMETRY_BUFFER_LN);

	aprsis_packet_tx_message_size = snprintf(
									aprsis_packet_tx_buffer,
									APRSIS_TX_BUFFER_LN - 1,
									"%s>AKLPRZ,qAR,%s:%s\r\n",
									callsign_with_ssid,
									callsign_with_ssid,
									aprsis_packet_telemetry_buffer);

 	gsm_sim800_tcpip_async_write((uint8_t *)aprsis_packet_tx_buffer, aprsis_packet_tx_message_size, aprsis_serial_port, aprsis_gsm_modem_state);
}

/**
 *
 * @param message
 */
void aprsis_send_ack_for_message(const message_t * const message) {
	aprsis_packet_tx_message_size = message_create_ack_for((uint8_t*)aprsis_packet_tx_buffer, APRSIS_TX_BUFFER_LN - 1, message);

 	gsm_sim800_tcpip_async_write((uint8_t *)aprsis_packet_tx_buffer, aprsis_packet_tx_message_size, aprsis_serial_port, aprsis_gsm_modem_state);

}

/**
 *
 * @param message pointer to string buffer with a message to send to APRS-IS
 * @param ln lenght of a string (not size of a buffer!!)
 */
void aprsis_send_any_string_buffer(const char * const message, const uint16_t ln) {

	if (aprsis_logged == 0 || ln == 0) {
		return;
	}

	if (gsm_sim800_tcpip_tx_busy() == 1) {
		// will die here
		backup_assert(BACKUP_REG_ASSERT_CONCURENT_ACCES_APRSIS_OTHER);
	}

	// copy input message to intermediate message buffer
	strcpy(aprsis_packet_tx_buffer, message);

	*(aprsis_packet_tx_buffer + ln) = '\r';
	*(aprsis_packet_tx_buffer + ln + 1) = '\n';
	*(aprsis_packet_tx_buffer + ln + 2) = 0x00;
	*(aprsis_packet_tx_buffer + ln + 3) = 0x00;

	aprsis_packet_tx_message_size = ln + 2;

 	gsm_sim800_tcpip_async_write((uint8_t *)aprsis_packet_tx_buffer, aprsis_packet_tx_message_size, aprsis_serial_port, aprsis_gsm_modem_state);
}

#ifdef UNIT_TEST
char * aprsis_get_tx_buffer(void) {
	return aprsis_packet_tx_buffer;
}
#endif

uint8_t aprsis_get_aprsis_logged(void) {
	return aprsis_logged;
}

void aprsis_debug_set_simulate_timeout(void) {
	aprsis_debug_simulate_timeout = 1;
}
