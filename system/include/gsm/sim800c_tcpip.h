/*
 * sim800c_tcpip.h
 *
 *  Created on: Feb 17, 2022
 *      Author: mateusz
 */

#ifndef INCLUDE_GSM_SIM800C_TCPIP_H_
#define INCLUDE_GSM_SIM800C_TCPIP_H_

#define TEST_IP 	"46.21.223.230\0"
#define TEST_PORT	"14580\0"

#include <stdint.h>

#include "drivers/serial.h"

#include "gsm/sim800_state_t.h"

extern const char * TCP3;
extern const char * TCP4;

uint8_t gsm_sim800_tcpip_connect(char * ip_address, uint8_t ip_address_ln, char * port, uint8_t port_ln, srl_context_t * srl_context, gsm_sim800_state_t * state);

#endif /* INCLUDE_GSM_SIM800C_TCPIP_H_ */
