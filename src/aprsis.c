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

char aprsis_callsign_with_ssid[10];

int32_t aprsis_passcode;

char aprsis_login_string[64];

uint8_t aprsis_connected;

const char * aprsis_sucessfull_login = "# logresp\0";

void aprsis_init(srl_context_t * context, gsm_sim800_state_t * gsm_modem_state, char * callsign, uint8_t ssid, uint32_t passcode) {
	aprsis_serial_port = context;

	aprsis_gsm_modem_state = gsm_modem_state;

	aprsis_passcode = (int32_t)passcode;

	sprintf(aprsis_callsign_with_ssid, "%s-%d", callsign, ssid);

	memset(aprsis_login_string, 0x00, 0x40);

	sprintf(aprsis_login_string, "user %s pass %ld vers ParaMETEO %s \r\n", aprsis_callsign_with_ssid, aprsis_passcode, SW_VER);

	aprsis_connected = 0;

}

void aprsis_connect_and_login(char * address, uint8_t address_ln, uint16_t port) {

	if (aprsis_serial_port == 0 || aprsis_gsm_modem_state == 0) {
		return;
	}

	char port_str[6];

	uint8_t * receive_buff;

	int8_t retval = 0xFF;

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

			if (*receive_buff == '#' && *(receive_buff + 1) == ' ') {
				gsm_sim800_tcpip_write((uint8_t *)aprsis_login_string, strlen(aprsis_login_string), aprsis_serial_port, aprsis_gsm_modem_state);

				retval = gsm_sim800_tcpip_receive(0, 0, aprsis_serial_port, aprsis_gsm_modem_state, 0, 2000);

				if (retval == 0) {
					receive_buff = srl_get_rx_buffer(aprsis_serial_port);

					retval = strncmp(aprsis_sucessfull_login, (const char * )receive_buff, (size_t)9);
					if (retval == 0) {
						aprsis_connected = 1;

					}
				}
			}
		}
	}

}
