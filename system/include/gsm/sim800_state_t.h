/*
 * sim800c_state_t.h
 *
 *  Created on: Jan 26, 2022
 *      Author: mateusz
 */

#ifndef INCLUDE_GSM_SIM800_STATE_T_H_
#define INCLUDE_GSM_SIM800_STATE_T_H_

typedef enum gsm_sim800_state_t {
	SIM800_UNKNOWN,
	SIM800_POWERED_OFF,
	SIM800_POWERING_ON,
	SIM800_WAITING_FOR_POWERUP,
	SIM800_NOT_YET_COMM,
	SIM800_HANDSHAKING,
	SIM800_INITIALIZING,
	SIM800_INITIALIZING_GPRS,
	SIM800_ALIVE,
	SIM800_ALIVE_SENDING_TO_MODEM,
	SIM800_ALIVE_WAITING_MODEM_RESP,
	SIM800_TCP_CONNECTED,
	SIM800_INHIBITED,
	SIM800_INHIBITED_RESET_COUNTER,
}gsm_sim800_state_t;

#endif /* INCLUDE_GSM_SIM800_STATE_T_H_ */
