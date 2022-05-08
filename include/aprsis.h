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



typedef enum aprsis_return {
	APRSIS_OK					= 0,
	APRSIS_NOT_CONFIGURED		= 1,
	APRSIS_WRONG_STATE			= 2,
	APRSIS_ALREADY_CONNECTED	= 3
}aprsis_return_t;

extern uint8_t aprsis_connected;

void aprsis_init(srl_context_t * context, gsm_sim800_state_t * gsm_modem_state, char * callsign, uint8_t ssid, uint32_t passcode, char * default_server, uint16_t default_port);
aprsis_return_t aprsis_connect_and_login(char * address, uint8_t address_ln, uint16_t port);
aprsis_return_t aprsis_connect_and_login_default(void);
void aprsis_disconnect(void);
void aprsis_receive_callback(srl_context_t* srl_context);
void aprsis_check_alive(void);

void aprsis_send_wx_frame(uint16_t windspeed, uint16_t windgusts, uint16_t winddirection, float temperatura, float cisnienie, uint8_t humidity);
void aprsis_send_beacon(uint8_t async);

#endif /* APRSIS_H_ */
