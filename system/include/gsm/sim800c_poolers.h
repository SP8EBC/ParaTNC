/*
 * sim800_poolers.h
 *
 *  Created on: Jan 27, 2022
 *      Author: mateusz
 */

#ifndef INCLUDE_GSM_SIM800C_POOLERS_H_
#define INCLUDE_GSM_SIM800C_POOLERS_H_

#include "drivers/serial.h"
#include "gsm/sim800_state_t.h"
#include "config_data.h"

void gsm_sim800_poolers_one_minute(srl_context_t * srl_context, gsm_sim800_state_t * state);
void gsm_sim800_poolers_one_second(srl_context_t * srl_context, gsm_sim800_state_t * state, const config_data_gsm_t * config);

#endif /* INCLUDE_GSM_SIM800C_POOLERS_H_ */
