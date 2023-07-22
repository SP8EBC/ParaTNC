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

#include "packet_tx_handler.h"

#include <stdint.h>

//!< Set to one externally to request engineering, get one time at startup by default
uint8_t sim800_poolers_request_engineering = 1;

void gsm_sim800_poolers_ten_seconds(srl_context_t * srl_context, gsm_sim800_state_t * state){

	// if no engineering is currently processed, gprs is ready and APRS-IS connection
	// is not alive now.
	if (	gsm_sim800_engineering_is_enabled == 0 &&
			gsm_sim800_gprs_ready == 1 &&
			aprsis_connected == 0) {
		aprsis_connect_and_login_default(1);
	}

	gsm_sim800_decrease_counter();

}

void gsm_sim800_poolers_one_second(srl_context_t * srl_context, gsm_sim800_state_t * state, const config_data_gsm_t * config) {

	if (*state == SIM800_ALIVE) {

		// initialize GPRS if it is not initialized
		if (gsm_sim800_gprs_ready == 0) {
			sim800_gprs_initialize(srl_context, state, config);

			return;
		}
		else {
			// if GPRS is ready an there is a request to obtain engineering information
			if (sim800_poolers_request_engineering == 1 && aprsis_connected == 0) {

				// initial state when engineering is not enabled and not finished
				if (gsm_sim800_engineering_is_enabled == 0 && gsm_sim800_engineering_successed == 0) {
					gsm_sim800_engineering_enable(srl_context, state);

					return;
				}

				if (gsm_sim800_engineering_is_enabled == 1 && gsm_sim800_engineering_successed == 0) {
					gsm_sim800_engineering_request_data(srl_context, state);

					return;
				}

				if (gsm_sim800_engineering_successed == 1) {
					gsm_sim800_engineering_disable(srl_context, state);

					// engineering request is single shot
					sim800_poolers_request_engineering = 0;

					// request
					packet_tx_force_gsm_status();

					return;
				}
			}

		}


	}

}

void gsm_sim800_poolers_request_engineering(void) {
	sim800_poolers_request_engineering = 1;
}
