/*
 * sim800_poolers.c
 *
 *  Created on: Jan 27, 2022
 *      Author: mateusz
 */

#include "gsm/sim800c_poolers.h"
#include "gsm/sim800c_engineering.h"
#include "gsm/sim800c_gprs.h"
#include "gsm/sim800c_tcpip.h"

#include "aprsis.h"

#include <stdint.h>

uint8_t sim800_poolers_five = 3;

void gsm_sim800_poolers_one_minute(srl_context_t * srl_context, gsm_sim800_state_t * state){


//		gsm_sim800_tcpip_connect(TEST_IP, strlen(TEST_IP), TEST_PORT, strlen(TEST_PORT), srl_context, state);
//		//gsm_sim800_engineering_enable(srl_context, state);
//
//		gsm_sim800_tcpip_close(srl_context, state);

}


void gsm_sim800_poolers_five_minutes(srl_context_t * srl_context, gsm_sim800_state_t * state) {
	sim800_poolers_five++;

	if (sim800_poolers_five == 5) {
		sim800_poolers_five  = 0;
	}

}

void gsm_sim800_poolers_one_second(srl_context_t * srl_context, gsm_sim800_state_t * state, const config_data_gsm_t * config) {

	if (*state == SIM800_ALIVE) {

		if (gsm_sim800_gprs_ready == 0) {
			sim800_gprs_initialize(srl_context, state, config);
		}

		if (gsm_sim800_engineering_is_enabled == 1 && gsm_sim800_engineering_successed == 0) {
			gsm_sim800_engineering_request_data(srl_context, state);

			return;
		}

		if (gsm_sim800_engineering_successed == 1) {
			gsm_sim800_engineering_disable(srl_context, state);

			return;
		}
	}
}
