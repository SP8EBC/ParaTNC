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
#include "gsm_comm_state_handler.h"

#include "packet_tx_handler.h"

#include <stdint.h>

void gsm_sim800_poolers_ten_seconds (srl_context_t *srl_context, gsm_sim800_state_t *state)
{
	(void)srl_context;
	(void)state;
	// if no engineering is currently processed, gprs is ready and APRS-IS connection
	// is not alive now.
	if (/* sim800_poolers_request_engineering == 0 &&
		gsm_sim800_gprs_ready == 1 && */
		aprsis_connected == 0 && 
		gsm_comm_state_get_current () == GSM_COMM_APRSIS) 
		{
			aprsis_connect_and_login_default (1);
		}

	gsm_sim800_decrease_counter ();
}

void gsm_sim800_poolers_one_second(srl_context_t * srl_context, gsm_sim800_state_t * state, const config_data_gsm_t * config) {

	if (*state == SIM800_ALIVE) {

		// initialize GPRS if it is not initialized
		if (gsm_sim800_gprs_ready == 0) {
			sim800_gprs_initialize(srl_context, state, config);

			//return;
		}
		else {
			// if GPRS is ready an there is a request to obtain engineering information
			if (gsm_comm_state_get_current() == GSM_COMM_ENGINEERING) {

				gsm_sim800_engineering_pool(srl_context, state);
			}

		}


	}

}

void gsm_sim800_poolers_request_engineering(void) {
	sim800_poolers_request_engineering = 1;
}
