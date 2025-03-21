/*
 * sim800c_engineering.h
 *
 *  Created on: Jan 26, 2022
 *      Author: mateusz
 */

#ifndef INCLUDE_GSM_SIM800C_ENGINEERING_H_
#define INCLUDE_GSM_SIM800C_ENGINEERING_H_

#include "gsm/sim800c.h"
#include "gsm/sim800_state_t.h"

/// ==================================================================================================
///	X-MACROS
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL MACROS
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL DEFINITIONS
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL TYPES
/// ==================================================================================================

/// ==================================================================================================
///	GLOBAL VARIABLES
/// ==================================================================================================

extern const char * ENGINEERING_ENABLE;
extern const char * ENGINEERING_DISABLE;
extern const char * ENGINEERING_GET;

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================


//extern uint8_t gsm_sim800_engineering_is_enabled;
//
//extern uint8_t gsm_sim800_engineering_successed;
//
//extern uint8_t gsm_sim800_engineering_fail;

uint8_t gsm_sim800_engineering_get_is_done(void);

void gsm_sim800_engineering_pool(srl_context_t * srl_context, gsm_sim800_state_t * state);

//void gsm_sim800_engineering_enable(srl_context_t * srl_context, gsm_sim800_state_t * state);
//void gsm_sim800_engineering_disable(srl_context_t * srl_context, gsm_sim800_state_t * state);
//void gsm_sim800_engineering_request_data(srl_context_t * srl_context, gsm_sim800_state_t * state);
void gsm_sim800_engineering_response_callback(srl_context_t * srl_context, gsm_sim800_state_t * state, uint16_t gsm_response_start_idx);


#endif /* INCLUDE_GSM_SIM800C_ENGINEERING_H_ */
