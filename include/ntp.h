/*
 * ntp.h
 *
 *  Created on: Aug 17, 2024
 *      Author: mateusz
 */

#ifndef NTP_H_
#define NTP_H_

#include <stdint.h>

#include "drivers/serial.h"
#include "gsm/sim800c_tcpip.h"

extern uint8_t ntp_done;

/**
 *
 * @param context
 * @param gsm_modem_state
 */
void ntp_init(srl_context_t * context, gsm_sim800_state_t * gsm_modem_state);

/**
 *
 */
void ntp_get_sync(void);


#endif /* NTP_H_ */
