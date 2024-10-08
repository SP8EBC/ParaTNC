/*
 * sim800c_gprs.h
 *
 *  Created on: Jan 28, 2022
 *      Author: mateusz
 */

#ifndef INCLUDE_GSM_SIM800C_GPRS_H_
#define INCLUDE_GSM_SIM800C_GPRS_H_


#include <stored_configuration_nvm/config_data.h>
#include "drivers/serial.h"
#include "gsm/sim800_state_t.h"
#include "gsm/sim800_mcc_t.h"

extern const char * START_CONFIG_APN;
extern const char * SHUTDOWN_GPRS;
extern const char * SHUTDOWN_GRPS_RESPONSE;
extern const char * ENABLE_EDGE;
extern const char * START_GPRS;
extern const char * GET_IP_ADDRESS;
extern const char * GET_CONNECTION_STATUS;
extern const char * CONFIGURE_DTR;

extern int8_t gsm_sim800_gprs_ready;

/// ==================================================================================================
///	FUNCTIONS
/// ==================================================================================================

void sim800_gprs_initialize(srl_context_t * srl_context, gsm_sim800_state_t * state, const config_data_gsm_t * config_gsm);
void sim800_gprs_create_apn_config_str(char * buffer, uint16_t buffer_ln, const sim800_mcc_t mcc, const uint8_t mnc);
void sim800_gprs_response_callback(srl_context_t * srl_context, gsm_sim800_state_t * state, uint16_t gsm_response_start_idx);
void sim800_gprs_reset(void);

void sim800_gprs_create_status(char * buffer, int ln);

#endif /* INCLUDE_GSM_SIM800C_GPRS_H_ */
