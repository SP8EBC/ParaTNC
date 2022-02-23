/*
 * aprsis.h
 *
 *  Created on: Feb 20, 2022
 *      Author: mateusz
 */

#ifndef APRSIS_H_
#define APRSIS_H_

#include "drivers/serial.h"
#include "gsm/sim800c_tcpip.h"

void aprsis_init(srl_context_t * context, gsm_sim800_state_t * gsm_modem_state, char * callsign, uint8_t ssid, uint32_t passcode);
void aprsis_connect_and_login(char * address, uint8_t address_ln, uint16_t port);

#endif /* APRSIS_H_ */
